#ifndef EPD7IN3F_6COLOR_H
#define EPD7IN3F_6COLOR_H

#include "epdif.h"  // Include the EpdIf class definition

// Pin definition (same as epdif.h)
#define RST_PIN         16
#define DC_PIN          17
#define CS_PIN          5
#define BUSY_PIN        4

// Display resolution
#define EPD_WIDTH       800
#define EPD_HEIGHT      480

#define UWORD   unsigned int
#define UBYTE   unsigned char
#define UDOUBLE  unsigned long

// 6 Color Index (4-bit values matching your COLOR_MAP)
#define EPD_6COLOR_BLACK   0x0  // 0000
#define EPD_6COLOR_WHITE   0x1  // 0001
#define EPD_6COLOR_YELLOW  0x2  // 0010
#define EPD_6COLOR_RED     0x3  // 0011
#define EPD_6COLOR_BLUE    0x5  // 0101
#define EPD_6COLOR_GREEN   0x6  // 0110

class Epd7in3f_6Color : public EpdIf {
public:
    Epd7in3f_6Color();
    ~Epd7in3f_6Color();
    int Init(void);
    bool InitFramebufferSPIFFS(UBYTE fill_color = EPD_6COLOR_WHITE);
    void SetFullWindow();
    void Reset(void);
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);
    void EPD_6COLOR_BusyHigh(void);
    void TurnOnDisplay(void);
    void EPD_6COLOR_Display(const UBYTE *image);
    void Clear(UBYTE color);
    void Sleep(void);

    // 🆕 Streaming / Region Drawing API
    void BeginSerialStream(Stream& s, int x, int y, int w, int h);
    void ProcessSerialStream();
    bool OverlayImage(const UBYTE* image, int x, int y, int w, int h);
    void RefreshDisplay(void);
    void ClearNoRefresh(UBYTE color);

private:
    unsigned int reset_pin;
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int busy_pin;
    unsigned long width;
    unsigned long height;
    Stream* serial_stream = nullptr;
    int stream_x = 0, stream_y = 0, stream_w = 0, stream_h = 0;
    int current_row = 0;
};

#endif