/*
 * NBA Park Arduino Library
 * Description: Example program demonstrating some of the possible shapes that the Adafruit GFX library can draw on the ILI9341 240x320 TFT Display.
                Uses rand() to dynamically change the coordinates and shapes. Adafruit_GFX Reference: https://adafruit.github.io/Adafruit-GFX-Library/html/class_adafruit___g_f_x.html
                Adafruit_SPITFT reference (intermediary class between *_GFX and hardware-specific subclasses for different displays): https://adafruit.github.io/Adafruit-GFX-Library/html/class_adafruit___s_p_i_t_f_t.html
 * Author: José Paulo Seibt Neto
 * Created: May - 2025
 * Last Modified: May - 2025
*/

// Activate sketch level serial debug and adafruit lib display debug
#define DEBUG_LEVEL 1
#define DEBUG_DISPLAY 1

#include <NBAPark.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <time.h>   // srand, time

#define TFT_MOSI 11
#define TFT_MISO 12
#define TFT_SCK 13
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8

// Prototypes
void draw_rand_shape();

// Instantiating Adafruit object for ILI9341
// Adafruit_ILI9341(int8_t _CS, int8_t _DC, int8_t _MOSI, int8_t _SCLK, int8_t _RST = -1, int8_t _MISO = -1);
Adafruit_ILI9341 ada(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST, TFT_MISO);

void setup()
{
    Serial.begin(115200);
    srand(time(nullptr));

    ada.begin();

    // Fill screen with a black color
    /* Given 8-bit red, green and blue values, return a 'packed' 16-bit color value in '565' RGB format (5 bits red, 6 bits green, 5 bits blue).
    uint16_t color565 (uint8_t r, uint8_t g, uint8_t b) */
    ada.fillScreen(ada.color565(0, 0, 0));

    // Rotate 0 through 3 corresponding to 4 cardinal rotations
    // void Adafruit_GFX::setRotation(uint8_t x)
    ada.setRotation(3);

    ada.setTextSize(6);
    ada.setTextColor(ada.color565(255, 255, 255));

    // Write "IT"
    ada.setCursor(120, 50);
    ada.print("IT\n");

    // Write "NBA Park"
    ada.setCursor(20, 120);
    ada.print("NBA Park");
    delay(500);
}

void loop()
{
    draw_rand_shape();
    debugDrawCrossCenter();
}

void draw_rand_shape()
{
    uint16_t color = ada.color565(rand() % 256, rand() % 256, rand() % 256);

    uint8_t rnum = rand() % 7;
    debugSkt(rnum);

    switch (rnum)
    {
        case 0:
            debugSkt(" -> Drawing circle\n");
            // void drawCircle (int16_t x0, int16_t y0, int16_t r, uint16_t color)
            ada.drawCircle(rand() % (ada.width() + 1), rand() % (ada.height() + 1), rand() % 100, color);
            break;
        case 1:
            debugSkt(" -> Drawing circle with filled color\n");
            // void fillCircle (int16_t x0, int16_t y0, int16_t r, uint16_t color)
            ada.fillCircle(rand() % (ada.width() + 1), rand() % (ada.height() + 1), rand() % 100, color);
            break;
        case 2:
            debugSkt(" -> Drawing rectangle\n");
            // void drawRect (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
            ada.drawRect(rand() % (ada.width() + 1), rand() % (ada.height() + 1), rand() % 100, rand() % 100, color);
            break;
        case 3:
            debugSkt(" -> Drawing rectangle with filled color\n");
            // void fillRect (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
            ada.fillRect(rand() % (ada.width() + 1), rand() % (ada.height() + 1), rand() % 100, rand() % 100, color);
            break;
        case 4:
            debugSkt(" -> Drawing line\n");
            // void drawLine (int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
            ada.drawLine(rand() % (ada.width() + 1), rand() % (ada.height() + 1), rand() % (ada.width() + 1), rand() % (ada.height() + 1), color);
            break;
        case 5:
            debugSkt(" -> Drawing triangle\n");
            // void drawTriangle (int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
            ada.drawTriangle(rand() % (ada.width() + 1), rand() % (ada.height() + 1), rand() % (ada.width() + 1), rand() % (ada.height() + 1), rand() % (ada.width() + 1), rand() % (ada.height() + 1), color);
            break;
        case 6:
            debugSkt(" -> Drawing triangle with filled color\n");
            // void fillTriangle (int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
            ada.fillTriangle(rand() % (ada.width() + 1), rand() % (ada.height() + 1), rand() % (ada.width() + 1), rand() % (ada.height() + 1), rand() % (ada.width() + 1), rand() % (ada.height() + 1), color);
            break;
    }
}