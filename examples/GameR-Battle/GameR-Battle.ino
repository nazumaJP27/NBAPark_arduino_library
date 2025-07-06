/*
 * NBA Park Arduino Library
 * Description: Example program that handles a display ILI9341 240x320, as a scoreboard with game timer and score for two
                playing teams (West vs. East), two infrared sensors detecting when a ball pass through a hoop, and a button
                to interact with the program. The program is part of an attraction on the NBA Park, by now called project
                R-Battle, a table top basketball game where each player (up to 4) is in control of a Eilik/Panxer battle
                tank (see https://www.youtube.com/watch?v=GOyO6Ep63zQ). The program begins on a menu screen, whaiting
                for a button press to start the game, and returns to the menu when a game is over. The table have a
                hole on each side, with the display updating accordingly, tracking scores and the timer in game, and on
                the winners and menu screens. (Work in Progress)
 * Author: José Paulo Seibt Neto
 * Created: May - 2025
 * Last Modified: Jul - 2025
*/

#define DEBUG_DISPLAY 0 // 0 = None, 2 = active

// Pin number used to trigger the board RESET pin
#define RSTPIN A0
// Display, IR sensors, and buzzer GPIO pin numbers
#define BUTTON A5
#define BUZZER 2
#define IR_0 3
#define IR_1 4
/* LED strips will be wired to the NC gate. At the end of each game, the loser conference
   will have it's side turned off throughout the winner's screen time */
#define RELAY_LED_0 5
#define RELAY_LED_1 6
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 8

#include <NBAPark.h>
#include <SPI.h>
#include <time.h>
#include <Ucglib.h>

// Coordinates for the elements drawn to the display
struct ElementCoordinate
{
    static const int16_t EST_X = 60;
    static const int16_t WST_X = 261;
    static const int16_t CONF_Y = 90;
    static const int16_t SCORE_Y = 150;
    static const int16_t CLOCK_Y = 20;
    static int16_t       CLOCK_X; // Initialized after Ucglib object setup
};

int16_t ElementCoordinate::CLOCK_X;

// Ucglib_ILI9341_18x240x320_HWSPI( uint8_t cd, uint8_t cs = UCG_PIN_VAL_NONE, uint8_t reset = UCG_PIN_VAL_NONE)
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Score related
uint8_t est_score;
uint8_t wst_score;
uint8_t last_est_score; // Score in the last time the display was updated
uint8_t last_wst_score; // Score in the last time the display was updated

// Clock related
uint32_t match_duration;
Clock c;
uint16_t now;
uint16_t reset_trigger_time;
uint8_t last_mm;
uint8_t last_ss;

Button button(BUTTON);
int16_t buzzer_feedback;

// Prototypes
void menu_screen();
void game_reset();
void game_over();
void winner_screen(uint8_t in_conf);
void highlight_frame(int16_t in_x, int16_t in_y, int16_t in_width, int16_t in_height, uint32_t in_timeout, uint8_t in_bgR, uint8_t in_bgG, uint8_t in_bgB);
void timer_setup();
void draw_static_elements();
void draw_clock_dynamic();
void draw_field_digits(uint8_t in_field);
void draw_score_dynamic();

