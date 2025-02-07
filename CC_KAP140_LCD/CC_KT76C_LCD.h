#pragma once

#include "Arduino.h"
#include <U8g2lib.h>
#include "kap140_20b_font_constants.h"
#include "kap140_20b_font.h"

// Horizontal positions
#define KT76_FL_X 20
#define KT76_FL_IND_X KT76_FL_X + 2
#define KT76_SQUAWK_X KT76_FL_X + 150
#define KT76_ALT_X (KT76_SQUAWK_X - 68)
#define KT76_ON_X  (KT76_SQUAWK_X - 40)
#define KT76_R_X  (KT76_SQUAWK_X - 10)

// Vertical positions
#define KT76_MAIN_Y 40
#define KT76_INDICATOR_UPPER_Y (KT76_MAIN_Y - 14)
#define KT76_INDICATOR_LOWER_Y (KT76_MAIN_Y + 7)

#define NEW_CODE_TIMEOUT_MS 3000

// 0 = Off, 1 = Standby,2 = Test, 3 = On, 4 = Alt, 5 = Ground
enum xpdrMode
{
  Off = 0,
  Standby,
  Test,
  On,
  Alt,
  Ground
};


class CC_KT76C_LCD
{
public:
  CC_KT76C_LCD(uint8_t cs, uint8_t dc, uint8_t reset);
  void begin();
  void attach();
  void detach();
  void set(int16_t messageID, char *setPoint);
  void update();

private:
  bool _initialised;
  bool _hasPower = false;

  bool _altTransmitting = false; // Turn on/off the R
  bool _selfTestMode = false;
  xpdrMode _curMode = xpdrMode::Off;
  int _curCompleteCode = 0;
  int _pressureAltFL = 0;
  int _lastNumberPressed = 0;
  bool _clearPressed = false;
  bool _vfrPressed = false;
  bool _vfrAssignPressed = false;
  int _curVFRCode = 1200;
  unsigned long _numChangeTimer = 0;
  bool _altIsTransmitting = false;
  bool _keyPressProcessed = true;
  bool _newCodeInProcess = false;
  int _newCodePlace = 1000;  // What we will multiply the next number by
  int _newCode = 0;  // When user updates code, hold temp value here.
  
  int _curPos = 0; // used with debugging

  U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2;

  //  U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI u8g2;

  void drawDisplay();
  void fillTestPattern();
  void fillCode();
  void fillFL();
  void fillOn();
  void fillSby();
  void processClear();
  void showAltTransmit();
  void processKeyPress();
  void resetCodeUpdate();
};