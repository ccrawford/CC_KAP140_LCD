#include "CC_KAP140_LCD.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <cerrno>

#include "kap140_20b_font.h"

String formatWithCommas(int number);
char *formatComma(int number);

// U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);	// Enable U8G2_16BIT in u8g2.h
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/17, /* dc=*/20, /* reset=*/21);

void rightBlockDisplay(int value);

void drawDisplay();

CC_KAP140_LCD::CC_KAP140_LCD(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void CC_KAP140_LCD::begin()
{

    u8g2.begin();
    u8g2.setFont(KAP_140_20b_font); // Use custom font
    u8g2.setFontMode(0);            // Enable transparent mode
    u8g2.setDrawColor(1);           // Note: this is a monochrome display. We can either draw on or off...
    // u8g2.setDisplayRotation(0);  // Set in constructor. R0 is pins on right, narrow part on bottom.

    u8g2.clearBuffer();
    u8g2.drawStr(LEFT_EDGE_X, TOP_LINE_Y, "CC 172 LCD 0.1");
    u8g2.drawStr(LEFT_EDGE_X + 6, BOT_LINE_Y, "CCRAWFORD.ORG");
    u8g2.sendBuffer();
    delay(3000);

    u8g2.clearBuffer();
    fillTestPattern();
    u8g2.sendBuffer();
    delay(3000);
}

void CC_KAP140_LCD::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;
}

void CC_KAP140_LCD::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

void CC_KAP140_LCD::set(int16_t messageID, char *setPoint)
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
            // Serial.print("Error: Input to strtol is out of range in CC_KAP140_LCD::set");
            data = 0; // or a more suitable error value
        }
        else if (*endptr != '\0' && endptr != setPoint) // Check for non-numeric characters
        {
            // String did not represent an integer, handle error
            // Serial.print("Error: Invalid input to strtol, non numeric characters in CC_KAP140_LCD::set ");
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
    case 0:
        _apActive = (data == 1);

        // Check to see if _ap is turning off.
        if (_apActive)
        {
            _apWasActive = true;
            _apOffAlertMode = false;
        }
        // _apOffAlertMode = (!_apActive && _apWasActive);  // if ap is now not active, but was, turn on the alert mode
        if (!_apActive && _apWasActive)
        {
            _apOffAlertMode = true;
            _apOffAlertTimer = millis();
            _apWasActive = false;
        }

        break;
    case 1:
        // Lateral mode
        _lateralMode = data;
        break;
    case 2:
        _rightBlockMode = data;
        break;
    case 3:
        _rightBlockData = data;
        break;
    case 4:
        _lateralArmMode = data;
        break;
    case 5:
        _verticalArmMode = data;
        break;
    case 6:
        _trimUp = false;
        _trimDown = false;
        if (data == 1)
            _trimUp = true;
        if (data == 2)
            _trimDown = true;
        break;
    case 7:
        _alertIndicator = (data == 1);
        break;
    case 8:
        _baroMode = data;
        break;
    case 9:
        _selfTestMode = (data == 1);
        break;
    case 10:
        _baro_blinking = (data == 1);
        break;
    case 11:
        _tempVsMode = (data == 1);
        break;
    case 12:
        _apHasPower = (data == 1);
        break;

    default:
        break;
    }
}

void CC_KAP140_LCD::update()
{
    // Do something which is required regulary
    drawDisplay();
}

