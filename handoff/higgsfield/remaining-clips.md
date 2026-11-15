# Remaining realism clips (as of 2026-11-15)

Done and in the game (`unreal/Content/Movies/Higgsfield/`): 01, 02 part 1, 02 part 2, 03, 04, 05 (draft), 06 part 1
(trimmed locally to 16.5 s), 06 part 2.

Still to generate, with exact parameters in `remaining-clips.json`:

| Clip | Why | Start image |
| :--- | :--- | :--- |
| `05_telemetry_fault` | Redo: the first take reframes the camera | still job `f623053a-…` |
| `07_valve_stuck` | New | still job `f5c648db-…` |
| `08_crank_valve_1` | New | still job `4d0a0e98-…` |
| `08_crank_valve_2` | New, after 08 part 1 | last frame of 08 part 1, uploaded |
| `09_dome_look` | New | still job `66740eab-…` |
| `10_log_close` | New | still job `efecf95f-…` |

## Why they weren't generated from Claude Code

Higgsfield rejects `use_unlim: true` from Claude Code. Free unlimited generations only work from Claude chat,
Claude Cowork or ChatGPT. The paid route costs about 140 credits per 20-second clip, against a recorded
balance of 9.5 credits. The free trial ends **2026-11-17 06:17 EST**.

## How to run them (Claude chat or Cowork, with the Higgsfield connector)

Paste this into a Claude chat with the Higgsfield connector, attaching `remaining-clips.json`:

> Submit each entry in this file with Higgsfield `generate_video`, using its `params` exactly, `use_unlim: true`.
> Submit one at a time and wait for each to finish (the account hits rate limits). Submit
> `08_crank_valve_2` only after `08_crank_valve_1` completes: take its last frame, upload it, and use that as
> `start_image`. Give me each `filename` with its job ID and result URL.

Then, back in Claude Code: "Download the new clips". The job IDs go in `realism-results.json`. Each MP4 goes in
`unreal/Content/Movies/Higgsfield/<filename>.mp4`. The game picks them up with no code change, because
`opening_slice_footage.json` already lists every filename.
