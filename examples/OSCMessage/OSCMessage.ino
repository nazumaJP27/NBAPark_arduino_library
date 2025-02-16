#include <NBAPark.h>
#include <OSCMessage.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <stdio.h>

BasketSensor basket(2, 3);
uint8_t ball_count;

uint8_t board_mac[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x04};
IPAddress board_ip(172, 30, 6, 200);
IPAddress pc_ip(172, 30, 6, 58);
int resolume_port = 7000;

EthernetUDP udp;
//IPAddress mvp4_pc_ip(172, 30, 4, 104) // NPG-MVP004 >NBAPARKDINAMICO/>172.30.4.104

char osc_message_buffer[40];

// Prototypes
void send_osc_resolume(uint8_t in_ball_count);


void setup()
{
    Ethernet.begin(board_mac, board_ip);
    udp.begin(resolume_port);

    Serial.begin(115200);

    ball_count = 0;
    send_osc_resolume(ball_count);
}

void loop()
{
    Serial.print("0\n");
    if (basket.ball_detected())
    {
        Serial.print("Ball detected! count: ");
        Serial.println(++ball_count);
        send_osc_resolume(ball_count);
        delay(20);
    }

    delay(100);
}

void send_osc_resolume(uint8_t in_ball_count)
{
    // Construct the OSC address using snprintf to write into a buffer and inserting the ball count variable
    snprintf(osc_message_buffer, sizeof(osc_message_buffer), "/composition/layers/2/clips/%d/connect", in_ball_count + 2);

    OSCMessage msg(osc_message_buffer);

    udp.beginPacket(pc_ip, resolume_port);
    msg.send(udp);
    udp.endPacket();
}