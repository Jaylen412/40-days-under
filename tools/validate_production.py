"""Validate contracts between authored scene data, feedback and the asset queue."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(relative):
    return json.loads((ROOT / relative).read_text())


def validate():
    contract = read("production/state_contract.json")
    events = set(contract["events"])
    assert len(events) == len(contract["events"]), "Duplicate canonical event IDs"
    scene = read("production/vertical_slice.json")
    assert scene["target_duration_wall_seconds"] == 300
    assert scene["time_policy"]["game_hours_per_wall_second"] == contract["clock"]["opening_override"]
    assert scene["time_policy"]["campaign_game_hours_per_wall_second_after_completion"] == contract["clock"]["default_game_hours_per_wall_second"]
    elapsed = 0
    line_ids = set()
    for beat in scene["beats"]:
        assert beat["target_start_seconds"] == elapsed, "Scene pacing contains a gap or overlap"
        elapsed += beat["target_duration_seconds"]
        for event in beat["events"]:
            assert event["id"] in events, f"Unknown scene event: {event['id']}"
            assert 0 <= event["severity"] <= 1
            assert -1 <= event.get("direction", 0) <= 1
        for line in beat["lines"]:
            assert line["id"] not in line_ids, "Duplicate localization ID"
            line_ids.add(line["id"])
    assert elapsed == 300

    feedback = read("production/dualsense_events.json")
    assert {event["id"] for event in feedback["events"]} == events

    manifest = read("production/assets/manifest.json")
    assets = manifest["assets"]
    assert [asset["id"] for asset in assets] == ["7.1", "7.2", "7.3", "7.4", "7.5"]
    previous = None
    for asset in assets:
        assert asset["predecessor"] == previous
        assert asset["prompt"] and asset["negative_prompt"]
        assert asset["generation_params"]["aspect_ratio"] == asset["aspect_ratio"]
        if asset["duration"] is not None:
            assert asset["generation_params"]["duration"] == asset["duration"]
        if asset["status"] == "completed":
            assert asset["job_id"] and asset["result_url"], "Completed asset lacks provenance"
        previous = asset["id"]
    if assets[1].get("job_id"):
        assert assets[0]["status"] == "completed", "Character motion preceded turnaround completion"
        assert manifest["identity_anchor"]["reference_element_id"], "Character motion has no persistent anchor"
    print(f"Production contracts passed: {len(events)} events, {len(scene['beats'])} beats, {len(assets)} asset requests")


if __name__ == "__main__":
    validate()
