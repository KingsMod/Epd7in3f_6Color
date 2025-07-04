#include "Epd7in3f_6Color.h"
#include <FS.h>
#include <SPIFFS.h>
#include <base64.hpp>

constexpr const char* FRAMEBUFFER_PATH = "/framebuffer.bin";

Epd7in3f_6Color::Epd7in3f_6Color() {
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
}

Epd7in3f_6Color::~Epd7in3f_6Color() {
}

int Epd7in3f_6Color::Init(void) {
    if (EpdIf::IfInit() != 0) {
        return -1;
    }
    Reset();
    EpdIf::DelayMs(20);
    EPD_6COLOR_BusyHigh();

    SendCommand(0xAA);    // CMDH
    SendData(0x49);
    SendData(0x55);
    SendData(0x20);
    SendData(0x08);
    SendData(0x09);
    SendData(0x18);

    SendCommand(0x01);    // POWER_SETTING
    SendData(0x3F);
    SendData(0x00);
    SendData(0x32);
    SendData(0x2A);
    SendData(0x0E);
    SendData(0x2A);

    SendCommand(0x00);    // PANEL_SETTING
    SendData(0x5F);
    SendData(0x69);

    SendCommand(0x03);    // POWER_OFF_SEQUENCE
    SendData(0x00);
    SendData(0x54);
    SendData(0x00);
    SendData(0x44);

    SendCommand(0x05);    // VCOM_AND_DATA_INTERVAL
    SendData(0x40);
    SendData(0x1F);
    SendData(0x1F);
    SendData(0x2C);

    SendCommand(0x06);    // VGH_AND_VGL
    SendData(0x6F);
    SendData(0x1F);
    SendData(0x1F);
    SendData(0x22);

    SendCommand(0x08);    // VSH1_AND_VSH2
    SendData(0x6F);
    SendData(0x1F);
    SendData(0x1F);
    SendData(0x22);

    SendCommand(0x13);    // IPC
    SendData(0x00);
    SendData(0x04);

    SendCommand(0x30);    // PLL_CONTROL
    SendData(0x3C);

    SendCommand(0x41);    // TSE
    SendData(0x00);

    SendCommand(0x50);    // VCOM_AND_DATA_INTERVAL
    SendData(0x3F);

    SendCommand(0x60);    // TCON_SETTING
    SendData(0x02);
    SendData(0x00);

    SendCommand(0x61);    // RESOLUTION
    SendData(0x03);
    SendData(0x20);
    SendData(0x01);
    SendData(0xE0);

    SendCommand(0x82);    // VCOM_DC_SETTING
    SendData(0x1E);

    SendCommand(0x84);    // VCOM_AND_DATA_INTERVAL
    SendData(0x00);

    SendCommand(0x86);    // AGID
    SendData(0x00);

    SendCommand(0xE3);    // VCOM
    SendData(0x2F);

    SendCommand(0xE0);    // CCSET
    SendData(0x00);

    SendCommand(0xE6);    // TSSET
    SendData(0x00);

    InitFramebufferSPIFFS(0x1);  // add this at the end

    return 0;
}

bool Epd7in3f_6Color::InitFramebufferSPIFFS(UBYTE fill_color) {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed");
        return false;
    }

    // Always reinitialize for now (avoid skipping if file exists)
    File f = SPIFFS.open(FRAMEBUFFER_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("Failed to open framebuffer for writing");
        return false;
    }

    uint8_t row[EPD_WIDTH / 2];
    uint8_t byte = (fill_color << 4) | fill_color;
    memset(row, byte, sizeof(row));

    for (int i = 0; i < EPD_HEIGHT; ++i) {
        f.write(row, sizeof(row));
    }

    f.flush();
    f.close();
    Serial.println("Framebuffer initialized in SPIFFS");
    return true;
}

void Epd7in3f_6Color::Reset(void) {
    EpdIf::DigitalWrite(reset_pin, HIGH);
    EpdIf::DelayMs(20);
    EpdIf::DigitalWrite(reset_pin, LOW);
    EpdIf::DelayMs(1);
    EpdIf::DigitalWrite(reset_pin, HIGH);
    EpdIf::DelayMs(20);
}

void Epd7in3f_6Color::SendCommand(unsigned char command) {
    EpdIf::DigitalWrite(dc_pin, LOW);
    EpdIf::SpiTransfer(command);
}

void Epd7in3f_6Color::SendData(unsigned char data) {
    EpdIf::DigitalWrite(dc_pin, HIGH);
    EpdIf::SpiTransfer(data);
}

bool Epd7in3f_6Color::OverlayImage(const UBYTE* image, int x, int y, int w, int h) {
    File f = SPIFFS.open(FRAMEBUFFER_PATH, "r+");
    if (!f) return false;

    int dest_stride = EPD_WIDTH / 2;
    int src_stride = w / 2;

    for (int row = 0; row < h; ++row) {
        size_t offset = (y + row) * dest_stride + (x / 2);
        f.seek(offset, SeekSet);
        f.write(image + row * src_stride, src_stride);
    }

    f.flush(); 
    f.close();
    return true;
}


void Epd7in3f_6Color::BeginSerialStream(Stream& s, int x, int y, int w, int h) {
    serial_stream = &s;
    stream_x = x;
    stream_y = y;
    stream_w = w;
    stream_h = h;
    current_row = 0;
    base64_buffer = "";
    streaming = true;

    serial_stream->print("READY\n");
}


