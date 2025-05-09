/*
 * NBA Park Arduino Library
 * Description: Example program that reads custom OSC messages send by Resolume Arena to correctly read values from three BasketSensor objs
                working in conjunction with a MVPHoops instance to count basketballs that go through the rim at specific times based
                on the clip projected in the Resolume composition, displaying the score count of the game in real time.
 * Author: José Paulo Seibt Neto
 * Created: Apr - 2025
 * Last Modified: May - 2025
*/

#include <NBAPark.h>
#include <string.h>       // strcmp

#define RSTPIN 12 // Pin number used to trigger the board RESET pin

// Time
Timer high_score_timer; // Instance used to reset the high score of the game or reset the Arduino (sending LOW to RESET pin) based on elapsed time
Timer game_timer;  // Instance used to setup game logic such as the valid sensors to check at any given time
int now;

// MVP hoops and sensors
MVPHoops mvp_hoops;
BitmapPattern curr_mvp_pattern;
MVPHoops::MVPState mvp_state;

const uint8_t trig_pins[] = {2, 4, 6};
const uint8_t echo_pins[] = {3, 5, 7};

const MVPHoops::MVPHoops::Layout test_layouts[] = {
    MVPHoops::Layout(5, BitmapPattern::LAYOUT_4),  // 0
    MVPHoops::Layout(12, BitmapPattern::LAYOUT_5), // 1
    MVPHoops::Layout(14, BitmapPattern::LAYOUT_1), // 2
    MVPHoops::Layout(21, BitmapPattern::LAYOUT_3), // 3
    MVPHoops::Layout(23, BitmapPattern::LAYOUT_2), // 4
    MVPHoops::Layout(27, BitmapPattern::LAYOUT_6), // 5
    MVPHoops::Layout(29, BitmapPattern::LAYOUT_4), // 6
    MVPHoops::Layout(33, BitmapPattern::LAYOUT_6), // 7
    MVPHoops::Layout(35, BitmapPattern::LAYOUT_2), // 8
    MVPHoops::Layout(41, BitmapPattern::LAYOUT_3), // 9
    MVPHoops::Layout(43, BitmapPattern::LAYOUT_1), // 10
    MVPHoops::Layout(45, BitmapPattern::LAYOUT_3), // 11
    MVPHoops::Layout(47, BitmapPattern::LAYOUT_2), // 12
    MVPHoops::Layout(50, BitmapPattern::LAYOUT_6), // 13
    MVPHoops::Layout(52, BitmapPattern::LAYOUT_4), // 14
    MVPHoops::Layout(54, BitmapPattern::LAYOUT_5), // 15
    MVPHoops::Layout(56, BitmapPattern::LAYOUT_1), // 16
    MVPHoops::Layout(59, BitmapPattern::LAYOUT_3), // 17
    MVPHoops::Layout(61, BitmapPattern::LAYOUT_2), // 18
    MVPHoops::Layout(64, BitmapPattern::LAYOUT_3), // 19
    MVPHoops::Layout(66, BitmapPattern::LAYOUT_1), // 20
    MVPHoops::Layout(68, BitmapPattern::LAYOUT_3), // 21
    MVPHoops::Layout(60, BitmapPattern::LAYOUT_2), // 22
    MVPHoops::Layout(72, BitmapPattern::LAYOUT_3), // 23
    MVPHoops::Layout(74, BitmapPattern::LAYOUT_1), // 24
    MVPHoops::Layout(77, BitmapPattern::LAYOUT_5), // 25
    MVPHoops::Layout(79, BitmapPattern::LAYOUT_4), // 26
    MVPHoops::Layout(80, BitmapPattern::LAYOUT_5), // 27
    MVPHoops::Layout(82, BitmapPattern::LAYOUT_7), // 28
    MVPHoops::Layout(93, BitmapPattern::LAYOUT_STOP),
};

BasketSensor mvp_baskets[] = {
    BasketSensor(trig_pins[0], echo_pins[0]),
    BasketSensor(trig_pins[1], echo_pins[1]),
    BasketSensor(trig_pins[2], echo_pins[2])
};

ThreeBasketSensors tbs(trig_pins, echo_pins);

// Stat tracking
uint16_t high_score_count;
uint16_t score_count;

const int BUFFER_SIZE = 64;
char buffer[BUFFER_SIZE];
uint8_t buffer_pos;
bool new_message;

// Prototypes
void read_to_buffer();
void reset_game_state();
void game_update();

