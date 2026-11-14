#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace lab04 {
struct Profile {
    const char* id; int priority; uint32_t ttlMs, cooldownMs;
    bool critical, presentation;
    const char* trigger; double leftTrigger, rightTrigger;
    const char* wave; double amplitude, pulseHz;
    const char* speaker; int light; // 0 none, 1 teal, 2 amber, 3 crimson
};
}
#include "profiles.generated.hpp"

namespace lab04 {
inline double Unit(double x) { return std::isfinite(x) ? std::clamp(x, 0.0, 1.0) : 0.0; }
struct Settings {
    bool triggers = true, haptics = true, speaker = true, light = true;
    double triggerScale = 1, hapticScale = 1, speakerScale = 0.5;
};
struct Cue { std::string id; bool mainAudio = true, caption = true; double speakerGain = 0; };
struct Frame {
    double leftTrigger = 0, rightTrigger = 0;
    std::string triggerMode = "off";
    double leftHaptic = 0, rightHaptic = 0;
    std::string hapticAsset;
    double r = 0, g = 0, b = 0;
    std::vector<Cue> cues;
};
// Application seam. A macOS engine backend translates supported Frame channels
// to device calls and authored assets; unsupported channels use game fallbacks.
// PS5/DualSense controls remain primary; the backend must implement release.
struct IFeedbackDevice {
    virtual ~IFeedbackDevice() = default;
    virtual void Apply(const Frame&) = 0;
    virtual void StopAll() = 0;
};
class Mixer {
    struct Active { const Profile* p; uint64_t begin, expires; double severity, direction; };
    struct Pending { const Profile* p; uint64_t expires; };
    std::unordered_map<std::string, Active> active_;
    std::unordered_map<std::string, uint64_t> lastCue_;
    std::vector<Pending> pending_;
    uint64_t clock_ = 0;
    uint64_t lastFrame_ = 0;
    double color_[3] = {0, 0, 0};
    bool enabled_ = true, dead_ = false;
    IFeedbackDevice& device_;
    Settings settings_;
    static uint64_t Add(uint64_t a, uint64_t b) {
        return a > UINT64_MAX - b ? UINT64_MAX : a + b;
    }
    void Prune(uint64_t now) {
        for (auto it = active_.begin(); it != active_.end();)
            if (it->second.expires <= now) it = active_.erase(it); else ++it;
    }
    void Clear() {
        active_.clear(); pending_.clear(); lastCue_.clear();
        color_[0] = color_[1] = color_[2] = 0; lastFrame_ = clock_; device_.StopAll();
    }
public:
    explicit Mixer(IFeedbackDevice& device) : device_(device) {}
    void Configure(Settings s) {
        settings_ = s; settings_.triggerScale = Unit(s.triggerScale);
        settings_.hapticScale = Unit(s.hapticScale); settings_.speakerScale = Unit(s.speakerScale);
        device_.StopAll(); // Release immediately, before the next render tick.
    }
    void SetActive(bool active) { enabled_ = active; Clear(); }
    void BeginRun() { Clear(); dead_ = false; enabled_ = true; }
    void Release(const std::string& id) {
        active_.erase(id);
        pending_.erase(std::remove_if(pending_.begin(), pending_.end(),
            [&](const Pending& p){ return id == p.p->id; }), pending_.end());
    }
    bool Submit(const std::string& id, uint64_t nowMs, double severity = 1,
                double direction = 0, bool presentation = false) {
        if (nowMs < clock_ || !enabled_ || dead_) return false;
        clock_ = nowMs; Prune(nowMs);
        auto p = std::find_if(Profiles.begin(), Profiles.end(), [&](const Profile& p){return id == p.id;});
        if (p == Profiles.end()) return false;
        if (presentation && (p->critical || id == "player.died" || id == "ending.reached")) return false;
        if (p->presentation && !presentation) return false;
        if (id == "player.died") { dead_ = true; Clear(); return true; }
        if (id == "ending.reached") { SetActive(false); return true; }
        if (id == "pump.stopped") { active_.erase("pump.nominal"); active_.erase("pump.low_fuel"); }
        if (id == "pump.nominal" || id == "pump.low_fuel") {
            active_.erase("pump.stopped");
            active_.erase(id == "pump.nominal" ? "pump.low_fuel" : "pump.nominal");
        }
        if (p->ttlMs) {
            // Refresh expiry without restarting oscillation phase.
            auto old = active_.find(id);
            uint64_t begin = old == active_.end() ? nowMs : old->second.begin;
            active_.insert_or_assign(id, Active{&*p, begin, Add(nowMs, p->ttlMs), Unit(severity),
                std::isfinite(direction) ? std::clamp(direction, -1.0, 1.0) : 0.0});
        }
        if (p->speaker[0]) {
            auto prior = lastCue_.find(id);
            if (prior == lastCue_.end() || nowMs - prior->second >= p->cooldownMs) {
                pending_.push_back({&*p, Add(nowMs, p->ttlMs)}); lastCue_[id] = nowMs;
            }
        }
        return true;
    }
    Frame Tick(uint64_t nowMs) {
        nowMs = std::max(nowMs, clock_); clock_ = nowMs; Prune(nowMs);
        Frame f;
        if (!enabled_ || dead_) { device_.Apply(f); return f; }
        const uint64_t frameDelta = nowMs - std::min(nowMs, lastFrame_);
        lastFrame_ = nowMs;
        const Active *trigger = nullptr, *haptic = nullptr, *light = nullptr;
        bool danger = false;
        // Stable tie break by event ID, independent of hash iteration order.
        auto higher = [](const Active& a, const Active* b) {
            return !b || a.p->priority > b->p->priority ||
                (a.p->priority == b->p->priority && std::string(a.p->id) < b->p->id);
        };
        for (const auto& pair : active_) {
            const Active& a = pair.second; danger |= a.p->critical;
            if ((a.p->leftTrigger || a.p->rightTrigger) && higher(a, trigger)) trigger = &a;
            if (a.p->amplitude && higher(a, haptic)) haptic = &a;
            if (a.p->light && higher(a, light)) light = &a;
        }
        if (settings_.triggers && trigger) {
            double pulse = std::string(trigger->p->trigger) == "chug" ?
                0.7 + 0.3 * std::sin((nowMs - trigger->begin) * 0.0125663706) : 1;
            f.leftTrigger = Unit(trigger->p->leftTrigger * trigger->severity * settings_.triggerScale * pulse);
            f.rightTrigger = Unit(trigger->p->rightTrigger * trigger->severity * settings_.triggerScale * pulse);
            f.triggerMode = trigger->p->trigger;
        }
        if (settings_.haptics && haptic) {
            const auto& a = *haptic;
            double phase = (nowMs - a.begin) * 0.001 * a.p->pulseHz;
            double envelope = a.p->pulseHz > 0 ? 0.35 + 0.65 * std::pow(0.5 + 0.5 * std::cos(phase * 6.283185307), 4) : 1;
            if (std::string(a.p->wave) == "pump_sputter")
                envelope *= (static_cast<uint64_t>((nowMs - a.begin) / 130) % 7 == 2 ? 0.1 : 1.0);
            double amp = Unit(a.p->amplitude * a.severity * settings_.hapticScale * envelope);
            f.leftHaptic = Unit(amp * (1 - std::max(0.0, a.direction)));
            f.rightHaptic = Unit(amp * (1 + std::min(0.0, a.direction)));
            f.hapticAsset = a.p->wave;
        }
        if (settings_.light && light) {
            double color[3][3] = {{0.02,0.28,0.30},{0.65,0.25,0.01},{0.65,0.01,0.04}};
            auto c = color[light->p->light - 1];
            double pulse = light->p->light == 3 ? 0.65 + 0.35 * std::sin((nowMs-light->begin)*0.0062831853) : 1;
            f.r = c[0]*pulse; f.g = c[1]*pulse; f.b = c[2]*pulse;
        }
        if (settings_.light && light) {
            const double alpha = 1.0 - std::exp(-static_cast<double>(frameDelta) / 180.0);
            color_[0] += (f.r - color_[0]) * alpha;
            color_[1] += (f.g - color_[1]) * alpha;
            color_[2] += (f.b - color_[2]) * alpha;
            f.r = color_[0]; f.g = color_[1]; f.b = color_[2];
        } else { color_[0] = color_[1] = color_[2] = 0; }
        // Speaker has one voice. Critical source wins over cosmetic/phantom radio.
        const Profile* voice = nullptr;
        for (const auto& cue : pending_) {
            if (cue.expires <= nowMs) continue;
            if (danger && cue.p->presentation) continue;
            if (!voice || cue.p->priority > voice->priority ||
                (cue.p->priority == voice->priority && std::string(cue.p->id) < voice->id)) voice = cue.p;
            // Main audio and captions carry every accepted cue, never speaker-only alarms.
            f.cues.push_back({cue.p->speaker, true, true, 0});
        }
        if (voice && settings_.speaker)
            for (auto& cue : f.cues) if (cue.id == voice->speaker) cue.speakerGain = settings_.speakerScale;
        pending_.clear(); device_.Apply(f); return f;
    }
};
}