void setup()
{
    digitalWrite(RSTPIN, HIGH); // Keep a weakly HIGH state on RSTPIN as the board RESET pin only triggers when it is pulled LOW

    Serial.begin(115200);

    pinMode(BUZZER, OUTPUT);
    pinMode(IR_0, INPUT);    // EST side
    pinMode(IR_1, INPUT);    // WST side 
    pinMode(RELAY_LED_0, OUTPUT); // EST side
    pinMode(RELAY_LED_1, OUTPUT); // WST side

    digitalWrite(RELAY_LED_0, HIGH);
    digitalWrite(RELAY_LED_1, HIGH);

    est_score = 0;
    wst_score = 0;
    last_est_score = 0;
    last_wst_score = 0;
    last_mm = 255;
    last_ss = 255;

    match_duration = R_BATTLE_DEFAULT_MATCH_DUR;

    /*----------------------------DISPLAY SETUP----------------------------*/
    /* Values and text will have the current color from index 0.
    If the setFontMode is UCG_FONT_MODE_SOLID, then the background color is defined by color index 1 (ucg.setColor(1, r, g, b)). */
    ucg.begin(UCG_FONT_MODE_SOLID); // Write a background for the text
    ucg.setColor(0, 255, 255, 255); // Text
    ucg.setColor(1, 0, 0, 0);       // Background
    ucg.setFontPosTop();

    // Rotate the screen by 90, 180 or 270 degree. Depending on the aspect ratio of the display, this will put the display in portrait or landscape mode.
    ucg.setRotate270();

    // Fills the screen with the color set at color index 1, which is the background color (NEED TESTING)
    ucg.clearScreen();

    debugDrawCrossCenter();
    debugDrawInnerCross();
    /*----------------------------DISPLAY SETUP (end)----------------------------*/

    // Initialize x coordinate for the clock
    ucg.setFont(ucg_font_freedoomr25_tn); // Setting the font used for the timer digits
    ElementCoordinate::CLOCK_X = DSP_H_CENTER - ucg.getStrWidth("00:00"); // String width not divided by 2, because will be scaled 2x

    // Set the default font for the project
    ucg.setFont(ucg_font_logisoso42_tr);

    menu_screen();
    c.run();
}

void loop()
{
    now = c.update();
    button.update();

    // Check buzzer feedback sound logic
    buzzer_feedback = (button.curr_press_dur < R_BATTLE_HARD_RESET_TRIGGER)
                       ? button.curr_press_dur - R_BATTLE_RESET_TRIGGER
                       : button.curr_press_dur - R_BATTLE_HARD_RESET_TRIGGER;

    if (buzzer_feedback > 0 && buzzer_feedback < R_BATTLE_BUZZER_FEEDBACK_DUR)
    {   // Activate buzzer feedback sound for R_BATTLE_BUZZER_FEEDBACK_DUR milliseconds
        digitalWrite(BUZZER, HIGH);
    }
    else
    {
        digitalWrite(BUZZER, LOW);
    }

    // Check if button was released in the reset window
    if ( button.release_time > R_BATTLE_RESET_TRIGGER
         && button.release_time < R_BATTLE_RESET_TRIGGER + BUTTON_RELEASE_WINDOW )
    {
        debugSkt("relese_time= "); debugSkt(button.release_time); debugSkt(" - reset\n");
        menu_screen();
    }
    else if ( button.release_time > R_BATTLE_HARD_RESET_TRIGGER
              && button.release_time < R_BATTLE_HARD_RESET_TRIGGER + BUTTON_RELEASE_WINDOW)
    {
        debugSkt("relese_time= "); debugSkt(button.release_time); debugSkt(" - hard reset\n");
        // Hard reset triggered, reseting board
        pinMode(RSTPIN, OUTPUT);
        digitalWrite(RSTPIN, LOW);
    }

    debugSkt("button press duration= "); debugSkt(button.curr_press_dur); debugSktln();
    debugSkt("relese_time= "); debugSkt(button.release_time); debugSktln();

    if (c.is_running())
    {
        draw_clock_dynamic();
        
        // Check IR sensors for points
        if (digitalRead(IR_0) == LOW)
        {   // WST scored
            c.stop();
            button.reset();

            ++wst_score;
            draw_score_dynamic();
            highlight_frame(ElementCoordinate::WST_X - 43, ElementCoordinate::CONF_Y - 5, 90, 55, R_BATTLE_SCORER_TIMEOUT, 255, 0, 0); // WST scored
            c.run();
        }
        else if (digitalRead(IR_1) == LOW)
        {   // EST scored
            c.stop();
            button.reset();

            ++est_score;
            draw_score_dynamic();
            highlight_frame(ElementCoordinate::EST_X / 2 - 15, ElementCoordinate::CONF_Y - 5, 90, 55, R_BATTLE_SCORER_TIMEOUT, 0, 0, 222); // EST scored
            c.run();
        }
    }
    else
    {   // Draw updated elements one last time and make sure the buzzer is not active
        digitalWrite(BUZZER, LOW);
        draw_clock_dynamic();
        draw_score_dynamic();
        game_over();
    }

    debugDrawCrossCenter();
    debugDrawInnerCross();
}