void setup()
{
    digitalWrite(RSTPIN, HIGH); // Keep a weakly HIGH state on RSTPIN as the board RESET pin only triggers when it is pulled LOW

    Serial.begin(115200);
    Serial.println("Program started");

    mvp_hoops.init(test_layouts, sizeof(test_layouts) / sizeof(test_layouts[0]));
    curr_mvp_pattern = mvp_hoops.get_curr_pattern();
    mvp_state = MVPHoops::MVPState::MVP_GAME_OVER;

    high_score_timer.reset();
    now = game_timer.reset();

    high_score_count = DEFAULT_HIGH_SCORE;
    score_count = 0;

    buffer[0] = '\0';
    buffer_pos = 0;
    new_message = false;
}

void loop()
{
    // Check for new messages
    if (Serial.available() > 1)
    {
        while (!new_message) read_to_buffer();

        if (strncmp(buffer, RESOLUME_MVPGAME_ADDRESS, BUFFER_SIZE) == 0)
        {
            debugSkt("GOT RESOLUME_MVPGAME_ADDRESS\n");
            reset_game_state();
        }
        else if (strncmp(buffer, RESOLUME_MVPWAIT_ADDRESS, BUFFER_SIZE) == 0)
        {
            debugSkt("GOT RESOLUME_MVPWAIT_ADDRESS\n");
            mvp_state = MVPHoops::MVPState::MVP_GAME_OVER;
        }
        new_message = false;
    }

    if (mvp_state == MVPHoops::MVPState::MVP_GAME_OVER)
    {
        // Check if it is time to reset the current highest score
        if (high_score_timer.get_elapsed_time() > HIGH_SCORE_RESET_TIME || high_score_count > 100)
        {
            high_score_count = DEFAULT_HIGH_SCORE;
            high_score_timer.reset();
            debugSkt("HIGH SCORE RESET...\n");
            pinMode(RSTPIN, OUTPUT);
            digitalWrite(RSTPIN, LOW);
        }
        debugSkt("WAITING /game\n");
    }
    else
    {   // GAME
        now = game_timer.get_elapsed_time();
        mvp_state = mvp_hoops.update(now);
        curr_mvp_pattern = mvp_hoops.get_curr_pattern();
        
        if (mvp_state == MVPHoops::MVPState::MVP_HOLD)
        {
            debugSkt("mvp_state: MVP_HOLD\n");
        }
        else if (mvp_state == MVPHoops::MVPState::MVP_GAME_OVER)
        {
            debugSkt("mvp_state: MVP_GAME_OVER\n");
            mvp_state = MVPHoops::MVPState::MVP_GAME_OVER;
        }
        else
        {
            //debugSkt("Call to game_update()\n");
            game_update();
        }
    }
}

// Check sensors and update stat variables
void game_update()
{
    debugSkt("Current Layout: ");
    debugSktVal(curr_mvp_pattern, BIN); debugSktln();

    // Add a delay before checking the sensors to prevent interferences from previous readings (ultrasonic sensors are finicky)
    delayMicroseconds(BALL_DETECTION_READ_DELAY);
    BitmapPattern checks = tbs.check_sensors();
    uint8_t shots_converted = tbs.filter_sensor_readings(curr_mvp_pattern, checks);
    
    if (shots_converted > 0)
    {
        score_count += shots_converted * 2; // Each shot converted grants 2 points
        debugSkt("BALL DETECTED! ball count: ");
        debugSkt(score_count);

        // Check for new high score
        if (score_count > high_score_count)
        {
            high_score_count = score_count;
            debugSkt(" | NEW HIGH SCORE!");
        }

        debugSkt(" | high score: ");
        debugSkt(high_score_count); debugSktln();
        delay(1000);
    }
}

void read_to_buffer()
{
    debugSkt("READING MESSAGE...\n");
    delay(1000);
    while (Serial.available() > 0)
    {
        char msg_byte = Serial.read();

        if (msg_byte != '\0' && msg_byte != '\n' && msg_byte != '\r' && buffer_pos < BUFFER_SIZE - 1)
        {   // Append msg_byte to the buffer and increment buffer_pos
            buffer[buffer_pos++] = msg_byte;
        }
        else
        {   // End of the message or max length reached
            buffer[buffer_pos] = '\0';
            buffer_pos = 0;
            debugSkt("New message buffered... | buffer: ");
            debugSkt(buffer); debugSktln();
            delay(2000);
            new_message = true;
        }
    }
}

// Reset global instances used in the game logic, like layouts, sensors, and some counters
void reset_game_state()
{
    mvp_hoops.reset();
    now = game_timer.reset();
    mvp_state = mvp_hoops.update(now);
    curr_mvp_pattern = mvp_hoops.get_curr_pattern();
    score_count = 0;
}