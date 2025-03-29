/*
 * NBA Park Arduino Library
 * Description: Example program to test functionality of the OSCPark::send method that uses Arduino Print class to send an OSC message to Resolume Arena through UDP packets
 * Author: José Paulo Seibt Neto
 * Created: Mar - 2025
 * Last Modified: Mar - 2025
*/

#include <NBAPark.h>
#include <Ethernet.h>
#include <EthernetUDP.h>

uint8_t test_msg[] = {0x2f, 0x67, 0x61, 0x6d, 0x65, 0x00, 0x00, 0x00, 0x2c, 0x66, 0x00, 0x00, 0x0000003f};
OSCPark msg;

uint8_t osc_message_buffer[255];
EthernetUDP udp;

const uint8_t board_mac[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x04};
const IPAddress board_ip(172, 30, 6, 200);
const IPAddress pc_ip(172, 30, 6, 58);
const int resolume_in_port = 7000;
const int resolume_out_port = 7001;

Timer timer;
int now;

void setup()
{
    Serial.begin(115200);
    now = timer.reset_timer();

    Ethernet.begin(board_mac, board_ip);
    udp.begin(resolume_out_port);
}

void loop()
{
    now = timer.get_elapsed_time();

    if (now < 15)
    {
        // Send test message
        msg.init("/composition/layers/1/clips/2/connect");
        Serial.print("Sending OSC message: ");
        msg.print();
        msg.info();
        udp.beginPacket(pc_ip, resolume_in_port);
        msg.send(udp);
        udp.endPacket();
    }
    else
    {
        Serial.println("Waiting");
        while (now < 30)
        {
            now = timer.get_elapsed_time();
        }
        now = timer.reset_timer();
    }
}