void CC_KAP140_LCD::drawDisplay()
{
    u8g2.clearBuffer();

    if (!_apHasPower)
    {
        u8g2.setPowerSave(true);
        return;
    }
    else
    {
        u8g2.setPowerSave(false);
    }

    if (_selfTestMode)
    {
        fillSelfTest();

        u8g2.sendBuffer();

        return;
    }

    // if the baro is blinking, only show it
    if (_baro_blinking)
    {
        // blinks like 0.7sec on 0.3 sec off
        if (millis() % 1000 < 700)
        {
            fillRightData();
            fillUnits();
        }
        u8g2.sendBuffer();
        return;
    }

    // if ap turn off alert mode, blink the AP notification and leave the target alt up.
    if (_apOffAlertMode)
    {
        // Blinks for 5 seconds.
        if (millis() - _apOffAlertTimer > 5000)
        {
            _apOffAlertMode = false;
            return;
        }

        // Target ALT remains solid

        int strWidth = u8g2.getStrWidth(formatComma(_rightBlockData));
        u8g2.drawStr(254 - strWidth, TOP_LINE_Y, formatComma(_rightBlockData));

        u8g2.drawGlyph(FT_X, BOT_LINE_Y, GLYPH_FT); // FT

        // blinks like 0.7sec on 0.3 sec off
        if (millis() % 1000 < 700)
        {
            u8g2.drawStr(LEFT_EDGE_X, TOP_LINE_Y, " AP");
            u8g2.drawGlyph(AP_X, TOP_LINE_Y, GLYPH_AP); // AP_box
        }

        u8g2.sendBuffer();
        return;
    }

    // Moving left to right, top to bottom.
    String val = "";
    // Draw the lateral mode: HDG, APR, ROL, REV.
    switch (_lateralMode)
    {
    case 1:
        val = "ROL";
        break;
    case 2:
        val = "HDG";
        break;
    case 3:
        val = "NAV";
        break;
    case 4:
        val = "APR";
        break;
    case 5:
        val = "REV";
        break;
    default:
        val = "";
        break;
    }

    u8g2.drawStr(LEFT_EDGE_X, TOP_LINE_Y, val.c_str());

    // Draw the AP on symbol
    if (_apActive)
        u8g2.drawGlyph(AP_X, TOP_LINE_Y, GLYPH_AP); // AP_box

    // Draw the vertical mode (ALT/VS)
    switch (_rightBlockMode)
    {
    case 1:
        val = " VS";
        break;
    case 2:
        val = "ALT";
        break;
    case 3:
        val = " GS";
        break;
    default:
        val = "";
        break;
    }

    u8g2.drawStr(MID_TEXT_X, TOP_LINE_Y, val.c_str());

    // Draw Pitch up/down
    if (_trimUp)
        u8g2.drawGlyph(PT_X, TOP_LINE_Y - 4, GLYPH_PITCHUP); // Pitch up
    if (_trimDown)
        u8g2.drawGlyph(PT_X, TOP_LINE_Y + 4, GLYPH_PITCHDN); // Pitch down

    fillRightData();

    // Draw the lateral armed mode nav/apr/rev with ARM marker lower left
    switch (_lateralArmMode)
    {
    case 1:
        val = "NAV";
        break;
    case 2:
        val = "APR";
        break;
    case 3:
        val = "REV";
        break;
    case 4:
        val = " GS";
        break;
    default:
        val = "";
        break;
    }

    u8g2.drawStr(LEFT_EDGE_X, BOT_LINE_Y, val.c_str());
    if (_lateralArmMode)
        u8g2.drawGlyph(AP_X - 5, BOT_LINE_Y, GLYPH_ARM); // ARM

    // Draw vertical armed mode with ARM marker.
    if (_verticalArmMode == 1)
    {
        u8g2.drawStr(MID_TEXT_X, BOT_LINE_Y, "ALT");
        u8g2.drawGlyph(PT_X - 5, BOT_LINE_Y, GLYPH_ARM); // ARM
    }

    // GLYPH SECTION
    // draw ALERT box
    if (_alertIndicator)
    {
        u8g2.drawGlyph(ALERT_X, BOT_LINE_Y, GLYPH_ALERT);
    }

    fillUnits();

    u8g2.sendBuffer();
}

char *formatComma(int number)
{
    static char formatted[10]; // Enough to hold "-99,999\0"
    char buffer[7];            // Holds the input number as a string without commas
    int isNegative = 0;

    // Check if the number is negative
    if (number < 0)
    {
        isNegative = 1;
        number = -number;
    }

    // Convert the number to a string
    snprintf(buffer, sizeof(buffer), "%d", number);

    int len = strlen(buffer);
    int formattedIndex = 0;
    int bufferIndex = 0;

    // Add negative sign if needed
    if (isNegative)
    {
        formatted[formattedIndex++] = '-';
    }

    // Add digits and commas
    for (int i = len; i > 0; i--)
    {
        formatted[formattedIndex++] = buffer[bufferIndex++];
        if (i > 1 && (i - 1) % 3 == 0)
        { // Add comma every 3 digits
            formatted[formattedIndex++] = ',';
        }
    }

    formatted[formattedIndex] = '\0'; // Null-terminate the string

    return formatted;
}

