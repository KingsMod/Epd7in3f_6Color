#ifndef EPDIF_H
#define EPDIF_H

#include <Arduino.h>

// Pin definition
#define RST_PIN         16
#define DC_PIN          17
#define CS_PIN          5
#define BUSY_PIN        4

class EpdIf {
public:
    EpdIf(void);
    ~EpdIf(void);

    static int IfInit(void);
    static void DigitalWrite(int pin, int value); 
    static int DigitalRead(int pin);
    static void DelayMs(unsigned int delaytime);
    static void SpiTransfer(unsigned char data);
};

#endif