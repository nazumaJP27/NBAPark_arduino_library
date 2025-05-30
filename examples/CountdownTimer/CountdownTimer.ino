/*
 * NBA Park Arduino Library
 * Description: Example program that updates a countdown timer drawn on an ILI9341 TFT Display, using a Clock instance.
                Showcasing the Clock usage, and overall approach for drawing and updating characters using the Ucglib.
 * Author: José Paulo Seibt Neto
 * Created: May - 2025
 * Last Modified: May - 2025
*/

#define DEBUG_DISPLAY 2 // 0 = None, 2 = active

// Useful for debugging the draw position
#define BG_R 0
#define BG_G 0
#define BG_B 0

// Display GPIO pin numbers
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8

#include <NBAPark.h>
#include <SPI.h>
#include <Ucglib.h>

// Setup start hours, minutes and seconds
#define START_HR 0
#define START_MIN 0
#define START_SEC 0

// Prototypes
void draw_clock_static();
void draw_clock_dynamic();
void draw_field_digits(uint8_t in_field);

// Globals
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

Clock c;
uint8_t last_hh;
uint8_t last_mm;
uint8_t last_ss;

const int16_t digitsHPos = 31; // Setting it to 31 will center the clock
int16_t digitsVPos; // const, but need to be setted after the ucg instance setup
int16_t boxWidth;   // const, but need to be setted after the ucg instance setup


void setup()
{
    Serial.begin(115200);

    c.setup(1, START_HR, START_MIN, START_SEC);
    last_hh = START_HR;
    last_mm = START_MIN;
    last_ss = START_SEC;

    /*----------------------------DISPLAY RELATED----------------------------*/
    /* Values and text will have the current color from index 0.
    If the setFontMode is UCG_FONT_MODE_SOLID, then the background color is defined by color index 1 (ucg.setColor(1, r, g, b)). */
    ucg.begin(UCG_FONT_MODE_SOLID); // Write a background for the text
    ucg.setColor(0, 255, 255, 255); // Text
    ucg.setColor(1, 0, 0, 0);       // Background
    ucg.setFont(ucg_font_logisoso42_tr);
    ucg.setFontPosTop();

    // Fills the screen with the color set at color index 1, which is the background color (NEED TESTING)
    ucg.clearScreen();

    // Rotate the screen by 90, 180 or 270 degree. Depending on the aspect ratio of the display, this will put the display in portrait or landscape mode.
    // void Ucglib::setRotate90(void)
    ucg.setRotate270();

    // Write "IT NBA Park" (two lines)
    ucg.drawString(DSP_H_CENTER - (ucg.getStrWidth("IT") / 2), 70, 0, "IT"); 
    ucg.drawString(DSP_H_CENTER - (ucg.getStrWidth("NBA Park") / 2), 118, 0, "NBA Park");

    delay(500);
    ucg.clearScreen();

    // Change to bigger font
    ucg.setFont(ucg_font_logisoso62_tn);

    // Setup the base coordinates for the digits
    digitsVPos = DSP_V_CENTER - ucg.getFontAscent() / 2;
    boxWidth = ucg.getStrWidth("00");

    // Draw static and first state of the dynamic elements of the clock
    draw_clock_static(); // ':'
    draw_field_digits(0); draw_field_digits(1); draw_field_digits(2); // hh mm ss

    debugDrawCrossCenter();
    debugDrawInnerCross();
}

void loop()
{
    c.update();
    draw_clock_dynamic();
    delay(250);

    debugDrawCrossCenter();
    debugDrawInnerCross();
}


// Draw static elements
void draw_clock_static()
{
    // Draw ':' between hours, minutes and seconds
    ucg.drawGlyph(digitsHPos + boxWidth - 7, digitsVPos, 0, ':');
    ucg.drawGlyph(digitsHPos + boxWidth * 2 + 3, digitsVPos, 0, ':');
}

// Draw and update dynamic elements
void draw_clock_dynamic()
{
    if (c.get_hh() != last_hh)
    {
        last_hh = c.get_hh();
        draw_field_digits(0); // Draw hours
    }

    if (c.get_mm() != last_mm)
    {
        last_mm = c.get_mm();
        draw_field_digits(1); // Draw seconds
    }

    if (c.get_ss() != last_ss)
    {
        last_ss = c.get_ss();
        draw_field_digits(2); // Draw seconds
    }
}

// Convert and draw the digits of the clock fields hh:mm:ss
void draw_field_digits(uint8_t in_field) // 0 = hh, 1 = mm, other = ss
{
    uint8_t field_value;
    uint16_t x_pos = digitsHPos;

    switch (in_field)
    {
        case 0:
            field_value = c.get_hh();
            break;
        case 1:
            field_value = c.get_mm();
            x_pos += 10 + boxWidth; // Increment horizontal draw coordinate based on the field (hh:mm:ss)
            break;
        default:
            field_value = c.get_ss();
            x_pos += 20 + boxWidth * 2; // Increment horizontal draw coordinate based on the field (hh:mm:ss)
    }

    char field_buff[3];
    itoa(field_value, field_buff, 10);

    if (field_value < 10)
    {   // With zero prefix for numbers 0-9, e.g. "07"
        char tmp = field_buff[0];
        field_buff[0] = '0';
        field_buff[1] = tmp;
        field_buff[2] = '\0';
    }
    else
    {   // Numbers 10-99, e.g. "27"
        field_buff[2] = '\0';
    }

    ucg.setColor(0, BG_R, BG_G, BG_B);
    ucg.drawBox(x_pos, digitsVPos, boxWidth, ucg.getFontAscent() + 2);
    ucg.setColor(0, 255, 255, 255);

    ucg.drawString(x_pos, digitsVPos, 0, field_buff);
}