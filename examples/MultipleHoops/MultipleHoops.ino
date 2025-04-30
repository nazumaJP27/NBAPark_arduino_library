/*
 * NBA Park Arduino Library
 * Description: Example program to test BasketSensor, Layout, and MVPHoopsLayouts objects working to demonstrate dynamic sensor check
 * Author: José Paulo Seibt Neto
 * Created: Fev - 2025
 * Last Modified: Apr - 2025
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

const Layout test_layouts[] = {
    Layout(0, Layout::LAYOUT_0),
    Layout(10, Layout::LAYOUT_1),
    Layout(20, Layout::LAYOUT_2),
    Layout(30, Layout::LAYOUT_3),
    Layout(40, Layout::LAYOUT_4),
    Layout(50, Layout::LAYOUT_5),
    Layout(60, Layout::LAYOUT_6),
    Layout(70, Layout::LAYOUT_7),
    Layout(80, Layout::LAYOUT_STOP),
};

// Globals
MVPHoopsLayouts test_mvp(test_layouts, sizeof(test_layouts) / sizeof(test_layouts[0]));
const bool* current_mvp_layout = test_mvp.get_curr_layout();

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
    if (test_mvp.update(now) == MVPHoopsLayouts::MVPState::MVP_GAME_OVER)
    {
        Serial.print("GAME OVER\n");
        delay(1500);
        setup();
    }

    for (int i = 0; i < NUM_MVP_HOOPS; ++i)
    {
        if (current_mvp_layout[i] && hoops[i].ball_detected())
        {
            Serial.print("BALL DETECTED! ball count: ");
            Serial.println(++ball_count);
            delay(1500);
        }
    }

    Serial.print(current_mvp_layout[0]);
    Serial.print(", ");
    Serial.print(current_mvp_layout[1]);
    Serial.print(", ");
    Serial.println(current_mvp_layout[2]);

    delay(10);
}