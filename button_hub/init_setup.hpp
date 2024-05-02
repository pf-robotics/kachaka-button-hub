#pragma once

static const char* kApSsid = "KachakaBtnHub";
static const char* kApPass = "12345678";

class InitialSetup {
 public:
  enum class State {
    kInit,
    kWaitForWiFiClient,
    kWaitForSettings,
  };

  explicit InitialSetup();
  void RunLoop();

 private:
  static void DrawScreen(State state);

  State prev_state_;
  State next_state_;
};
