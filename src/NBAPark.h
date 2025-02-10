#ifndef NBAPARK_H
#define NBAPARK_H

#include <Arduino.h>

// Constants
#ifndef ULONG_MAX
#define ULONG_MAX 4294967295UL
#endif

#define SOUND_SPEED 0.0343f         // Speed of sound in centimeters per microsecond
#define BALL_DETECTION_THRESHOLD 15 // Value in centimeters
#define DEFAULT_NUM_MVP_HOOPS 3

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
    void send_OSC_message();
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

struct MVPHoopsLayout
{
    int m_time;
    uint8_t m_num_hoops;
    bool m_valid_hoops[DEFAULT_NUM_MVP_HOOPS];

    MVPHoopsLayout();
    MVPHoopsLayout(int in_time, const bool* in_valid_hoops);
};

class MVPHoopsLayouts
{
public:
    enum LayoutId 
    {
        LAYOUT_STOP, // sentinel value
        LAYOUT_1,
        LAYOUT_2,
        LAYOUT_3,
        LAYOUT_4,
        LAYOUT_5,
        LAYOUT_6,
        LAYOUT_7,
        LAYOUT_8,
        NUM_LAYOUTS
    };

    struct Layout
    {
        // Member variables
        int time;
        uint8_t num_hoops;
        bool valid_hoops[DEFAULT_NUM_MVP_HOOPS];
        // All the possible layouts for three hoops attached to the wall
        static const bool possible_layouts[NUM_LAYOUTS][3];


        // Constructors
        Layout();
        Layout(int in_time, const bool* in_valid_hoops);
        Layout(int in_time, bool in_hoop0, bool in_hoop1, bool in_hoop2);
        Layout(int in_time, LayoutId in_layout_id);
    };

private:
    // Member variables
    uint8_t m_curr;
    uint8_t m_next;
    const LayoutId* m_layouts_arr; // Pointer to an array 

public:
    // Constructors
    MVPHoopsLayouts();
    MVPHoopsLayouts(const LayoutId* in_layouts_arr, const uint8_t in_size);

private:
    // Method to iterate over layouts and ensure correct boundary checking
    bool validate_layouts(const LayoutId* in_layouts_arr, const uint8_t in_size);
};


#endif // NBAPARK_H