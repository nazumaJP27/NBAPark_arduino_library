#include <NBAPark.h>

// Create a BasketSensor object and assign two pins to it
BasketSensor basket0(2, 3);

void setup()
{
    // Initialize serial communication for debugging
    Serial.begin(115200);
}

void loop()
{
    Serial.print("Distance: ");
    Serial.print(basket0.get_ultrasonic_distance());
    Serial.print("cm\n");
    
    if (basket0.ball_detected())
    {
        Serial.print("Ball detected!\n");
        delay(1000);
    }

    delay(10);
}