void Epd7in3f_6Color::ProcessSerialStream() {
    if (!serial_stream || current_row >= stream_h) return;

    String header = serial_stream->readStringUntil('\n');
    if (!header.startsWith("START")) return;

    int index = serial_stream->readStringUntil('\n').toInt();
    int chunk_len = serial_stream->readStringUntil('\n').toInt();

    if (index == -1) {
        serial_stream->("DONE\n)";
        current_row = 0;
        return;
    }

    // Read chunk data
    String chunk = serial_stream->readStringUntil('\n');
    String end_marker = serial_stream->readStringUntil('\n');
    if (!end_marker.startsWith("END")) {
        serial_stream->println("ERROR");
        return;
    }

    // Decode base64 (cast to non-const)
    char* encoded = (char*)malloc(chunk_len);
    memcpy(encoded, chunk.c_str(), chunk_len);

    uint8_t row_bytes[EPD_WIDTH / 2];
    memset(row_bytes, 0, sizeof(row_bytes));
    unsigned int decoded_len = decode_base64((unsigned char*)encoded, chunk_len, row_bytes);
    free(encoded);

    // Write to framebuffer at correct offset
    File f = SPIFFS.open(FRAMEBUFFER_PATH, "r+");
    if (f) {
        int dest_stride = EPD_WIDTH / 2;
        int offset = (stream_y + current_row) * dest_stride + (stream_x / 2);
        f.seek(offset, SeekSet);
        f.write(row_bytes, stream_w / 2);
        f.close();
    }

    current_row++;

    serial_stream->println("ACK");
    serial_stream->println("OK");
}

// Step 2: Trigger the actual display update when ready
void Epd7in3f_6Color::RefreshDisplay(void) {
    File f = SPIFFS.open(FRAMEBUFFER_PATH, FILE_READ);
    if (!f) {
        Serial.println("Failed to open framebuffer for refresh");
        return;
    }

    SetFullWindow();
    SendCommand(0x10);  // DATA_START_TRANSMISSION

    uint8_t buffer[64];
    while (f.available()) {
        size_t len = f.read(buffer, sizeof(buffer));
        for (size_t i = 0; i < len; ++i) {
            SendData(buffer[i]);
        }
    }

    f.close();

    // Now trigger refresh
    SendCommand(0x04);  // POWER_ON
    EPD_6COLOR_BusyHigh();

    SendCommand(0x12);  // DISPLAY_REFRESH
    SendData(0x00);
    EPD_6COLOR_BusyHigh();

    SendCommand(0x02);  // POWER_OFF
    SendData(0x00);
    EPD_6COLOR_BusyHigh();

    Serial.println("Display refreshed");
}

void Epd7in3f_6Color::EPD_6COLOR_BusyHigh(void) {
    while (!EpdIf::DigitalRead(busy_pin)) {
        EpdIf::DelayMs(1);
    }
}



void Epd7in3f_6Color::TurnOnDisplay(void) {
    SendCommand(0x04);  // POWER_ON
    EPD_6COLOR_BusyHigh();

    SendCommand(0x12);  // DISPLAY_REFRESH
    SendData(0x00);
    EPD_6COLOR_BusyHigh();

    SendCommand(0x02);  // POWER_OFF
    SendData(0x00);
    EPD_6COLOR_BusyHigh();
}

void Epd7in3f_6Color::EPD_6COLOR_Display(const UBYTE *image) {
    SendCommand(0x10);  // DATA_START_TRANSMISSION_1
    for (unsigned long i = 0; i < height; i++) {
        for (unsigned long j = 0; j < width / 2; j++) {
            SendData(image[j + (width / 2) * i]);
        }
    }
    TurnOnDisplay();
}

void Epd7in3f_6Color::SetFullWindow() {
    SendCommand(0x44);  // SET_RAM_X_ADDRESS_START_END_POSITION
    SendData(0x00);                         // X start = 0
    SendData((EPD_WIDTH / 2) - 1);          // X end = width/2 - 1

    SendCommand(0x45);  // SET_RAM_Y_ADDRESS_START_END_POSITION
    SendData(0x00);                         // Y start = 0
    SendData(0x00);
    SendData((EPD_HEIGHT - 1) & 0xFF);      // Y end (low byte)
    SendData(((EPD_HEIGHT - 1) >> 8) & 0xFF); // Y end (high byte)

    // Set current RAM pointer
    SendCommand(0x4E);  // SET_RAM_X_ADDRESS_COUNTER
    SendData(0x00);
    SendCommand(0x4F);  // SET_RAM_Y_ADDRESS_COUNTER
    SendData(0x00);
    SendData(0x00);
}

void Epd7in3f_6Color::ClearNoRefresh(UBYTE color) {
    File f = SPIFFS.open(FRAMEBUFFER_PATH, "r+");
    if (!f) return;

    uint8_t row[EPD_WIDTH / 2];
    uint8_t byte = (color << 4) | color;
    memset(row, byte, sizeof(row));

    for (int i = 0; i < EPD_HEIGHT; ++i) {
        f.write(row, sizeof(row));
    }
}


void Epd7in3f_6Color::Clear(UBYTE color) {
    ClearNoRefresh(color);
    RefreshDisplay();
}

void Epd7in3f_6Color::Sleep(void) {
    SendCommand(0x07);  // DEEP_SLEEP
    SendData(0xA5);
    EpdIf::DelayMs(10);
    EpdIf::DigitalWrite(RST_PIN, LOW);
}