/*
 * NBA Park Arduino Library
 * Description: Example program to demonstrate the exchange of serial messages between two arduino boards through the RX and TX pins.
                Arduino 01 have an ultrasonic sensor to instantiate a BasketSensor object and send a message when a ball is detected.
                Arduino 02 lights up its LED_BUITIN when it receives these messages.
 * Author: José Paulo Seibt Neto
 * Created: Apr - 2025
 * Last Modified: Apr - 2025
*/

#include <NBAPark.h>

// Shared message
#define MESSAGE "ball detected"

BasketSensor hoop(4, 5); // Set trig on pin 4 and echo on pin 5

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    if (hoop.ball_detected())
    {
        Serial.println(MESSAGE);
        delay(500);
    }
}