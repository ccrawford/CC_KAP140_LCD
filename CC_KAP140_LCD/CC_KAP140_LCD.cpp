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

    //   u8g2.clearBuffer();
    //   u8g2.drawStr(0, 20, "NAV");
    //   delay(1000);
    //   u8g2.drawStr(0, 200, " 1,200");
    //   u8g2.sendBuffer();
    //   delay(1000);
    //   u8g2.setPowerSave(true);
    //   delay(1000);
    //   u8g2.setPowerSave(false);
    //   delay(1000);
    //   u8g2.sendBuffer();
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
    static unsigned int lastBlink = 0;

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

        u8g2.sendBuffer();
        
        return;
    }

    // if the baro is blinking, only show it
    if (_baro_blinking)
    {
        // blinks like 0.7sec on 0.3 sec off
        if (millis() % 1000 < 700)
        {
            // char debug[20];
            // sprintf(debug, "%ld", millis() % 1000);
            // u8g2.drawStr(0,30, debug);
            int strWidth = u8g2.getStrWidth(formatComma(_rightBlockData));
            u8g2.drawStr(254 - strWidth, TOP_LINE_Y, formatComma(_rightBlockData));
            if (_baroMode == 1)
                u8g2.drawGlyph(RIGHT_TEXT_X + 17, BOT_LINE_Y, 0x21); // HPA
            if (_baroMode == 2)
            {
                u8g2.drawGlyph(FPM_X, BOT_LINE_Y, 0x7E); // IN
                u8g2.drawGlyph(FT_X, BOT_LINE_Y, 0x5E);  // HG
            }
        }
        lastBlink = !lastBlink;
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
        u8g2.drawGlyph(AP_X, TOP_LINE_Y, 0x5B); // AP_box

    // Draw the vertical mode (ALT/VS)
    switch (_rightBlockMode)
    {
    case 1:
        val = "ALT";
        break;
    case 2:
        val = " VS";
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
        u8g2.drawGlyph(PT_X, TOP_LINE_Y - 4, 0x25);
    if (_trimDown)
        u8g2.drawGlyph(PT_X, TOP_LINE_Y + 4, 0x26);

    // Draw the vertical data (right block numeric with comma for 1000s)
    int strWidth = u8g2.getStrWidth(formatComma(_rightBlockData));
    u8g2.drawStr(254 - strWidth, TOP_LINE_Y, formatComma(_rightBlockData));

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
        u8g2.drawGlyph(AP_X, BOT_LINE_Y, 0x5D); // ARM

    // Draw vertical armed mode with ARM marker.
    if (_verticalArmMode == 1)
    {
        u8g2.drawStr(MID_TEXT_X, BOT_LINE_Y, "ALT");
        u8g2.drawGlyph(PT_X, BOT_LINE_Y, 0x5D); // ARM
    }

    // GLYPH SECTION
    // draw ALERT box
    if (_alertIndicator)
    {
        u8g2.drawGlyph(RIGHT_TEXT_X + 8, BOT_LINE_Y, 0x23);
    }

    // Draw appropriate units (FPM / FT )
    // Pressure mode first.
    if (_baroMode)
    {
        if (_baroMode == 1)
            u8g2.drawGlyph(RIGHT_TEXT_X + 17, BOT_LINE_Y, 0x21); // HPA
        if (_baroMode == 2)
        {
            u8g2.drawGlyph(FPM_X, BOT_LINE_Y, 0x7E); // IN
            u8g2.drawGlyph(FT_X, BOT_LINE_Y, 0x5E);  // HG
        }
    }
    else // FT or FPM
    {
        if (_rightBlockMode == 1)
            u8g2.drawGlyph(FPM_X, BOT_LINE_Y, 0x7B); // FPM
        if (_rightBlockMode == 2 || _rightBlockMode == 0)
            u8g2.drawGlyph(FT_X, BOT_LINE_Y, 0x22); // FT
    }

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

void CC_KAP140_LCD::fillTestPattern()
{
    String val;

    // : is the 8+V, _ is the 8 + D, ; is the 8 + I
    val = ":_:";
    u8g2.drawStr(LEFT_EDGE_X, TOP_LINE_Y, val.c_str());

    u8g2.drawGlyph(AP_X, TOP_LINE_Y, 0x5B); // AP_box

    val = "8:;";
    u8g2.drawStr(MID_TEXT_X, TOP_LINE_Y, val.c_str());

    u8g2.drawGlyph(PT_X, TOP_LINE_Y - 4, 0x25); // TRIM UP
    u8g2.drawGlyph(PT_X, TOP_LINE_Y + 4, 0x26); // TRIM DOWN

    val = "88,888";
    u8g2.drawStr(RIGHT_TEXT_X, TOP_LINE_Y, val.c_str());

    val = ":8:";
    u8g2.drawStr(LEFT_EDGE_X, BOT_LINE_Y, val.c_str());
    u8g2.drawGlyph(AP_X, BOT_LINE_Y, 0x5D); // ARM

    val = ";8;";
    u8g2.drawStr(MID_TEXT_X, BOT_LINE_Y, val.c_str());
    u8g2.drawGlyph(PT_X, BOT_LINE_Y, 0x5D); // ARM

    u8g2.drawGlyph(RIGHT_TEXT_X + 8, BOT_LINE_Y, 0x23);  // ALERT
    u8g2.drawGlyph(RIGHT_TEXT_X + 17, BOT_LINE_Y, 0x21); // HPA
    u8g2.drawGlyph(FPM_X, BOT_LINE_Y, 0x7E);             // IN
    u8g2.drawGlyph(FT_X, BOT_LINE_Y, 0x5E);              // HG

    u8g2.drawGlyph(FPM_X, BOT_LINE_Y, 0x7B); // FPM
    u8g2.drawGlyph(FT_X, BOT_LINE_Y, 0x22);  // FT
}