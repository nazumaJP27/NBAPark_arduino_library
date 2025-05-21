/*
 * NBA Park Arduino Library
 * Description: Example program demonstrating some of the possible shapes that the Ucglib can draw on the ILI9341 240x320 TFT Display.
                Uses rand() to dynamically change the coordinates and shapes. Ucglib Reference: https://github.com/olikraus/ucglib/wiki/reference
 * Author: José Paulo Seibt Neto
 * Created: May - 2025
 * Last Modified: May - 2025
*/

// Activate sketch level serial debug and Ucglib display debug
#define DEBUG_LEVEL 1
#define DEBUG_DISPLAY 2

#include <NBAPark.h>
#include <SPI.h>
#include <time.h>   // srand, time
#include <Ucglib.h>

#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8

// Prototypes
void draw_rand_shape();

// Instantiating Ucg object for ILI9341
// Ucglib_ILI9341_18x240x320_HWSPI( uint8_t cd, uint8_t cs = UCG_PIN_VAL_NONE, uint8_t reset = UCG_PIN_VAL_NONE)
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

void setup()
{
    Serial.begin(115200);
    srand(time(nullptr));

    // Select a type of text background:
    //ucg.begin(UCG_FONT_MODE_TRANSPARENT);  // Does not overwrite background
    ucg.begin(UCG_FONT_MODE_SOLID);          // Write a background for the text

    // Fill screen with a black color
    ucg.setColor(0, 0, 0);
    ucg.drawBox(0, 0, ucg.getWidth(), ucg.getHeight());
    ucg.setFont(ucg_font_logisoso42_tr);

    // Rotate the screen by 90, 180 or 270 degree. Depending on the aspect ratio of the display, this will put the display in portrait or landscape mode.
    // void Ucglib::setRotate90(void)
    ucg.setRotate270();

    /* Values and text will have the current color from index 0.
    If the setFontMode is UCG_FONT_MODE_SOLID, then the background color is defined by color index 1 (ucg.setColor(1, r, g, b)). */
    uint8_t str_w;
    char buffer[64];
    ucg.setColor(0, 255, 255, 255); // Text
    ucg.setColor(1, 0, 0, 0);       // Background

    // Write "IT"
    strncpy(buffer, "TI", 2);
    buffer[2] = '\0';
    str_w = ucg.getStrWidth(buffer);
    ucg.setPrintPos((ucg.getWidth() - str_w) / 2, 115);
    ucg.print("IT");

    // Write "NBA Park"
    strncpy(buffer, "NBA Park", 8);
    buffer[8] = '\0';
    str_w = ucg.getStrWidth(buffer);
    ucg.setPrintPos((ucg.getWidth() - str_w) / 2, 165);
    ucg.print("NBA Park");

    delay(500);
    debugDrawCrossCenter();
}

void loop()
{
    draw_rand_shape();
    debugDrawCrossCenter();
}

void draw_rand_shape()
{
    ucg.setColor(rand() % 256, rand() % 256, rand() % 256);

    uint8_t rnum = rand() % 7;
    debugSkt(rnum);

    switch (rnum)
    {
        case 0:
            debugSkt(" -> Drawing circle\n");
            // void Ucglib::drawCircle(ucg_int_t x0, ucg_int_t y0, ucg_int_t rad, uint8_t option)
            ucg.drawCircle(rand() % (ucg.getWidth() + 1), rand() % (ucg.getHeight() + 1), rand() % 100, UCG_DRAW_ALL);
            break;
        case 1:
            debugSkt(" -> Drawing disc\n");
            // void Ucglib::drawDisc(ucg_int_t x0, ucg_int_t y0, ucg_int_t rad, uint8_t option)
            ucg.drawDisc(rand() % (ucg.getWidth() + 1), rand() % (ucg.getHeight() + 1), rand() % 100, UCG_DRAW_ALL);
            break;
        case 2:
            debugSkt(" -> Drawing frame\n");
            // void Ucglib::drawFrame(ucg_int_t x, ucg_int_t y, ucg_int_t w, ucg_int_t h)
            ucg.drawFrame(rand() % (ucg.getWidth() + 1), rand() % (ucg.getHeight() + 1), rand() % 100, rand() % 100);
            break;
        case 3:
            debugSkt(" -> Drawing box\n");
            // void Ucglib::drawBox(ucg_int_t x, ucg_int_t y, ucg_int_t w, ucg_int_t h)
            ucg.drawBox(rand() % (ucg.getWidth() + 1), rand() % (ucg.getHeight() + 1), rand() % 100, rand() % 100);
            break;
        case 4:
            debugSkt(" -> Drawing gradient box\n");
            // Rand the remaining color indexes
            ucg.setColor(1, rand() % 256, rand() % 256, rand() % 256);
            ucg.setColor(2, rand() % 256, rand() % 256, rand() % 256);
            ucg.setColor(3, rand() % 256, rand() % 256, rand() % 256);
            // void Ucglib::drawGradientBox(ucg_int_t x, ucg_int_t y, ucg_int_t w, ucg_int_t h)
            ucg.drawGradientBox(rand() % (ucg.getWidth() + 1), rand() % (ucg.getHeight() + 1), rand() % 100, rand() % 100);
            break;
        case 5:
            debugSkt(" -> Drawing gradient line\n");
            // Rand the remaining color indexes
            ucg.setColor(1, rand() % 256, rand() % 256, rand() % 256);
            ucg.setColor(2, rand() % 256, rand() % 256, rand() % 256);
            ucg.setColor(3, rand() % 256, rand() % 256, rand() % 256);
            // void Ucglib::drawGradientLine(ucg_int_t x, ucg_int_t y, ucg_int_t len, ucg_int_t dir)
            ucg.drawGradientLine(rand() % (ucg.getWidth() + 1), rand() % (ucg.getHeight() + 1), rand() % 100, rand() % 4);
            break;
        case 6:
            debugSkt(" -> Drawing triangle\n");
            // void Ucglib::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
            ucg.drawTriangle(rand() % (ucg.getWidth() + 1), rand() % (ucg.getHeight() + 1), rand() % (ucg.getWidth() + 1), rand() % (ucg.getHeight() + 1), rand() % (ucg.getWidth() + 1), rand() % (ucg.getHeight() + 1));
            break;
    }
}