#include "CC_KT76C_LCD.h"
#include "allocateMem.h"
#include "commandmessenger.h"

#include <cerrno>


// U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8G2_16BIT in u8g2.h

CC_KT76C_LCD::CC_KT76C_LCD(uint8_t cs, uint8_t dc, uint8_t reset) : u8g2(U8G2_R0, /* cs=*/17, /* dc=*/20, /* reset=*/21)
{
}

void CC_KT76C_LCD::begin()
{

    u8g2.begin();
    u8g2.setFont(KAP_140_20b_font); // Use custom font
    u8g2.setFontMode(0);            // Enable transparent mode
    u8g2.setDrawColor(1);           // Note: this is a monochrome display. We can either draw on or off...
    // u8g2.setDisplayRotation(0);  // Set in constructor. R0 is pins on right, narrow part on bottom.

    u8g2.clearBuffer();
    u8g2.drawStr(0, 27, "CC 172 XPDR 0.1");
    u8g2.drawStr(7, 62, "CCRAWFORD.ORG");
    u8g2.sendBuffer();
    delay(2000);

    u8g2.clearBuffer();
    fillTestPattern();
    u8g2.sendBuffer();
    delay(2000);
}

void CC_KT76C_LCD::attach()
{
}

void CC_KT76C_LCD::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

void CC_KT76C_LCD::set(int16_t messageID, char *setPoint)
{
    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -2 will be send from the board when PowerSavingMode is set
            Message will be "0" for leaving and "1" for entering PowerSavingMode
        MessageID == -1 will be send from the connector when Connector stops running
        Put in your code to enter this mode (e.g. clear a display)

    ********************************************************************************** */

    int32_t data = 0;

    if (setPoint == NULL)
    {
        return;
    }
    else
    {
        char *endptr; // Used for error detection with strtol
        errno = 0;    // Reset errno before strtol
        long converted_value = strtol(setPoint, &endptr, 10);

        if (errno == ERANGE)
        {
            // Handle overflow or underflow
            // Serial.print("Error: Input to strtol is out of range in CC_KT76C_LCD::set");
            data = 0; // or a more suitable error value
        }
        else if (*endptr != '\0' && endptr != setPoint) // Check for non-numeric characters
        {
            // String did not represent an integer, handle error
            // Serial.print("Error: Invalid input to strtol, non numeric characters in CC_KT76C_LCD::set ");
            data = 0; // or a more suitable error value
        }
        else
        {
            data = (int32_t)converted_value; // cast back to int32_t
        }
    }

    // do something according your messageID
    switch (messageID)
    {
    case -1:
        // tbd., get's called when Mobiflight shuts down
        break;
    case -2:
        // tbd., get's called when PowerSavingMode is entered
        break;
    case 0: // XPDR power state. NOTE: No power on test sequence for device.
        _hasPower = (data == 1);
        break;
    case 1: // XPDR state (in the enum)
        _curMode = xpdrMode(data);
        break;
    case 2:
        _curCompleteCode = data;
        resetCodeUpdate();
        break;
    case 3: // Altitude
        _pressureAltFL = data;
        break;
    case 4: // Alt Transmit
        _altIsTransmitting = (data == 1);
        break;
    case 5: // Number button press.
        if (data >= 0 && data <= 7) {
            _lastNumberPressed = data;
            _keyPressProcessed = false;
            _newCodeInProcess = true;
            _numChangeTimer = millis();
        }
        break;
    case 6: // CLR button press
        _clearPressed = (data == 1);
        _numChangeTimer = millis();
        break;
    case 7: // VFR button press
        _vfrPressed = (data == 1);
        break;
    case 8: // VFR set mode active
        _vfrAssignPressed = (data == 1);
        break;
    default:
        break;
    }
}

void CC_KT76C_LCD::update()
{
    // Check to see if a new code update has timed out
    if(_newCodeInProcess && ((millis() - _numChangeTimer) > NEW_CODE_TIMEOUT_MS))
    {
        resetCodeUpdate();
    }

    processClear();

    processKeyPress();

    drawDisplay();
}

void CC_KT76C_LCD::resetCodeUpdate()
{
        _newCode = 0;
        _newCodeInProcess = false;
        _newCodePlace = 1000;
        _clearPressed = false;
        _keyPressProcessed = true;
        _numChangeTimer = 0;
}


void CC_KT76C_LCD::processClear()
{
    if(_clearPressed && _newCodeInProcess)
    {
        // Eat one line of entry.
        if(_newCodePlace == 1000) return;
        _newCodePlace = _newCodePlace * 10;
        _newCode = (int)(_newCode/(_newCodePlace * 10)) * (_newCodePlace * 10);  //Truncate with integer math
    }
    // Else don't do anything. Sim will tell us what to do.
    
    _clearPressed = false;

//        resetCodeUpdate();
}

