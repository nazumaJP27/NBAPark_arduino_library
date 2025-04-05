/*
 * NBA Park Arduino Library
 * Description: Example program to demonstrate the exchange of serial messages between two arduino boards through the RX and TX pins.
                Arduino 01 have an ultrasonic sensor to instantiate a BasketSensor object and send a message when a ball is detected.
                Arduino 02 lights up its LED_BUITIN when it receives these messages.
 * Author: José Paulo Seibt Neto
 * Created: Apr - 2025
 * Last Modified: Apr - 2025
*/

#include <string.h>  // strncmp, strnlen

// Shared message
#define MESSAGE "ball detected"

const int BUFFER_SIZE = 64;
char buffer[BUFFER_SIZE];
uint8_t buffer_pos;

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    buffer[0] = '\0';
    buffer_pos = 0;
}

void loop()
{
    while (Serial.available() > 0)
    {
        char msg_byte = Serial.read();

        if (msg_byte != '\0' && msg_byte != '\n' && msg_byte != '\r' && buffer_pos < BUFFER_SIZE - 1)
        {
            // Append msg_byte to the buffer and increment buffer_pos
            buffer[buffer_pos++] = msg_byte;
        }
        else // End of the message or max length reached
        {
            buffer[buffer_pos] = '\0';

            // Check message
            if (strncmp(buffer, "ball detected", strnlen(buffer, BUFFER_SIZE)) == 0)
            {
                // Light up the board led for one second
                digitalWrite(LED_BUILTIN, HIGH);
                delay(1000);
            }

            // Reset the buffer to parse a new message
            buffer[0] = '\0';
            buffer_pos = 0;
        }
    }

    digitalWrite(LED_BUILTIN, LOW);
}