/*-------------------FUNCTION DEFINITIONS----------------------*/
void menu_screen()
{
    ucg.setFont(ucg_font_logisoso42_tr); // Make sure default font is set

    int16_t str_width;
    char str_buff[12]; // bighest string stored "PRESS START" (11 chars)

    // Draw background
    ucg.setColor(0, 0, 0, 222);
    ucg.setColor(1, 0, 0, 222);
    ucg.drawBox(0, 0, ucg.getWidth(), ucg.getHeight());

    // Draw NBA
    ucg.setColor(0, 255, 200, 200);
    strncpy(str_buff, "NBA", sizeof(str_buff));
    str_width = ucg.getStrWidth(str_buff);
    ucg.drawString(DSP_H_CENTER - str_width / 2 - 2, 48, 0, str_buff);
    ucg.setColor(0, 255, 255, 255);
    ucg.drawString(DSP_H_CENTER - str_width / 2, 44, 0, str_buff);

    // Draw R-BATTLE
    ucg.setColor(0, 255, 200, 200);
    strncpy(str_buff, "R-BATTLE", sizeof(str_buff));
    str_width = ucg.getStrWidth(str_buff);
    ucg.drawString(DSP_H_CENTER - str_width / 2 - 3, 100, 0, str_buff);
    ucg.setColor(0, 255, 255, 255);
    ucg.drawString(DSP_H_CENTER - str_width / 2, 96, 0, str_buff);

    // Set variables to change the colors of the borders and the PRESS START in rapid succession
    uint8_t g = 0;
    uint8_t b = 0;
    strncpy(str_buff, "PRESS START", sizeof(str_buff));
    ucg.setFont(ucg_font_freedoomr10_tr);
    str_width = ucg.getStrWidth(str_buff);

    /* Set button release time to zero to break from the loop when value greater than zero.
       Better than to use button_state because of the constant drawings, making it always responsive regardless the drawing time.
       But still the button state is checked between the major drawings. */
    button.release_time = 0;
    do
    {   // Borders and PRESS START changing colors
        button.update();

        // Draw outter frames
        ucg.setColor(0, 255, g, b);
        ucg.drawFrame(4, 4, ucg.getWidth() - 8, ucg.getHeight() - 8);
        ucg.drawFrame(6, 6, ucg.getWidth() - 12, ucg.getHeight() - 12);
        ucg.drawFrame(12, 12, ucg.getWidth() - 24, ucg.getHeight() - 24);
        ucg.drawFrame(14, 14, ucg.getWidth() - 28, ucg.getHeight() - 28);

        g = (g == 0 ? 255 : 0);
        b = (b == 0 ? 255 : 0);

        button.update();

        // Draw inner frame and PRESS START
        ucg.setColor(0, 255, g, b);
        ucg.drawFrame(8, 8, ucg.getWidth() - 16, ucg.getHeight() - 16);
        ucg.drawFrame(10, 10, ucg.getWidth() - 20, ucg.getHeight() - 20);
        ucg.drawString(DSP_H_CENTER - str_width / 2, 180, 0, str_buff);

        button.update();
        debugSkt("relese_time= "); debugSkt(button.release_time); debugSktln();
    }
    while (!button.state); // Break if button is pressed

    // Check button hold trigger for timer setup or hard reset
    while (!button.release_time)
    {
        button.update();
        // Update a buzzer feedback window
        buzzer_feedback = (button.curr_press_dur < R_BATTLE_HARD_RESET_TRIGGER)
                           ? button.curr_press_dur - R_BATTLE_RESET_TRIGGER
                           : button.curr_press_dur - R_BATTLE_HARD_RESET_TRIGGER;
        
        debugSkt("buzzer_feedback="); debugSkt(buzzer_feedback); debugSktln();

        if (buzzer_feedback > 0 && buzzer_feedback < R_BATTLE_BUZZER_FEEDBACK_DUR)
        {   // Activate buzzer feedback sound for R_BATTLE_BUZZER_FEEDBACK_DUR milliseconds
            digitalWrite(BUZZER, HIGH);
        }
        else
        {
            digitalWrite(BUZZER, LOW);
        }
    }
    digitalWrite(BUZZER, LOW); // Make sure buzzer is not activated

    if ( button.release_time >= R_BATTLE_RESET_TRIGGER
         && button.release_time <= R_BATTLE_RESET_TRIGGER + BUTTON_RELEASE_WINDOW )
    {   // Timer setup triggered
        timer_setup();
    }
    else if ( button.release_time >= R_BATTLE_HARD_RESET_TRIGGER
              && button.release_time <= R_BATTLE_HARD_RESET_TRIGGER + BUTTON_RELEASE_WINDOW )
    {   // Hard reset triggered, reseting board
        pinMode(RSTPIN, OUTPUT);
        digitalWrite(RSTPIN, LOW);
    }
    else
    {   // Start a game
        // Reset to standard font and color
        ucg.setColor(0, 255, 255, 255);
        ucg.setFont(ucg_font_logisoso42_tr);

        game_reset();
    }
}

