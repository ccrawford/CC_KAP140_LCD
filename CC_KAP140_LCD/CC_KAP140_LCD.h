#pragma once

#include "Arduino.h"

// Display dimension constants
#define CHAR_WIDTH 18

// Horizontal positions
#define LEFT_EDGE_X 1
#define AP_X (LEFT_EDGE_X + 62)
#define MID_TEXT_X (AP_X + 24)
#define PT_X (MID_TEXT_X + 62)
#define RIGHT_TEXT_X (PT_X + 11)
#define FPM_X (RIGHT_TEXT_X + 50)
#define FT_X (FPM_X + 32)

// Vertical positions
#define TOP_LINE_Y 26
#define BOT_LINE_Y 63

class CC_KAP140_LCD
{
public:
    CC_KAP140_LCD(uint8_t Pin1, uint8_t Pin2);
    void begin();
    void attach(uint16_t Pin3, char *init);
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:
    bool _initialised;
    bool _apHasPower = false;
    bool _apActive = false;
    bool _selfTestMode = false;
    bool _trimUp = false;
    bool _trimDown = false;
    bool _isRevArm = false;
    bool _isNavArm = false;
    bool _isAprArm = false;
    bool _alertIndicator = false;
    int _rightBlockMode = 0;     // 1: VS, 2: ALT, 3: GS, 4: Baro Mb, 5: Baro In HG
    int _rightBlockData = 0;     // Just a number to display in in the right side info: Altitude, FPM, Baro.
    int _altAlerter = 0;         // Alt alerter altitude
    int _baroMode = 0;           // Are we displaying barometric pressure 0: no, 1: HPA, 2: In Hg
    bool _baro_blinking = false; // used to clear display after baro warning after PFT.
    bool _startPFT = false;      // trigger the pft.
    int _lateralMode = 0;        // 0: none, 1: ROL, 2: HDG, 3: NAV, 4: APR
    int _lateralArmMode = 0;     // 0: none, 1: NAV, 2: APR, 3:REV, 4: GS
    int _verticalArmMode = 0;    // 0: none, 1: ALT
    bool _tempVsMode = false;

    void drawDisplay();
    void fillTestPattern();

    uint8_t _pin1, _pin2, _pin3;
};