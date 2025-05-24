/*
 * NBA Park Arduino Library
 * Description: Example program that updates a simple clock with seconds, minutes and hours fields, on the ILI9341 TFT Display.
                Great for showcasing manipulation and overall approach for drawing and updating characters using the Ucglib.
 * Author: José Paulo Seibt Neto
 * Created: May - 2025
 * Last Modified: May - 2025
*/

#define DEBUG_DISPLAY 2 // 0 = None, 2 = active

// Useful for debugging the draw position
#define BG_R 0
#define BG_G 0
#define BG_B 0

// Setup start hours and minutes
#define START_HR 0
#define START_MIN 0

// Display GPIO pin numbers
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8

#include <NBAPark.h>
#include <SPI.h>
#include <Ucglib.h>

// Prototypes
void draw_clock_static();
void draw_clock_dynamic();
void draw_field_digits(uint8_t in_field);

// Globals
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

Timer clock_timer;
uint16_t c_now;
uint8_t c_hr;
uint8_t c_min;
uint8_t c_sec;

char clock_buff[3]; // Store chars for the digits that will be drawn (zero prefix for single digit numbers)

const int16_t digitsHPos = 31; // Setting it to 31 will center the clock
int16_t digitsVPos; // const, but need to be setted after the ucg instance setup
int16_t boxWidth;   // const, but need to be setted after the ucg instance setup


void setup()
{
    Serial.begin(115200);

    c_now = 0;
    c_hr = START_HR;
    c_min = START_MIN;
    c_sec = 0;

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

    clock_timer.reset();
}

void loop()
{
    draw_clock_dynamic();
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
    c_now = clock_timer.get_elapsed_time();
    if (c_now == c_sec) return; // Exit function early

    if (c_now > 59)
    {
        clock_timer.reset();
        c_sec = 0;

        if (++c_min > 59)
        {
            c_min = 0;
            c_hr = (c_hr < 23 ? ++c_hr : 0);
            draw_field_digits(0); // Draw hours
        }

        draw_field_digits(1); // Draw minutes
    }
    else
    {
        c_sec = c_now;
    }

    draw_field_digits(2); // Draw seconds
}

// Convert and draw the digits of the clock fields hh:mm:ss
void draw_field_digits(uint8_t in_field) // 0 = hh, 1 = mm, other = ss
{
    uint8_t field_value;
    uint16_t x_pos = digitsHPos;

    switch (in_field)
    {
        case 0:
            field_value = c_hr;
            break;
        case 1:
            field_value = c_min;
            x_pos += 10 + boxWidth; // Increment horizontal draw coordinate based on the field (hh:mm:ss)
            break;
        default:
            field_value = c_sec;
            x_pos += 20 + boxWidth * 2; // Increment horizontal draw coordinate based on the field (hh:mm:ss)
    }

    itoa(field_value, clock_buff, 10);

    if (field_value < 10)
    {   // With zero prefix for numbers 0-9, e.g. "07"
        char tmp = clock_buff[0];
        clock_buff[0] = '0';
        clock_buff[1] = tmp;
        clock_buff[2] = '\0';
    }
    else
    {   // Numbers 10-99, e.g. "27"
        clock_buff[2] = '\0';
    }

    ucg.setColor(0, BG_R, BG_G, BG_B);
    ucg.drawBox(x_pos, digitsVPos, boxWidth, ucg.getFontAscent() + 2);
    ucg.setColor(0, 255, 255, 255);

    ucg.drawString(x_pos, digitsVPos, 0, clock_buff);
}