void game_reset()
{
    // Reset Clock instance
    c.setup(1, match_duration);
    button.reset();

    est_score = 0;
    wst_score = 0;

    // Set to the uint8_t max value, making the draw functions update the elements
    last_est_score = 255;
    last_wst_score = 255;
    last_mm = 255;
    last_ss = 255;

    // Redraw elements - on game start
    draw_static_elements();
    draw_clock_dynamic();
    draw_score_dynamic();

    c.run();
}

void game_over()
{
    // Buzzer sound
    digitalWrite(BUZZER, HIGH);
    delay(500);
    digitalWrite(BUZZER, LOW);

    // Highlight the winner
    if (est_score > wst_score)
    {   // The winner is East
        winner_screen(0);
        menu_screen();
    }
    else if (wst_score > est_score)
    {   // The winner is West
        winner_screen(1);
        menu_screen();
    }
    else
    {   // Overtime
        // Beep to signal the overtime
        for (uint8_t i = 0; i < 3; ++i)
        {
            delay(100);
            digitalWrite(BUZZER, HIGH);
            delay(100);
            digitalWrite(BUZZER, LOW);
        }

        c.setup(1, R_BATTLE_OVERTIME);
        c.run();
    }
}

/* Draw three square frames (coordinates and dimension passed as arguments) that switch colors for the duration of the received in_timeout (milliseconds).
   The function stays on a loop for all timeout duration, "highlighting" the area; then "erases" the lines by drawing three square frames with
   the in_bgR, in_bgG, in_bgB color combination in the same location of the previous frame drawings. Those arguments should match the background color. */
void highlight_frame(int16_t in_x, int16_t in_y, int16_t in_width, int16_t in_height, uint32_t in_timeout, uint8_t in_bgR, uint8_t in_bgG, uint8_t in_bgB)
{
    // Set variables to change the colors of the frames in rapid succession
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;

    Timer scorer_timer;
    do
    {
        // Draw outter frame
        ucg.setColor(0, r, g, b);
        ucg.drawFrame(in_x, in_y, in_width, in_height);
        ucg.drawFrame(in_x + 2, in_y + 2, in_width - 4, in_height - 4); 

        r = (r != 255) ? 255 : 235;
        g = (g != 255) ? 255 : 20;
        b = (b != 255) ? 255 : 235;

        // Draw inner frame
        ucg.setColor(0, r, g, b);
        ucg.drawFrame(in_x + 1, in_y + 1, in_width - 2, in_height - 2); 
        delay(50);
    }
    while (scorer_timer.get_elapsed_time(false) < in_timeout);

    // Clear the frame with the background color of the scorer conference
    ucg.setColor(0, in_bgR, in_bgG, in_bgB);
    ucg.drawFrame(in_x, in_y, in_width, in_height);
    ucg.drawFrame(in_x + 2, in_y + 2, in_width - 4, in_height - 4);
    ucg.drawFrame(in_x + 1, in_y + 1, in_width - 2, in_height - 2);

    // Reset to standard color
    ucg.setColor(0, 255, 255, 255);
}

