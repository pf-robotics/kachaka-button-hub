#include "gpio_button.hpp"

#include <M5Unified.h>

namespace gpio_button {

static constexpr std::array<int, 6> kGpioButtonPins = {16, 17, 2, 5, 35, 36};

constexpr uint64_t kButtonPushIgnoreDurationMs = 50;

class ButtonImpl {
 public:
  explicit ButtonImpl(int pin)
      : pin_(pin),
        was_switch_on_(false),
        last_on_time_(0),
        button_push_event_(false) {}

  void Begin() { pinMode(pin_, INPUT); }
  void UpdateButtonState();
  bool PopButtonPushEvent();
  int GetPin() const { return pin_; }

 private:
  int pin_;

  bool was_switch_on_;
  int last_on_time_;

  bool button_push_event_;
};

static std::vector<ButtonImpl> g_gpio_buttons;

void ButtonImpl::UpdateButtonState() {
  const time_t now = millis();
  bool is_switch_on = digitalRead(pin_);

  if (is_switch_on and !was_switch_on_) {
    last_on_time_ = now;
  }

  if (last_on_time_ == -1) {
    was_switch_on_ = is_switch_on;
    return;
  }

  const uint64_t button_duration_ms = now - last_on_time_;
  if (was_switch_on_ and !is_switch_on) {
    if (button_duration_ms > kButtonPushIgnoreDurationMs) {
      button_push_event_ = true;
    }
  }

  if (!is_switch_on) {
    last_on_time_ = -1;
  }
  was_switch_on_ = is_switch_on;
}

bool ButtonImpl::PopButtonPushEvent() {
  const bool event = button_push_event_;
  button_push_event_ = false;
  return event;
}

void Register(CommandTable& command_table) {
  for (const int pin : kGpioButtonPins) {
    g_gpio_buttons.emplace_back(pin);
  }
  for (ButtonImpl& button : g_gpio_buttons) {
    button.Begin();
  }
  for (int i = 0; i < g_gpio_buttons.size(); ++i) {
    command_table.SetButtonName(KButton(GpioButton(i + 1)),
                                "HubPlusボタン" + String(i + 1));
  }
}

void Unregister(CommandTable& command_table) {
  for (int i = 0; i < g_gpio_buttons.size(); ++i) {
    command_table.DeleteButtonName(KButton(GpioButton(i + 1)));
  }
  g_gpio_buttons.clear();
}

void HandleEvents(const std::function<void(const KButton&)>& callback) {
  for (int i = 0; i < g_gpio_buttons.size(); ++i) {
    ButtonImpl& button = g_gpio_buttons.at(i);
    button.UpdateButtonState();
    if (button.PopButtonPushEvent()) {
      callback(KButton(GpioButton(i + 1)));
    }
  }
}

}  // namespace gpio_button
