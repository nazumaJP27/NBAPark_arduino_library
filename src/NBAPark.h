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
    bool m_valid_hoops[num_hoops];

    MVPHoopsLayout() : time(0), hoop0(1), hoop1(1), hoop2(1);
    MVPHoopsLayout(int in_time, uint8_t in_num_hoops, bool in_hoop0, bool in_hoop1, bool in_hoop2) : time(in_time), hoop0(in_hoop0), hoop1(in_hoop1), hoop2(in_hoop2) {}
};



#endif // NBAPARK_H