#include <NBAPark.h>

const uint8_t num_hoops = 3;

int trig_pins[] = 2, 4, 6;
int echo_pins[] = 3, 5, 7;

BasketSensor hoops[num_hoops] = 
{
    BasketSensor(trig_pins[0], echo_pins[0]),
    BasketSensor(trig_pins[1], echo_pins[1]),
    BasketSensor(trig_pins[2], echo_pins[2]),
};

Timer timer;



void setup()
{
    Serial.begin(115200);
}

void loop()
{

}