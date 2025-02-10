#include <NBAPark.h>

int trig_pins[] = {2, 4, 6};
int echo_pins[] = {3, 5, 7};

// DEFAULT_NUM_MVP_HOOPS == 3
BasketSensor hoops[DEFAULT_NUM_MVP_HOOPS] = {
    BasketSensor(trig_pins[0], echo_pins[0]),
    BasketSensor(trig_pins[1], echo_pins[1]),
    BasketSensor(trig_pins[2], echo_pins[2]),
};

bool valid_layouts[4][DEFAULT_NUM_MVP_HOOPS] = {
    {1, 1, 1},
    {0, 1, 1},
    {1, 0, 1},
    {1, 1, 0},
};

MVPHoopsLayout layouts[5] = {
    MVPHoopsLayout(0, valid_layouts[0]),
    MVPHoopsLayout(10, valid_layouts[1]),
    MVPHoopsLayout(20, valid_layouts[2]),
    MVPHoopsLayout(30, valid_layouts[3]),
    MVPHoopsLayout(40, nullptr),
};

MVPHoopsLayouts::LayoutId id_arr[] = {1, 2, 3};

MVPHoopsLayouts test(id_arr, 3);

uint8_t curr;
uint8_t next;
bool* current_valid_layout = layouts[curr].m_valid_hoops;

Timer timer;
int now;

void setup()
{
    Serial.begin(115200);

    curr = 0;
    next = 1;

    timer.reset_timer();
    int now = timer.get_elapsed_time();
}

void loop()
{
    now = timer.get_elapsed_time();
    Serial.print("Time: ");
    Serial.print(now);
    Serial.print(" seconds | Valid hoops: ");

    if (now > layouts[next].m_time)
    {
        ++curr;
        current_valid_layout = layouts[next].m_valid_hoops;
        if (++next > 4) setup();
    }

    if (current_valid_layout != nullptr) {
        // Print valid hoop states
        Serial.print(current_valid_layout[0]);
        Serial.print(", ");
        Serial.print(current_valid_layout[1]);
        Serial.print(", ");
        Serial.println(current_valid_layout[2]);
    } else {
        Serial.println("No valid layout.");
    }

    delay(10);
}