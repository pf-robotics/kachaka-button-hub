#include "qrcode.hpp"

#include "common.hpp"

namespace qrcode {

void ShowQrCode(const char* text, float horizontal_position) {
  constexpr int kQrSize = 150;
  constexpr int kQrVersion = 4;

  const int width = M5.Lcd.width();
  const int height = M5.Lcd.height();
  const int x = (width - kQrSize) * horizontal_position;

  M5.Lcd.qrcode(text, x, height - kQrSize, kQrSize, kQrVersion);
}

}  // namespace qrcode
