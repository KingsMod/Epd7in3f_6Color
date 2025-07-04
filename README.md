# Epd7in3f_6Color
!!! This is a WIP with serial streaming still in development. I hope you can contribute to the project!!!

An ESP32 Arduino library for controlling the Waveshare 7.3" Spectra 6-color E-Ink display. This library avoids RAM bottlenecks by using SPIFFS as a persistent framebuffer.

## Features

- 🧠 Works without PSRAM
- 🎨 6-color support (Black, White, Red, Yellow, Blue, Green)
- ⚡ Region overlay updates
- 🔁 Serial streaming of image chunks in base64 (!!!WIP!!!)
- 🛠 SPI 4-wire communication

## Pinout

| EPD Pin | ESP32 Pin |
|---------|-----------|
| RST     | 16        |
| DC      | 17        |
| CS      | 5         |
| BUSY    | 4         |
| CLK/MOSI| default SPI pins |

## Example

```cpp
#include <Epd7in3f_6Color.h>

Epd7in3f_6Color epd;

void setup() {
  Serial.begin(115200);
  epd.Init();
  epd.Clear(EPD_6COLOR_WHITE);

  // Stream or overlay here...
}

void loop() {}
```