// Configuration screen to setup the game match duration
void timer_setup()
{
    ucg.setFont(ucg_font_logisoso42_tr); // Make sure that the default font is set

    // Draw background
    ucg.setColor(0, 0, 0, 222);
    ucg.setColor(1, 0, 0, 222);
    ucg.drawBox(0, 0, ucg.getWidth(), ucg.getHeight());
    
    // Draw header text
    ucg.setColor(0, 251, 251, 251);
    ucg.drawString(DSP_H_CENTER - ucg.getStrWidth("TIMER SETUP") / 2 - 2, 23, 0, "TIMER SETUP");
    ucg.setColor(0, 255, 255, 255);
    ucg.drawString(DSP_H_CENTER - ucg.getStrWidth("TIMER SETUP") / 2, 20, 0, "TIMER SETUP");

    // Draw "min" and "secs" caption
    ucg.setFont(ucg_font_freedoomr10_tr);
    ucg.setColor(0, 255, 0, 0);
    ucg.drawString(72, 90, 0, "MINUTES");
    ucg.drawString(193, 90, 0, "SECONDS");

    // Coordinates for the clock digits
    int16_t y = 120;
    int16_t x_mm = 60;  // X for the minutes digits
    int16_t x_ss = 182; // X for the seconds digits

    ucg.setFont(ucg_font_logisoso62_tn);
    ucg.setColor(0, 255, 255, 255);
    ucg.drawGlyph(DSP_H_CENTER - ucg.getStrWidth(":") / 2, y, 0, ':');

    // Instantiate a Clock obj with the current match duration
    Clock setup_clock(1, match_duration);
    uint8_t mm_value = setup_clock.get_mm();
    uint8_t ss_value = setup_clock.get_ss();

    // Draw first state of the field digits (mm:ss)
    char digits_buff[3];

    // Define lambda function to prefix the clock field values with zero (if < 10) and draw at the coordinates passed by args
    auto draw_clock_field = [&digits_buff] (int16_t in_x, int16_t in_y, uint8_t in_field_value)
    {
        itoa(in_field_value, digits_buff, 10);
        if (in_field_value < 10)
        {   // With zero prefix for numbers 0-9, e.g. "07"
            char tmp = digits_buff[0];
            digits_buff[0] = '0';
            digits_buff[1] = tmp;
            digits_buff[2] = '\0';
        }
        else
        {   // Numbers 10-99, e.g. "27"
            digits_buff[2] = '\0';
        }

        int16_t eraser_width = ucg.getStrWidth(digits_buff);
        ucg.setColor(0, 0, 0, 222);
        ucg.drawBox(in_x, in_y, eraser_width, ucg.getFontAscent() + 2);
        ucg.setColor(0, 255, 255, 255);
        ucg.drawString(in_x, in_y, 0, digits_buff);
    };

    // Draw minutes and seconds digits
    draw_clock_field(x_mm, y, mm_value);
    draw_clock_field(x_ss, y, ss_value);

    ucg.setColor(0, 255, 255, 255);
    ucg.drawString(x_ss, y, 0, digits_buff);

    uint8_t g = 255;
    Timer frame_color_timer;

    // Setting the minutes
    frame_color_timer.reset();
    button.reset();
    while(1)
    {
        // Check if enogh time has passed to change frame colors
        if (frame_color_timer.get_elapsed_time(false) > 100)
        {   // Change the color of the hightlight frame and reset the timer
            g = (g != 255) ? 255 : 0;
            frame_color_timer.reset();
        }
        ucg.setColor(0, 255, g, 255);
        ucg.drawFrame(x_mm - 10, y - 10, ucg.getStrWidth("00") + 20, ucg.getFontAscent() + 20);

        button.release_time = 0;
        button.update();
        debugSkt(button.release_time); debugSktln();

        if (button.release_time > 0 && button.release_time < 500)
        {   // Update mm_value and redraw the clock digits
            mm_value = (mm_value < 20) ? ++mm_value : 0;
            draw_clock_field(x_mm, y, mm_value);
        }
        else if (button.release_time >= 500 && button.release_time < UINT16_MAX)
        {   // Button held for a second or more
            break;
        }
    }

    // Erase hightlight frame
    ucg.setColor(0, 0, 0, 222);
    ucg.drawFrame(x_mm - 10, y - 10, ucg.getStrWidth("00") + 20, ucg.getFontAscent() + 20);

    // Setting the seconds field
    frame_color_timer.reset();
    button.reset();
    while(1)
    {
        // Check if enogh time has passed to change frame colors
        if (frame_color_timer.get_elapsed_time(false) > 100)
        {   // Change the color of the hightlight frame and reset the timer
            g = (g != 255) ? 255 : 0;
            //b = (b != 255) ? 255 : 175;
            frame_color_timer.reset();
        }
        ucg.setColor(0, 255, g, 255);
        ucg.drawFrame(x_ss - 10, y - 10, ucg.getStrWidth("00") + 20, ucg.getFontAscent() + 20);

        button.release_time = 0;
        button.update();
        debugSkt(button.release_time); debugSktln();

        if (button.release_time > 0 && button.release_time < 500)
        {   // Update ss_value and redraw the clock digits
            ss_value = (ss_value < 59) ? ++ss_value : 0;
            draw_clock_field(x_ss, y, ss_value);
        }
        else if (button.release_time >= 500 && button.release_time < UINT16_MAX)
        {   // Button held for a second or more
            break;
        }
    }

    // Erase hightlight frame
    ucg.setColor(0, 0, 0, 222);
    ucg.drawFrame(x_ss - 10, y - 10, ucg.getStrWidth("00") + 20, ucg.getFontAscent() + 20);

    // Update match duration variable
    match_duration = mm_value * 60 + ss_value;

    debugDrawCrossCenter();
    debugDrawInnerCross();

    game_reset();
}

