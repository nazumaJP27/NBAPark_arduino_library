/*
 * NBA Park Arduino Library
 * Description: Example program that reads custom OSC messages send by Resolume Arena to correctly read values from three BasketSensor objs
                working in conjunction with a MVPHoopsLayout instance to count basketballs that go through the rim at specific times based
                on the clip projected in the Resolume composition, displaying the score count of the game in real time.
 * Author: José Paulo Seibt Neto
 * Created: Mar - 2025
 * Last Modified: Apr - 2025
*/

#include <NBAPark.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <string.h>       // strncmp

#define RSTPIN 12 // Pin number used to trigger the board RESET pin

// Time
Timer high_score_timer;  // Instance used to reset the high score of the game or reset the Arduino (sending LOW to RESET pin) based on elapsed time
Timer game_timer;        // Instance used to setup game logic such as the valid sensors to check at any given time
uint32_t now; 

// MVP hoops and sensors
MVPHoopsLayouts layouts_mvp;
const bool* current_mvp_layout; // Pointer to the valid layout at any given time, as the MVPHoopsLayouts instance updates
MVPHoopsLayouts::MVPState mvp_state;

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

// Stat tracking
bool new_high_score;
uint16_t high_score_count;
uint16_t score_count;

// OSC messages and network configuration
uint8_t osc_message_buffer[255];
EthernetUDP udp;
const uint8_t board_mac[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x05};
const IPAddress board_ip(172, 30, 6, 199);
const IPAddress pc_ip(172, 30, 6, 58);
const int resolume_in_port = 7000;
const int resolume_out_port = 7001;

enum ScoreType : uint8_t
{
    SCORE,
    HIGH_SCORE
};

// Prototypes
bool send_score_to_resolume(ScoreType in_type, uint16_t in_score);
void reset_game_state();
void game_update();

void setup()
{
    digitalWrite(RSTPIN, HIGH); // Keep a weakly HIGH state on RSTPIN as the board RESET pin only triggers when it is pulled LOW

    Serial.begin(115200);
    debugSkt("[GameMVP.ino] setup\n");

    layouts_mvp.init(test_layouts, sizeof(test_layouts) / sizeof(test_layouts[0]));
    current_mvp_layout = layouts_mvp.get_curr_layout();
    mvp_state = MVPHoopsLayouts::MVPState::MVP_GAME_OVER;

    Ethernet.begin(board_mac, board_ip);
    udp.begin(resolume_out_port);

    high_score_timer.reset();
    now = game_timer.reset();

    new_high_score = false;
    high_score_count = DEFAULT_HIGH_SCORE;
    score_count = 0;
}

void loop()
{
    // Check for new messages
    if (udp.parsePacket())
    {
        udp.read(osc_message_buffer, sizeof(osc_message_buffer));
        OSCPark msg(osc_message_buffer);

        if (strncmp(msg.get_addr_cmp(), RESOLUME_MVPGAME_ADDRESS, RESOLUME_MAX_ADDRESS_LEN) == 0)
        {
            if (mvp_state == MVPHoopsLayouts::MVPState::MVP_GAME_OVER)
            {   
                reset_game_state();
                send_score_to_resolume(SCORE, 0);
                send_score_to_resolume(HIGH_SCORE, high_score_count);
            }
        }
        else if (strncmp(msg.get_addr_cmp(), RESOLUME_MVPWAIT_ADDRESS, RESOLUME_MAX_ADDRESS_LEN) == 0)
        {
            debugSkt("GOT RESOLUME_MVPWAIT_ADDRESS\n");
            mvp_state = MVPHoopsLayouts::MVPState::MVP_GAME_OVER;
        }
    }

    if (mvp_state == MVPHoopsLayouts::MVPState::MVP_GAME_OVER)
    {
        // Check if it is time to reset the current highest score
        if (high_score_timer.get_elapsed_time() > HIGH_SCORE_RESET_TIME || high_score_count > 100)
        {
            high_score_count = DEFAULT_HIGH_SCORE;
            high_score_timer.reset();
            debugSkt("HIGH SCORE RESET...\n");
        }
        debugSkt("WAITING RESOLUME_MVPGAME_ADDRESS\n");
    }
    else
    {   // GAME
        now = game_timer.get_elapsed_time();
        mvp_state = layouts_mvp.update(now);

        if (mvp_state == MVPHoopsLayouts::MVPState::MVP_HOLD)
        {
            debugSkt("mvp_state: MVP_HOLD\n");
        }
        else if (mvp_state == MVPHoopsLayouts::MVPState::MVP_GAME_OVER)
        {
            debugSkt("mvp_state: MVP_GAME_OVER\n");
            mvp_state = MVPHoopsLayouts::MVPState::MVP_GAME_OVER;
        }
        else
        {
            game_update();
        }
    }
}

