#pragma once

namespace beep {

void Begin(int volume);

void SetVolume(int volume);

void PlayInitialSetupNext();
void PlayInitialSetupPrev();
void PlayInitialSetupComplete();

void PlayInitialSetupBleCommandReceived();
void PlayInitialSetupBleCommandRejected();
void PlayInitialSetupBleCommandCompleted();

void PlayNextPage();

void PlayCommandSent();
void PlayCommandFailed();
void PlayBeaconDetected();

void PlayOtaCompleted();
void PlayOtaFailed();
void PlayClearAllDataCompleted();

}  // namespace beep