// Display information about which one of the conferences win the match
void winner_screen(uint8_t in_conf) // 0 = EST, 1 = WST
{
    char conference_buff[5];

    switch (in_conf)
    {
        case 0:
            // EST conference winner
            strncpy(conference_buff, "EAST", 5);
            ucg.setColor(0, 0, 0, 222);
            ucg.setColor(1, 0, 0, 222);
            // Turn off the loser side LED
            digitalWrite(RELAY_LED_1, LOW);
            break;
        case 1:
            // WST conference winner
            strncpy(conference_buff, "WEST", 5);
            ucg.setColor(0, 255, 0, 0);
            ucg.setColor(1, 255, 0, 0);
            // Turn off the loser side LED
            digitalWrite(RELAY_LED_0, LOW);
    }

    // Fills the screen of the conference color
    ucg.drawBox(0, 0, ucg.getWidth(), ucg.getHeight());

    // Draw "The Winner" (offset first)
    ucg.setColor(0, 245, 245, 245);
    ucg.drawString(DSP_H_CENTER - ucg.getStrWidth("The Winner") / 2 - 2, 31, 0, "The Winner");
    ucg.setColor(0, 255, 255, 255);
    ucg.drawString(DSP_H_CENTER - ucg.getStrWidth("The Winner") / 2, 30, 0, "The Winner");

    // Draw conference name (offset first)
    ucg.setScale2x2();
    ucg.setColor(0, 245, 245, 245);
    ucg.drawString((DSP_H_CENTER - ucg.getStrWidth(conference_buff)) / 2 - 2, 105 / 2, 0, conference_buff);
    ucg.setColor(0, 255, 255, 255);
    ucg.drawString((DSP_H_CENTER - ucg.getStrWidth(conference_buff)) / 2, 100 / 2, 0, conference_buff);
    ucg.undoScale();

    // Draw border outline
    ucg.drawFrame(4, 4, ucg.getWidth() - 8, ucg.getHeight() - 8);
    ucg.setColor(0, 245, 245, 245);
    ucg.drawFrame(2, 2, ucg.getWidth() - 4, ucg.getHeight() - 4);

    delay(4000);

    // Turn on both LED sides
    digitalWrite(RELAY_LED_0, HIGH);
    digitalWrite(RELAY_LED_1, HIGH);

    // Reset to standard font
    ucg.setFont(ucg_font_logisoso42_tr);
}


