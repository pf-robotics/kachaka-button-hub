#include "beep.hpp"

#include "common.hpp"

namespace beep {

constexpr int kDuration = 50;
constexpr int kFreqCH = 554;
constexpr int kFreqC = 523;
constexpr int kFreqE = 659;
constexpr int kFreqG = 783;

static inline int ConvertVolume(int setting) {
  return (setting * 255) / 11;
}

void Begin(const int volume) {
  auto spk_cfg = M5.Speaker.config();
  M5.Speaker.config(spk_cfg);
  M5.Speaker.begin();
  M5.Speaker.setVolume(ConvertVolume(volume));
}

static void ToneAndDelay(int freq, int duration, bool mute = false) {
  M5.Speaker.tone(freq);
  delay(duration);
  if (mute) {
    M5.Speaker.stop();
  }
}

void SetVolume(const int volume) {
  M5.Speaker.setVolume(ConvertVolume(volume));
  ToneAndDelay(kFreqC, kDuration, true);
}

void PlayInitialSetupNext() {
  ToneAndDelay(kFreqC, kDuration);
  ToneAndDelay(kFreqE, kDuration);
  ToneAndDelay(kFreqG, kDuration, true);
}

void PlayInitialSetupPrev() {
  ToneAndDelay(kFreqG, kDuration);
  ToneAndDelay(kFreqE, kDuration);
  ToneAndDelay(kFreqC, kDuration, true);
}

void PlayInitialSetupComplete() {
  for (int i = 0; i < 3; i++) {
    PlayInitialSetupNext();
    delay(kDuration);
  }
  M5.Speaker.stop();
}

void PlayNextPage() {
  ToneAndDelay(kFreqC, kDuration, true);
}

void PlayCommandSent() {
  ToneAndDelay(kFreqE, kDuration);
  ToneAndDelay(kFreqG, kDuration, true);
}

void PlayCommandFailed() {
  delay(kDuration * 2);
  ToneAndDelay(kFreqCH, kDuration);
  delay(kDuration / 2);
  ToneAndDelay(kFreqCH, kDuration * 2, true);
}

void PlayBeaconDetected() {
  ToneAndDelay(kFreqC, kDuration, true);
  delay(kDuration / 2);
  ToneAndDelay(kFreqC, kDuration, true);
}

void PlayOtaCompleted() {
  PlayInitialSetupComplete();  // TODO: Replace
}

void PlayOtaFailed() {
  PlayCommandFailed();  // TODO: Replace
}

void PlayClearAllDataCompleted() {
  PlayInitialSetupComplete();  // TODO: Replace
}

}  // namespace beep
