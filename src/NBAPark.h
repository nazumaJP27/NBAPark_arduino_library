#ifndef NBAPARK_H
#define NBAPARK_H

#include <Arduino.h>

// Constants
#define ULONG_MAX 4294967295UL      // Max value for an usigned long type (used to calculate the offset in Timer)
#define SOUND_SPEED 0.0343f         // Speed of sound in centimeters per microsecond
#define BALL_DETECTION_THRESHOLD 15 // Value in centimeters
#define NUM_MVP_HOOPS 3


class BasketSensor
{
    // Pins used by the Ultrasonic Sensor
    uint8_t m_trig_pin;
    uint8_t m_echo_pin;

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
    void reset_timer();
    unsigned long get_elapsed_time() const;
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
    MVPState update(int in_time);
    void reset();

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

    char m_addr[64];
    char m_type_tags[8];
    Value m_value;

    uint8_t m_addr_len;
    uint8_t m_type_len;
    uint8_t m_values_len;

public:
    // Constructors
    OSCPark();
    OSCPark(const uint8_t* in_buffer);

    // Destructor
    ~OSCPark() { clear(); }

    // Methods
    void init(const uint8_t* in_buffer);
    void clear();

    // Serial Monitor debug
    void print() const;
    void info() const;

    // Accessors
    const char* get_addr() const { return m_addr; }
    const char* get_type() const { return m_type_tags; }
    int32_t get_int() const { return m_value.data.i_value; }
    float get_float() const { return m_value.data.f_value; }
    char* get_str() const { return m_value.data.s_value; }
    uint8_t get_addr_len() const { return m_addr_len; }
    uint8_t get_type_len() const { return m_type_len; }
    uint8_t get_values_len() const { return m_values_len; }
};

#endif // NBAPARK_H