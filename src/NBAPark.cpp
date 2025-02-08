#include "NBAPark.h"

// BasketSensor Class (begin)
// Constructors
BasketSensor::BasketSensor(uint8_t in_trig_pin, uint8_t in_echo_pin) : m_trig_pin(in_trig_pin), m_echo_pin(in_echo_pin)
{
    // Set trigger and echo pins
    pinMode(m_trig_pin, OUTPUT);
    pinMode(m_echo_pin, INPUT);
}

// Methods
bool BasketSensor::ball_detected()
{
    float distance = get_ultrasonic_distance();

    return distance < BALL_DETECTION_THRESHOLD && distance > 0;
}

float BasketSensor::get_ultrasonic_distance()
{
    // Send a pulse to the ultrasonic sensor
    digitalWrite(m_trig_pin, LOW);
    delayMicroseconds(2);
    digitalWrite(m_trig_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(m_trig_pin, LOW);

    unsigned long duration = pulseIn(m_echo_pin, HIGH);   // Read the echo signal
    return (duration * SOUND_SPEED) / 2; // Caculate distance in centimeters
}

// BasketSensor Class (end)

// Timer Class (begin)
// Constructors
Timer::Timer() : m_start_time(millis()), m_offset_time(ULONG_MAX)
{
    m_offset_time = ULONG_MAX - m_start_time + 1;
}

void Timer::reset_timer()
{
    m_start_time = millis();
    m_offset_time = ULONG_MAX - m_start_time + 1;
}

unsigned long Timer::get_elapsed_time() const
{
    unsigned long now = millis();
    // Calculate elapsed time, handling overflow by adding offset time
    unsigned long elapsed = (now >= m_start_time) ? (now - m_start_time) : (m_offset_time + now);

    return elapsed / 1000; // Converts to seconds
}

// Timer (end)

// MVPHoops (begin)
// Constructors
MVPHoopsLayout::MVPHoopsLayout() : m_time(0), m_num_hoops(DEFAULT_NUM_MVP_HOOPS)
{
    for (int i = 0; i < m_num_hoops, ++i)
    {
        m_valid_hoops[i] = true;
    }
}

MVPHoopsLayout::MVPHoopsLayout(int in_time, bool in_hoops[]) : time(in_time), hoop0(in_hoop0), hoop1(in_hoop1), hoop2(in_hoop2) {}