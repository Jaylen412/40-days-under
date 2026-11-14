#include "feedback.hpp"
#include <cassert>
#include <iostream>

struct Device : lab04::IFeedbackDevice {
    lab04::Frame last;
    int stops = 0;
    void Apply(const lab04::Frame& f) override { last = f; }
    void StopAll() override { last = {}; ++stops; }
};
int main() {
    Device d; lab04::Mixer m(d);
    assert(m.Submit("pump.nominal", 0));
    auto f = m.Tick(100);
    assert(f.hapticAsset == "pump_thump" && f.g > f.r);
    assert(m.Submit("pump.low_fuel", 100));
    f = m.Tick(500);
    assert(f.hapticAsset == "pump_sputter" && f.r > f.g);
    assert(m.Submit("air.co2_warning", 500));
    assert(m.Submit("isolation.phantom_radio", 500, 1, 0, true));
    f = m.Tick(900);
    assert(f.hapticAsset == "alarm_double" && f.r > f.g * 2);
    assert(f.cues.size() == 1 && f.cues[0].id == "co2_alarm");
    assert(f.cues[0].mainAudio && f.cues[0].caption && f.cues[0].speakerGain > 0);
    assert(m.Submit("air.co2_warning", 1000));
    assert(m.Tick(1000).cues.empty()); // repeated state does not retrigger speech
    f = m.Tick(3000);
    assert(f.hapticAsset.empty() && f.r == 0 && f.leftTrigger == 0);
    assert(!m.Submit("air.co2_warning", 3000, 1, 0, true));
    assert(!m.Submit("unknown", 3000));
    assert(m.Submit("hull.groan", 3000, 1, 0, true));
    m.Release("hull.groan");
    assert(m.Submit("radio.message", 3000));
    m.Release("radio.message");
    assert(m.Tick(3000).cues.empty()); // release clears pending audio
    assert(m.Submit("speargun.load", 3000, 2, 99));
    f = m.Tick(3000);
    assert(f.rightTrigger == .65 && f.leftHaptic == 0 && f.rightHaptic <= 1);
    m.Release("speargun.load");
    assert(m.Submit("valve.turn", 3000, std::numeric_limits<double>::quiet_NaN()));
    f = m.Tick(3000);
    assert(f.rightTrigger == 0 && std::isfinite(f.rightHaptic));
    m.SetActive(false);
    assert(d.last.leftTrigger == 0 && d.stops > 0);
    assert(!m.Submit("watch.alarm", 4000));
    m.SetActive(true);
    assert(m.Tick(4000).cues.empty());
    lab04::Settings s; s.haptics = s.triggers = s.speaker = s.light = false;
    m.Configure(s);
    assert(m.Submit("watch.alarm", 4000));
    f = m.Tick(4000);
    assert(f.leftHaptic == 0 && f.r == 0 && f.rightTrigger == 0);
    assert(f.cues.size() == 1 && f.cues[0].speakerGain == 0 && f.cues[0].mainAudio && f.cues[0].caption);
    assert(m.Submit("player.died", 4000));
    m.SetActive(true);
    assert(!m.Submit("pump.nominal", 4000)); // re-enable cannot resurrect run
    assert(m.Tick(4000).cues.empty());
    m.BeginRun();
    assert(m.Submit("radio.message", 4000));
    assert(m.Tick(12000).cues.empty()); // expired pending cue is never played late
    assert(!m.Submit("pump.nominal", 100)); // old timestamps rejected
    assert(m.Submit("pressure.unsafe_ascent", 12000));
    assert(m.Submit("ending.reached", 12000));
    assert(m.Tick(12000).rightTrigger == 0);
    std::cout << "Feedback priority, expiry, source validation, accessibility, lifecycle and clamping checks passed\n";
}
