/*
 * NBA Park Arduino Library
 * Description: Example program that reads custom OSC messages send by Resolume Arena to correctly read values from three BasketSensor objs
                working in conjunction with a MVPHoopsLayout instance to count basketballs that go through the rim at specific times based
                on the clip projected in the Resolume composition, displaying the ball count in real time.
 * Author: José Paulo Seibt Neto
 * Created: Mar - 2025
 * Last Modified: Mar - 2025
*/

#include <NBAPark.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <OSCMessage.h>
#include <string.h>     // strcmp

const uint8_t trig_pins[] = {2, 4, 6};
const uint8_t echo_pins[] = {3, 5, 7};

const Layout test_layouts[] = {
    Layout(5, Layout::LAYOUT_2),
    Layout(12, Layout::LAYOUT_6),
    Layout(13, Layout::LAYOUT_4),
    Layout(21, Layout::LAYOUT_7),
    Layout(22, Layout::LAYOUT_3),
    Layout(27, Layout::LAYOUT_5),
    Layout(28, Layout::LAYOUT_2),
    Layout(33, Layout::LAYOUT_5),
    Layout(34, Layout::LAYOUT_3),
    Layout(41, Layout::LAYOUT_7),
    Layout(42, Layout::LAYOUT_4),
    Layout(45, Layout::LAYOUT_7),
    Layout(46, Layout::LAYOUT_3),
    Layout(50, Layout::LAYOUT_5),
    Layout(51, Layout::LAYOUT_2),
    Layout(54, Layout::LAYOUT_6),
    Layout(55, Layout::LAYOUT_4),
    Layout(59, Layout::LAYOUT_7),
    Layout(60, Layout::LAYOUT_3),
    Layout(64, Layout::LAYOUT_7),
    Layout(65, Layout::LAYOUT_4),
    Layout(68, Layout::LAYOUT_7),
    Layout(69, Layout::LAYOUT_3),
    Layout(72, Layout::LAYOUT_7),
    Layout(73, Layout::LAYOUT_4),
    Layout(77, Layout::LAYOUT_6),
    Layout(78, Layout::LAYOUT_2),
    Layout(80, Layout::LAYOUT_6),
    Layout(81, Layout::LAYOUT_1),
    Layout(93, Layout::LAYOUT_STOP),
};

BasketSensor mvp_baskets[] = {
    BasketSensor(trig_pins[0], echo_pins[0]),
    BasketSensor(trig_pins[1], echo_pins[1]),
    BasketSensor(trig_pins[2], echo_pins[2])
};

uint8_t ball_count;

MVPHoopsLayouts layouts_mvp;
const bool* current_mvp_layout;
MVPHoopsLayouts::MVPState mvp_state;

Timer timer;
int now;

uint8_t osc_message_buffer[255];
OSCPark msg;

EthernetUDP udp;

const uint8_t board_mac[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x04};
const IPAddress board_ip(172, 30, 6, 200);
const IPAddress pc_ip(172, 30, 6, 58);
const int resolume_in_port = 7000;
const int resolume_out_port = 7001;

// Prototypes
void send_osc_resolume(uint8_t in_ball_count);

void setup()
{
    Serial.begin(115200);

    ball_count = 0;

    layouts_mvp.init(test_layouts, sizeof(test_layouts) / sizeof(test_layouts[0]));
    current_mvp_layout = layouts_mvp.get_curr_layout();
    mvp_state = MVPHoopsLayouts::MVPState::MVP_GAME_OVER;

    Ethernet.begin(board_mac, board_ip);
    udp.begin(resolume_out_port);

    msg.clear();
    now = timer.reset_timer();
}

void loop()
{
    while (mvp_state == MVPHoopsLayouts::MVPState::MVP_GAME_OVER)
    {
        now = timer.reset_timer();
        if (udp.parsePacket())
        {
            udp.read(osc_message_buffer, sizeof(osc_message_buffer));
            msg.init(osc_message_buffer);
            if (strncmp(msg.get_addr_cmp(), "/game", msg.get_addr_len()) == 0)
            {
                mvp_state = layouts_mvp.update(now);
                ball_count = 0;
                send_osc_resolume(ball_count);
            }
        }
        Serial.println("WAITING OSC");
    }

    // Check if the Resolume clip is the WAIT
    if (udp.parsePacket())
    {
        udp.read(osc_message_buffer, sizeof(osc_message_buffer));
        msg.init(osc_message_buffer);
        if (strncmp(msg.get_addr_cmp(), "/wait", msg.get_addr_len()) == 0)
        {
            Serial.print("GAME OVER\n");
            mvp_state = MVPHoopsLayouts::MVPState::MVP_GAME_OVER;
            layouts_mvp.reset();
            loop();
        }
    }

    now = timer.get_elapsed_time();

    mvp_state = layouts_mvp.update(now);

    if (mvp_state == MVPHoopsLayouts::MVPState::MVP_HOLD)
    {
        Serial.println("MVP_HOLD");
    }
    else if (mvp_state == MVPHoopsLayouts::MVPState::MVP_GAME_OVER)
    {
        Serial.print("GAME OVER\n");
        mvp_state = MVPHoopsLayouts::MVPState::MVP_GAME_OVER;
        delay(1500);
    }
    else
    { // Check the sensors
        for (int i = 0; i < NUM_MVP_HOOPS; ++i)
        {
            if (current_mvp_layout[i] && mvp_baskets[i].ball_detected())
            {
                Serial.print("BALL DETECTED! ball count: ");
                Serial.println(++ball_count);
                send_osc_resolume(ball_count);
                delay(1000);
            }
        }
        //Serial.print("MVP State: ");
        //Serial.println(mvp_state);
        Serial.print(current_mvp_layout[0]);
        Serial.print(", ");
        Serial.print(current_mvp_layout[1]);
        Serial.print(", ");
        Serial.println(current_mvp_layout[2]);
    }
}

void send_osc_resolume(uint8_t in_ball_count)
{
    // Construct the OSC address using snprintf to write into a buffer and inserting the ball count variable
    snprintf((char*)osc_message_buffer, sizeof(osc_message_buffer), "/composition/layers/2/clips/%d/connect", in_ball_count + 2);

    OSCMessage msg((char*)osc_message_buffer);

    udp.beginPacket(pc_ip, resolume_in_port);
    msg.send(udp);
    udp.endPacket();
}