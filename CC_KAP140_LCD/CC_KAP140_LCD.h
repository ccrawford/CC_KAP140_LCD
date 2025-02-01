#pragma once

#include "Arduino.h"
#include <U8g2lib.h>

// Display dimension constants
#define CHAR_WIDTH 18

// Horizontal positions
#define LEFT_EDGE_X 1
#define AP_X (LEFT_EDGE_X + 64)
#define MID_TEXT_X (AP_X + 22)
#define PT_X (MID_TEXT_X + 62)
#define RIGHT_TEXT_X (PT_X + 11)
#define FPM_X (RIGHT_TEXT_X + 60)
#define FT_X (FPM_X + 23)
#define ALERT_X (RIGHT_TEXT_X + 17)

// These should really be in the font file, but it auto-authors.
#define GLYPH_AP 0x5B
#define GLYPH_FT 0x22
#define GLYPH_PITCHUP 0x25
#define GLYPH_PITCHDN 0x26
#define GLYPH_ARM 0x5D
#define GLYPH_ALERT 0x23
#define GLYPH_IN 0x7E
#define GLYPH_HG 0x5E
#define GLYPH_HPA 0x21
#define GLYPH_FPM 0x7B



// Vertical positions
#define TOP_LINE_Y 27
#define BOT_LINE_Y 62

class CC_KAP140_LCD
{
public:
    CC_KAP140_LCD(uint8_t cs, uint8_t dc, uint8_t reset );
    void begin();
    void attach();
    void detach();
    void set(int16_t messageID, char *setPoint);
    void update();

private:
    bool _initialised;
    bool _apHasPower = false;
    bool _apWasActive = false;
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
    bool _apOffAlertMode = false;
    long unsigned int _apOffAlertTimer = 0;

  //  U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI u8g2;

    void drawDisplay();
    void fillTestPattern();
    void fillRightData();
    void fillUnits();
    void fillSelfTest();
    char *formatComma(int number);

};