// Check sensors and update stat variables
void game_update()
{
    debugSkt("[game_update] ");
    for (int i = 0; i < NUM_MVP_HOOPS; ++i)
    {
        if (current_mvp_layout[i] && mvp_baskets[i].ball_detected())
        {
            score_count += 2;
            send_score_to_resolume(SCORE, score_count);
            debugSkt("BALL DETECTED! ball count: ");
            debugSkt(score_count);

            // Check for new high score
            if (score_count > high_score_count)
            {
                high_score_count = score_count;
                debugSkt(" | NEW HIGH SCORE!\n");
                send_score_to_resolume(HIGH_SCORE, high_score_count);

                // Check if the new high score clip was already triggered
                if (!new_high_score)
                {   // Activate the new high score pop-up clip on Resolume Arena
                    snprintf(reinterpret_cast<char*>(osc_message_buffer), sizeof(osc_message_buffer), RESOLUME_NEW_HIGH_SCORE_ADDRESS);
                    OSCPark msg(reinterpret_cast<char*>(osc_message_buffer));
                    udp.beginPacket(pc_ip, resolume_in_port);
                    msg.send(udp);
                    udp.endPacket();
                    new_high_score = true;
                }
            }
        }
    }
    debugSkt("Current Layout: ");
    debugSkt(current_mvp_layout[0]);
    debugSkt(", ");
    debugSkt(current_mvp_layout[1]);
    debugSkt(", ");
    debugSkt(current_mvp_layout[2]); debugSktln();
}

// Send OSC message to change the value in the High Score or Score text block in Resolume Arena
bool send_score_to_resolume(ScoreType in_type, uint16_t in_score)
{
    debugSkt("[send_score_to_resolume] ");
    memset(osc_message_buffer, 0, sizeof(osc_message_buffer));

    // Set string value
    // Construct the OSC address using snprintf to write into the global buffer (based in the ScoreType argument)
    switch (in_type)
    {
        case SCORE:
            debugSkt("Sending score...\n");
            snprintf(reinterpret_cast<char*>(osc_message_buffer), sizeof(osc_message_buffer), RESOLUME_SCORE_ADDRESS);
            break;
        case HIGH_SCORE:
            debugSkt("Sending high score...\n");
            snprintf(reinterpret_cast<char*>(osc_message_buffer), sizeof(osc_message_buffer), RESOLUME_HIGH_SCORE_ADDRESS);
            break;
        default:
            debugSkt("Invalid ScoreType argument...\n");
            return false;
            break;
    }

    OSCPark msg(reinterpret_cast<char*>(osc_message_buffer));

    char score_buffer[4]; // Store chars for the numbers of the high score or score
    itoa(in_score, score_buffer, 10);

    if (in_score < 10)
    {   // With zero prefix for numbers 0-9, e.g. "07"
        char tmp = score_buffer[0];
        score_buffer[0] = '0';
        score_buffer[1] = tmp;
        score_buffer[2] = '\0';
    }
    else if (in_score < 100)
    {   // Numbers 10-99, e.g. "27"
        score_buffer[2] = '\0';
    }
    else
    {   // Three digit numbers (max), e.g. "127"
        score_buffer[3] = '\0';
    }

    // Set score buffer as the value for the OSCPark instance
    msg.set_string(score_buffer);

    // Send message through EthernetUDP global instance
    udp.beginPacket(pc_ip, resolume_in_port);
    msg.send(udp);
    udp.endPacket();

    return true;
}

// Reset global instances used in the game logic, like layouts, sensors, and some counters
void reset_game_state()
{
    layouts_mvp.reset();
    now = game_timer.reset();
    mvp_state = layouts_mvp.update(now);
    new_high_score = false;
    score_count = 0;
}