void CC_KAP140_LCD::fillSelfTest()
{
    if (_rightBlockData == 100)
    {
        u8g2.drawStr(MID_TEXT_X, TOP_LINE_Y, "PFT");
        u8g2.drawStr(RIGHT_TEXT_X, TOP_LINE_Y, "1");
    }
    if (_rightBlockData == 200)
    {
        u8g2.drawStr(MID_TEXT_X, TOP_LINE_Y, "PFT");
        u8g2.drawStr(RIGHT_TEXT_X, TOP_LINE_Y, "2");
    }
    if (_rightBlockData == 88888)
    {
        fillTestPattern();
    }
    return;
}

void CC_KAP140_LCD::fillRightData()
{
    // Draw the vertical data (right block numeric with comma for 1000s)
    // Need special handling for the decimal on the in hg baro.
    char rightBlockDisp[12];
    if (_baroMode == 2)
    {
        sprintf(rightBlockDisp, "%02d.%02d ", _rightBlockData / 100, _rightBlockData % 100);
    }
    else
    {
        strcpy(rightBlockDisp, formatComma(_rightBlockData));
    }

    int strWidth = u8g2.getStrWidth(rightBlockDisp);
    u8g2.drawStr(254 - strWidth, TOP_LINE_Y, rightBlockDisp);

    return;
}

void CC_KAP140_LCD::fillUnits()
{
    // Draw appropriate units (FPM / FT )
    // Pressure mode first.
    if (_baroMode)
    {
        if (_baroMode == 1)
            u8g2.drawGlyph(ALERT_X, BOT_LINE_Y, GLYPH_HPA); // HPA
        if (_baroMode == 2)
        {
            u8g2.drawGlyph(FPM_X, BOT_LINE_Y, GLYPH_IN); // IN
            u8g2.drawGlyph(FT_X, BOT_LINE_Y, GLYPH_HG);  // HG
        }
    }
    else // FT or FPM
    {
        if (_tempVsMode)
            u8g2.drawGlyph(FPM_X, BOT_LINE_Y, GLYPH_FPM); // FPM
        else
            u8g2.drawGlyph(FT_X, BOT_LINE_Y, GLYPH_FT); // FT
    }

    return;
}

void CC_KAP140_LCD::fillTestPattern()
{
    String val;

    // : is the 8+V, _ is the 8 + D, ; is the 8 + I
    val = ":_:";
    u8g2.drawStr(LEFT_EDGE_X, TOP_LINE_Y, val.c_str());

    u8g2.drawGlyph(AP_X, TOP_LINE_Y, GLYPH_AP); // AP_box

    val = "8:;";
    u8g2.drawStr(MID_TEXT_X, TOP_LINE_Y, val.c_str());

    u8g2.drawGlyph(PT_X, TOP_LINE_Y - 4, GLYPH_PITCHUP); // TRIM UP
    u8g2.drawGlyph(PT_X, TOP_LINE_Y + 4, GLYPH_PITCHDN); // TRIM DOWN

    val = "88,888";
    u8g2.drawStr(RIGHT_TEXT_X, TOP_LINE_Y, val.c_str());

    val = ":8:";
    u8g2.drawStr(LEFT_EDGE_X, BOT_LINE_Y, val.c_str());
    u8g2.drawGlyph(AP_X - 5, BOT_LINE_Y, GLYPH_ARM); // ARM

    val = ";8;";
    u8g2.drawStr(MID_TEXT_X, BOT_LINE_Y, val.c_str());
    u8g2.drawGlyph(PT_X - 5, BOT_LINE_Y, GLYPH_ARM); // ARM

    u8g2.drawGlyph(ALERT_X, BOT_LINE_Y, GLYPH_ALERT); // ALERT
    u8g2.drawGlyph(ALERT_X, BOT_LINE_Y, GLYPH_HPA);   // HPA
    u8g2.drawGlyph(FPM_X, BOT_LINE_Y, GLYPH_IN);      // IN
    u8g2.drawGlyph(FT_X, BOT_LINE_Y, GLYPH_HG);       // HG

    u8g2.drawGlyph(FPM_X, BOT_LINE_Y, GLYPH_FPM); // FPM
    u8g2.drawGlyph(FT_X, BOT_LINE_Y, GLYPH_FT);   // FT

    return;
}