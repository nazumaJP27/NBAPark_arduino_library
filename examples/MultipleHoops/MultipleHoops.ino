/*
 * NBA Park Arduino Library
 * Description: Example program to test BasketSensor, Layout, and MVPHoops objects working to demonstrate dynamic sensor check
 * Author: José Paulo Seibt Neto
 * Created: Fev - 2025
 * Last Modified: May - 2025
*/

#include <NBAPark.h>

int trig_pins[] = {2, 4, 6};
int echo_pins[] = {3, 5, 7};

// NUM_MVP_HOOPS == 3
BasketSensor hoops[NUM_MVP_HOOPS] = {
    BasketSensor(trig_pins[0], echo_pins[0]),
    BasketSensor(trig_pins[1], echo_pins[1]),
    BasketSensor(trig_pins[2], echo_pins[2]),
};

const MVPHoops::Layout test_layouts[] = {
    MVPHoops::Layout(0, BitmapPattern::LAYOUT_0),
    MVPHoops::Layout(10, BitmapPattern::LAYOUT_1),
    MVPHoops::Layout(20, BitmapPattern::LAYOUT_2),
    MVPHoops::Layout(30, BitmapPattern::LAYOUT_3),
    MVPHoops::Layout(40, BitmapPattern::LAYOUT_4),
    MVPHoops::Layout(50, BitmapPattern::LAYOUT_5),
    MVPHoops::Layout(60, BitmapPattern::LAYOUT_6),
    MVPHoops::Layout(70, BitmapPattern::LAYOUT_7),
    MVPHoops::Layout(80, BitmapPattern::LAYOUT_STOP),
};

// Globals
MVPHoops test_mvp(test_layouts, sizeof(test_layouts) / sizeof(test_layouts[0]));
BitmapPattern curr_mvp_pattern = test_mvp.get_curr_pattern();

Timer timer;
int now;

int ball_count;

void setup()
{
    Serial.begin(115200);

    timer.reset();
    now = timer.get_elapsed_time();

    test_mvp.reset();

    ball_count = 0;
}

void loop()
{
    now = timer.get_elapsed_time();
    Serial.print("Time: ");
    Serial.print(now);
    Serial.print(" seconds | Valid hoops: ");

    // Update layouts state and check for the end of the game
    if (test_mvp.update(now) == MVPHoops::MVPState::MVP_GAME_OVER)
    {
        Serial.print("GAME OVER\n");
        delay(1500);
        setup();
    }

    for (int i = 0; i < NUM_MVP_HOOPS; ++i)
    {
        if (((curr_mvp_pattern >> i) && 1) && hoops[i].ball_detected())
        {
            Serial.print("BALL DETECTED! ball count: ");
            Serial.println(++ball_count);
            delay(1500);
        }
    }

    debugSktVal(curr_mvp_pattern, BIN); debugSktln();
    delay(10);
}