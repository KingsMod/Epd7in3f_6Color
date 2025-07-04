#include <Epd7in3f_6Color.h>

Epd7in3f_6Color epd;

void setup() {
  Serial.begin(115200);
  epd.Init();
  epd.Clear(EPD_6COLOR_WHITE);

  uint8_t row[800 / 2];
  for (int i = 0; i < sizeof(row); ++i) row[i] = 0x33;

  for (int y = 0; y < 240; ++y)
    epd.OverlayImage(row, 0, y, 800, 1);

  epd.RefreshDisplay();
}

void loop() {}
