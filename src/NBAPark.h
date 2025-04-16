/*
 * NBA Park Arduino Library
 * Description: Declarations of classes and structs
 * Author: José Paulo Seibt Neto
 * Created: Fev - 2025
 * Last Modified: Apr - 2025
*/

#ifndef NBAPARK_H
#define NBAPARK_H

#include <Arduino.h>

// Constants
#define ULONG_MAX 4294967295UL      // Max value for an usigned long type (used to calculate the offset in Timer)
#define SOUND_SPEED 0.0343f         // Speed of sound in centimeters per microsecond
#define BALL_DETECTION_THRESHOLD 30 // Value in centimeters
#define BALL_DETECTION_COOLDOWN 500 // Value in milliseconds
#define BALL_DETECTION_TIMEOUT 5000 // Value in microseconds (3-5ms timeout should be enough for reads up to ~50cm)
#define NUM_MVP_HOOPS 3


class Timer
{
    unsigned long m_start_time;
    unsigned long m_offset_time;

public:
    // Constructors
    Timer();

    // Accessors
    const unsigned long& get_start_time() const { return m_start_time; }
    const unsigned long& get_offset_time() const { return m_offset_time; }

    // Methods
    int reset_timer();
    unsigned long get_elapsed_time(bool seconds=true) const;
};


// Need a HC-SR04 sensor
class BasketSensor
{
    // Pins used by the ultrasonic sensor
    uint8_t m_trig_pin;
    uint8_t m_echo_pin;

    // Separate hoop state that handle the cooldown for checking sensor after a ball is detected (all in-lined for simplicity)
    struct HoopCooldown
    {
        Timer mil_timer;
        bool on_cooldown;
        uint16_t cooldown_time;

        // Constructor
        HoopCooldown() : on_cooldown(false), cooldown_time(0) { mil_timer.reset_timer(); }

        // Methods
        void set_cooldown(int in_cooldown_amount)
        {
            mil_timer.reset_timer();
            cooldown_time = in_cooldown_amount;
            on_cooldown = true;
        }

        void update()
        {
            if (on_cooldown && mil_timer.get_elapsed_time(false) > cooldown_time)
            {
                on_cooldown = false;
                cooldown_time = 0;
            }
        }

        void reset()
        {
            mil_timer.reset_timer();
            on_cooldown = false;
            cooldown_time = 0;
        }
    } m_hoop_cooldown;

public:
    // Constructors
    BasketSensor(uint8_t in_trig_pin, uint8_t in_echo_pin);

    // Acessors
    const uint8_t& get_trig_pin() const { return m_trig_pin; }
    const uint8_t& get_echo_pin() const { return m_echo_pin; }

    // Methods
    float get_ultrasonic_distance();
    bool ball_detected();
};


struct Layout
{
    enum LayoutId 
    {
        LAYOUT_0,
        LAYOUT_1,
        LAYOUT_2,
        LAYOUT_3,
        LAYOUT_4,
        LAYOUT_5,
        LAYOUT_6,
        LAYOUT_7,
        LAYOUT_STOP, // sentinel value
        NUM_LAYOUTS
    };

    // All the possible layouts for three hoops attached to the wall
    static const bool POSSIBLE_LAYOUTS[NUM_LAYOUTS][NUM_MVP_HOOPS];

    // Member variables
    int time;
    LayoutId id;

    // Constructors
    Layout();
    Layout(int in_time, LayoutId in_layout_id);

    // Accessor
    const bool* get_bool_layout() const { return POSSIBLE_LAYOUTS[id]; }
};

// Class needs to point to an array of Layout objects with the last layout being a LAYOUT_STOP
class MVPHoopsLayouts
{
    // Member variables
    const Layout* m_layouts_arr;
    uint8_t m_curr;                    // Index for the current Layout obj  
    uint8_t m_next;                    // Index for the next Layout obj
    bool m_curr_layout[NUM_MVP_HOOPS]; // Copy of the current layout boolean array (Layout::POSSIBLE_LAYOUTS)

public:
    enum MVPState
    {
        MVP_GAME_OVER,
        MVP_RUNNING,
        MVP_HOLD
    };

    // Constructors
    MVPHoopsLayouts();
    MVPHoopsLayouts(const Layout* in_layouts_arr, const uint8_t in_size);

    // Methods
    bool init(const Layout* in_layouts_arr, const uint8_t in_size);
    MVPState update(int in_time);
    MVPState reset(); // Always return MVP_GAME_OVER

    // Accessors
    const bool* get_curr_layout() const { return m_curr_layout; }

private:
    // Method to iterate over layouts and ensure correct boundary checking
    bool validate_layouts_arr(const Layout* in_layouts_arr, const uint8_t in_size);
    void copy_layout(const Layout* in_layout);
};


class OSCPark
{
    // Struct used to store the value data from OSCMessages
    struct Value
    {
        char type_tag;
        union Data
        {
            int32_t i_value;
            float f_value;
            char* s_value;
        } data;

        void setup(const char in_type_tag, const uint8_t* in_ptr);
    };

    char m_addr[80];
    char m_type_tags[8];
    Value m_value;

    uint8_t m_addr_len;
    uint8_t m_type_len;
    uint8_t m_values_len;

public:
    // Constructors
    OSCPark();
    OSCPark(const uint8_t* in_buffer);
    OSCPark(const char* in_address);

    // Destructor
    ~OSCPark() { clear(); }

    // Methods
    void init(const uint8_t* in_buffer);
    void init(const char* in_address);
    void set_int(const int in_int);
    void set_float(const float in_float);
    void set_string(const char* in_str);
    void send(Print& in_p);
    void clear();

    // Serial Monitor debug
    void print() const;
    void info() const;

    // Accessors
    const char* get_addr() const { return m_addr; }
    const char* get_addr_cmp() { return m_addr; } // Non-const return to be used in strncmp
    const char* get_type() const { return m_type_tags; }
    int32_t get_int() const { return m_value.data.i_value; }
    float get_float() const { return m_value.data.f_value; }
    char* get_str() const { return m_value.data.s_value; }
    uint8_t get_addr_len() const { return m_addr_len; }
    uint8_t get_type_len() const { return m_type_len; }
    uint8_t get_values_len() const { return m_values_len; }
};

#endif // NBAPARK_H