/*-------------------DRAWER FUNCTIONS----------------------*/
void draw_static_elements()
{
    // Draw background color - EST half (blue) and WST half (red)
    ucg.setColor(0, 0, 0, 222);
    ucg.drawBox(0, 0, DSP_H_CENTER, ucg.getHeight());
    
    ucg.setColor(0, 255, 0, 0);
    ucg.drawBox(DSP_H_CENTER, 0, DSP_H_CENTER, ucg.getHeight());

    ucg.setColor(0, 251, 251, 251);
    ucg.drawVLine(DSP_H_CENTER, 0, ucg.getHeight());

    // Draw clock box
    ucg.setFont(ucg_font_freedoomr25_tn);
    int16_t clock_box_width = ucg.getStrWidth("00:00") * 2;
    ucg.setColor(0, 0, 0, 0);
    ucg.drawBox(ElementCoordinate::CLOCK_X - 6, ElementCoordinate::CLOCK_Y - 6, clock_box_width + 12, ucg.getFontAscent() * 2 + 12);
    ucg.setColor(1, 0, 0, 0);

    // Draw clock frame
    ucg.setColor(0, 235, 235, 235);
    ucg.drawFrame(ElementCoordinate::CLOCK_X - 8, ElementCoordinate::CLOCK_Y - 8, clock_box_width + 16, ucg.getFontAscent() * 2 + 16);
    ucg.setColor(0, 251, 251, 251);
    ucg.drawFrame(ElementCoordinate::CLOCK_X - 6, ElementCoordinate::CLOCK_Y - 6, clock_box_width + 12, ucg.getFontAscent() * 2 + 12);
    ucg.setColor(0, 255, 255, 255);
    ucg.drawFrame(ElementCoordinate::CLOCK_X - 4, ElementCoordinate::CLOCK_Y - 4, clock_box_width + 8, ucg.getFontAscent() * 2 + 8);

    // Draw ':' (scaled 2x) between minutes and seconds, and reset font
    ucg.setScale2x2();
    ucg.setColor(0, 251, 251, 251);
    ucg.drawGlyph((DSP_H_CENTER - ucg.getStrWidth(":") - 3) / 2, (ElementCoordinate::CLOCK_Y / 2) - 3, 0, ':');

    // Reset project default font and color
    ucg.undoScale();
    ucg.setColor(0, 255, 255, 255);
    ucg.setFont(ucg_font_logisoso42_tr);

    // EST conference and score
    ucg.setColor(1, 0, 0, 222); // Set blue background
    ucg.setColor(0, 235, 235, 235);
    ucg.drawString(ElementCoordinate::EST_X - (ucg.getStrWidth("EST") / 2) - 2, ElementCoordinate::CONF_Y + 2, 0, "EST");
    ucg.setColor(0, 251, 251, 251);
    ucg.drawString(ElementCoordinate::EST_X - (ucg.getStrWidth("EST") / 2), ElementCoordinate::CONF_Y, 0, "EST");

    // Draw 'VS' in between the two conferences (offset first)
    // Draw the 'v' on the blue side
    ucg.setColor(0, 235, 235, 235);
    ucg.drawGlyph(DSP_H_CENTER - (ucg.getStrWidth("v")) - 2, 120 + 2, 0, 'v');
    ucg.setColor(0, 251, 251, 251);
    ucg.drawGlyph(DSP_H_CENTER - (ucg.getStrWidth("v")), 120, 0, 'v');

    // Draw the 's' on the red side
    ucg.setColor(1, 255, 0, 0); // Set red background
    ucg.setColor(0, 235, 235, 235);
    ucg.drawGlyph(DSP_H_CENTER + 2, 120 + 2, 0, 's');
    ucg.setColor(0, 251, 251, 251);
    ucg.drawGlyph(DSP_H_CENTER, 120, 0, 's');

    // WST conference and score (offset first)
    ucg.setColor(0, 235, 235, 235);
    ucg.drawString(ElementCoordinate::WST_X - (ucg.getStrWidth("WST") / 2) + 2, ElementCoordinate::CONF_Y + 2, 0, "WST");
    ucg.setColor(0, 251, 251, 251);
    ucg.drawString(ElementCoordinate::WST_X - (ucg.getStrWidth("WST") / 2), ElementCoordinate::CONF_Y, 0, "WST");
}