void CC_KT76C_LCD::processKeyPress()
{
    if(_keyPressProcessed) return;

    _newCode = _newCode + (_newCodePlace * _lastNumberPressed);
    if (_newCodePlace == 1)
    {
        _curCompleteCode = _newCode;
        // Entry complete
        resetCodeUpdate();
    }
    else
    {
        _newCodePlace = _newCodePlace / 10;
    }     
    
    _keyPressProcessed = true;
    
}

void CC_KT76C_LCD::drawDisplay()
{
    u8g2.clearBuffer();

    if (_curMode == xpdrMode::Off)
    {
        u8g2.setPowerSave(true);
        return;
    }
    else
    {
        u8g2.setPowerSave(false);
    }

    if (_curMode == xpdrMode::Test)
    {
        fillTestPattern();

        u8g2.sendBuffer();

        return;
    }

    // The current squawk code is shown in SBY, On, Alt modes.

    // Show current xpdr code
    fillCode();

    // Show altitude
    fillFL();

    // Show On mode
    fillOn();

    // Show Standby Mode
    fillSby();

    // Show Transmit
    showAltTransmit();

    u8g2.sendBuffer();
}


void CC_KT76C_LCD::fillCode()
{
    // Four digits, zero padded.
    // During code entry the values fill left to right with - padding. E.g. "1 2 - -"
    char dispStr[8] = "8888";
    
    if (_newCodeInProcess)
    {
// char dat[6];
// sprintf(dat, "%d,", _newCode);
// u8g2.drawStr(_curPos, 64, dat);
// _curPos += 72;
// if (_curPos > 200) _curPos = 0;

        if(_newCodePlace == 1000) sprintf(dispStr, "----");
        if(_newCodePlace == 100) sprintf(dispStr, "%01d---", _newCode/1000);
        if(_newCodePlace == 10) sprintf(dispStr, "%02d--", _newCode/100);
        if(_newCodePlace == 1) sprintf(dispStr, "%03d-", _newCode/10);
    }
    else  sprintf(dispStr, "%04d", _curCompleteCode);

    u8g2.drawStr(KT76_SQUAWK_X, KT76_MAIN_Y, dispStr);

    return;
}

void CC_KT76C_LCD::fillOn()
{
    if(_curMode == xpdrMode::On)
        u8g2.drawGlyph(KT76_ON_X, KT76_INDICATOR_UPPER_Y, GLYPH_ON);
}

void CC_KT76C_LCD::fillSby()
{
    if(_curMode == xpdrMode::Standby)
        u8g2.drawGlyph(KT76_ON_X, KT76_INDICATOR_LOWER_Y, GLYPH_SBY);
}

void CC_KT76C_LCD::showAltTransmit()
{
    if(_altIsTransmitting)
    {
        u8g2.drawGlyph(KT76_R_X, KT76_INDICATOR_UPPER_Y, GLYPH_R);
    }
}
void CC_KT76C_LCD::fillFL()
{
 
    if(_curMode != xpdrMode::Alt) return;
 
    // Draw the altitude.
    int dispValue = _pressureAltFL;

    char dispStr[8];

    if (dispValue >=0)
        sprintf(dispStr, " %03d", dispValue);
    else
        sprintf(dispStr, "%04d", dispValue);


    u8g2.drawStr(KT76_FL_X, KT76_MAIN_Y, dispStr);
    u8g2.drawGlyph(KT76_FL_IND_X, KT76_INDICATOR_LOWER_Y, GLYPH_FL);

    // Draw the ALT indicator
    u8g2.drawGlyph(KT76_ALT_X, KT76_INDICATOR_UPPER_Y, GLYPH_ALT);
}

void CC_KT76C_LCD::fillTestPattern()
{
    String val;

    // FL code
    val = "-888";
    u8g2.drawStr(KT76_FL_X, KT76_MAIN_Y, val.c_str());

    u8g2.drawGlyph(KT76_FL_IND_X, KT76_INDICATOR_LOWER_Y, GLYPH_FL); // FL

    // Squawk code
    val = "8888";
    u8g2.drawStr(KT76_SQUAWK_X, KT76_MAIN_Y, val.c_str());

    // Radio indicator
    u8g2.drawGlyph(KT76_R_X, KT76_INDICATOR_UPPER_Y, GLYPH_R);

    // Alt

    u8g2.drawGlyph(KT76_ALT_X, KT76_INDICATOR_UPPER_Y, GLYPH_ALT);


    // On
    u8g2.drawGlyph(KT76_ON_X, KT76_INDICATOR_UPPER_Y, GLYPH_ON);

    // GND
    u8g2.drawGlyph(KT76_ALT_X, KT76_INDICATOR_LOWER_Y, GLYPH_GND);

    // SBY
    u8g2.drawGlyph(KT76_ON_X, KT76_INDICATOR_LOWER_Y, GLYPH_SBY);

    // Indicators
    val = "????";
    u8g2.drawStr(KT76_SQUAWK_X, KT76_INDICATOR_LOWER_Y, val.c_str());

    return;
}