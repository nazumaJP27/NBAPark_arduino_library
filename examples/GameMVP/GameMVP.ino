/*
 * NBA Park Arduino Library
 * Description: Example program that reads custom OSC messages send by Resolume Arena to correctly read values from three BasketSensor objs
                working in conjunction with a MVPHoopsLayout instance to count basketballs that go through the rim at specific times based
                on the clip projected in the Resolume composition, displaying the ball count in real time.
 * Author: José Paulo Seibt Neto
 * Created: Mar - 2025
 * Last Modified: Apr - 2025
*/

#include <NBAPark.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <string.h>     // strcmp

Timer timer;
int now;

const uint8_t trig_pins[] = {2, 4, 6};
const uint8_t echo_pins[] = {3, 5, 7};

const Layout test_layouts[] = {
    Layout(5, Layout::LAYOUT_2),  // 0
    Layout(12, Layout::LAYOUT_6), // 1
    Layout(14, Layout::LAYOUT_4), // 2
    Layout(21, Layout::LAYOUT_7), // 3
    Layout(23, Layout::LAYOUT_3), // 4
    Layout(27, Layout::LAYOUT_5), // 5
    Layout(29, Layout::LAYOUT_2), // 6
    Layout(33, Layout::LAYOUT_5), // 7
    Layout(35, Layout::LAYOUT_3), // 8
    Layout(41, Layout::LAYOUT_7), // 9
    Layout(43, Layout::LAYOUT_4), // 10
    Layout(45, Layout::LAYOUT_7), // 11
    Layout(47, Layout::LAYOUT_3), // 12
    Layout(50, Layout::LAYOUT_5), // 13
    Layout(52, Layout::LAYOUT_2), // 14
    Layout(54, Layout::LAYOUT_6), // 15
    Layout(56, Layout::LAYOUT_4), // 16
    Layout(59, Layout::LAYOUT_7), // 17
    Layout(61, Layout::LAYOUT_3), // 18
    Layout(64, Layout::LAYOUT_7), // 19
    Layout(66, Layout::LAYOUT_4), // 20
    Layout(68, Layout::LAYOUT_7), // 21
    Layout(60, Layout::LAYOUT_3), // 22
    Layout(72, Layout::LAYOUT_7), // 23
    Layout(74, Layout::LAYOUT_4), // 24
    Layout(77, Layout::LAYOUT_6), // 25
    Layout(79, Layout::LAYOUT_2), // 26
    Layout(80, Layout::LAYOUT_6), // 27
    Layout(82, Layout::LAYOUT_1), // 28
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

uint8_t osc_message_buffer[255];

EthernetUDP udp;

const uint8_t board_mac[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x02};
const IPAddress board_ip(172, 30, 4, 199);
const IPAddress pc_ip(172, 30, 4, 102);
const int resolume_in_port = 7000;
const int resolume_out_port = 7001;

// Prototypes
void send_osc_to_resolume(uint8_t in_ball_count, bool zero_prefix=false);
void reset_game_state();

void setup()
{
    Serial.begin(115200);

    layouts_mvp.init(test_layouts, sizeof(test_layouts) / sizeof(test_layouts[0]));
    current_mvp_layout = layouts_mvp.get_curr_layout();
    mvp_state = MVPHoopsLayouts::MVPState::MVP_GAME_OVER;

    Ethernet.begin(board_mac, board_ip);
    udp.begin(resolume_out_port);

    now = timer.reset_timer();
    ball_count = 0;
}

void loop()
{
    start:

    while (mvp_state == MVPHoopsLayouts::MVPState::MVP_GAME_OVER)
    {
        if (udp.parsePacket())
        {
            udp.read(osc_message_buffer, sizeof(osc_message_buffer));
            OSCPark msg(osc_message_buffer);
            if (strncmp(msg.get_addr_cmp(), "/game", msg.get_addr_len()) == 0)
            {
                reset_game_state();
                send_osc_to_resolume(0);
            }
        }
        Serial.println("WAITING /game");
    }

    // Check if the Resolume clip is the WAIT
    if (udp.parsePacket())
    {
        udp.read(osc_message_buffer, sizeof(osc_message_buffer));
        OSCPark msg(osc_message_buffer);
        if (strncmp(msg.get_addr_cmp(), "/wait", msg.get_addr_len()) == 0)
        {
            Serial.print("GOT /wait\n");
            mvp_state = MVPHoopsLayouts::MVPState::MVP_GAME_OVER;
            goto start;
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
    }
    else
    { // Check the sensors
        for (int i = 0; i < NUM_MVP_HOOPS; ++i)
        {
            if (current_mvp_layout[i] && mvp_baskets[i].ball_detected())
            {
                Serial.print("BALL DETECTED! ball count: ");
                Serial.println(++ball_count);
                send_osc_to_resolume(ball_count);
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

void send_osc_to_resolume(uint8_t in_ball_count, bool zero_prefix=false)
{
    // Construct the OSC address using snprintf to write into the global buffer        /composition/layers/2/clips/2/video/effects/textblock/effect/text/params/lines
    memset(osc_message_buffer, 0, sizeof(osc_message_buffer));
    snprintf(reinterpret_cast<char*>(osc_message_buffer), sizeof(osc_message_buffer), "/composition/layers/2/clips/1/video/effects/textblock/effect/text/params/lines");

    OSCPark msg(reinterpret_cast<char*>(osc_message_buffer));

    // Set the value for the string
    char bc_buffer[4];
    itoa(in_ball_count, bc_buffer, 10);

    if (zero_prefix && in_ball_count < 10)
    {   // With zero prefix for numbers 0-9, e.g. "07"
        char tmp = bc_buffer[0];
        bc_buffer[0] = '0';
        bc_buffer[1] = tmp;
        bc_buffer[2] = '\0';
    }
    else if (in_ball_count < 100)
    {   // Numbers 10-99, e.g. "27"
        bc_buffer[2] = '\0';
    }
    else
    {   // Three digit numbers (max), e.g. "127"
        bc_buffer[3] = '\0';
    }

    msg.set_string(bc_buffer);

    // Send message through EthernetUDP global instance
    udp.beginPacket(pc_ip, resolume_in_port);
    msg.send(udp);
    udp.endPacket();
}

// Reset global instances used in the game logic, like layouts, sensors, and counters
void reset_game_state()
{
    layouts_mvp.reset();
    now = timer.reset_timer();
    mvp_state = layouts_mvp.update(now);
    ball_count = 0;
}