// Helper that calls the draw function for the field values that changed
void draw_clock_dynamic()
{
    if (c.get_mm() != last_mm)
    {
        last_mm = c.get_mm();
        draw_field_digits(0); // Draw seconds
    }

    if (c.get_ss() != last_ss)
    {
        last_ss = c.get_ss();
        draw_field_digits(1); // Draw seconds
    }
}

// Convert and draw the digits of the clock fields (0 = mm, other = ss)
void draw_field_digits(uint8_t in_field)
{
    ucg.setFont(ucg_font_freedoomr25_tn);
    uint8_t field_value; // mm or ss
    int16_t field_box_width = ucg.getStrWidth("00") * 2; // Used to center the digits and erase previous values -> field_box_width * 2 if scaled
    uint16_t x_pos = ElementCoordinate::CLOCK_X;

    ucg.setColor(1, 0, 0, 0); // Set black background
    ucg.setColor(0, 0, 0, 0); // Set color for the eraser box

    switch (in_field)
    {
        case 0:
            field_value = c.get_mm();
            break;
        default:
            field_value = c.get_ss();
            x_pos += 23 + field_box_width; // Increment horizontal draw coordinate based on the field (hh:mm:ss)
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

    ucg.drawBox(x_pos, ElementCoordinate::CLOCK_Y, field_box_width, ucg.getFontAscent() * 2);
    ucg.setColor(0, 255, 255, 255);

    ucg.setScale2x2();
    ucg.drawString(x_pos / 2, ElementCoordinate::CLOCK_Y / 2, 0, field_buff);

    ucg.undoScale();
    ucg.setColor(0, 255, 255, 255);
}

// Draw and update dynamic elements of the scores
void draw_score_dynamic()
{
    // Change to score digit font
    ucg.setFont(ucg_font_logisoso62_tn);
    int16_t score_box_width = ucg.getStrWidth("000"); // Used erase previous values
    int16_t str_width;

    char score_buff[4];

    // EST conference score
    if (est_score > last_est_score || est_score < last_est_score)
    {   // Need to update score
        itoa(est_score, score_buff, 10);
        str_width = ucg.getStrWidth(score_buff);

        // Erase area of the last draw
        ucg.setColor(0, 0, 222);
        ucg.drawBox(ElementCoordinate::EST_X - score_box_width / 2, ElementCoordinate::SCORE_Y, score_box_width, ucg.getFontAscent() + 2);
        ucg.setColor(1, 0, 0, 222); // Set blue background

        // Draw score count (offset first)
        ucg.setColor(251, 251, 251);
        ucg.drawString(ElementCoordinate::EST_X - str_width / 2 - 2, ElementCoordinate::SCORE_Y + 2, 0, score_buff);
        ucg.setColor(255, 255, 255);
        ucg.drawString(ElementCoordinate::EST_X - str_width / 2, ElementCoordinate::SCORE_Y, 0, score_buff);

        last_est_score = est_score;
    }

    // WST conference score
    if (wst_score > last_wst_score || wst_score < last_wst_score)
    {   // Need to update score
        itoa(wst_score, score_buff, 10);
        str_width = ucg.getStrWidth(score_buff);

        // Erase area of the last draw
        ucg.setColor(255, 0, 0);
        ucg.drawBox(ElementCoordinate::WST_X - score_box_width / 2, ElementCoordinate::SCORE_Y, score_box_width, ucg.getFontAscent() + 2);
        ucg.setColor(1, 255, 0, 0); // Set red background

        // Draw score count (offset first)
        ucg.setColor(251, 251, 251);
        ucg.drawString(ElementCoordinate::WST_X - str_width / 2 + 2, ElementCoordinate::SCORE_Y + 2, 0, score_buff);
        ucg.setColor(255, 255, 255);
        ucg.drawString(ElementCoordinate::WST_X - str_width / 2, ElementCoordinate::SCORE_Y, 0, score_buff);

        last_wst_score = wst_score;
    }

    // Reset to standard font
    ucg.setFont(ucg_font_logisoso42_tr);
}