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
    for (int i = 0; i < m_num_hoops; ++i)
    {
        m_valid_hoops[i] = true;
    }
}

MVPHoopsLayout::MVPHoopsLayout(int in_time, const bool *in_valid_hoops) : m_time(in_time), m_num_hoops(DEFAULT_NUM_MVP_HOOPS)
{
    for (int i = 0; i < m_num_hoops; ++i) // Checks for nullptr
    {
        in_valid_hoops[i] ? (m_valid_hoops[i] = in_valid_hoops[i]) : (m_valid_hoops[i] = false);
    }
}


// Layout (begin)
// Constructors
Layout::Layout() : time(0), id(LAYOUT_1) {}

Layout::Layout(int in_time, LayoutId in_layout_id) : time(in_time), id(in_layout_id) {}

const bool Layout::POSSIBLE_LAYOUTS[LayoutId::NUM_LAYOUTS][3] = {
    {0, 0, 0}, // LAYOUT_1
    {1, 1, 1}, // LAYOUT_2
    {1, 0, 0}, // LAYOUT_3
    {0, 1, 0}, // LAYOUT_4
    {0, 0, 1}, // LAYOUT_5
    {1, 1, 0}, // LAYOUT_6
    {1, 0, 1}, // LAYOUT_7
    {0, 1, 1}, // LAYOUT_8
};
// Layout (end)


// MVPHoopsLayouts (begin)
// Constructors
MVPHoopsLayouts::MVPHoopsLayouts(const LayoutId* in_layouts_arr, const uint8_t in_size) : m_curr(0), m_next(1)
{
    if (!validate_layouts(in_layouts_arr, in_size))
    {
        // Invalid - raise error
    }
    m_layouts_arr = in_layouts_arr;
}

// Iterate over the array to validate its layouts and check its size argument
bool MVPHoopsLayouts::validate_layouts_arr(const Layout* in_layouts_arr, const uint8_t in_size)
{
    if (!in_layouts_arr)
    {
        // Handle nullptr error
        return false;
    }
    else if (in_layouts_arr[0].id == LAYOUT_STOP || in_size < 2)
    {
        // Empty array or without a valid layouts
        return false;
    }

    uint8_t i = 0;
    Layout curr_layout = in_layouts_arr[0];

    // Iterate ultil the second to last element
    for(; i < in_size - 1 && curr_layout.id != LAYOUT_STOP; curr_layout = in_layouts_arr[++i])
    {
        if (curr_layout.id > NUM_LAYOUTS || curr_layout.id < 0) return false;
    }

    // Checks the in_size and if the last element in in_layouts_arr is the sentinel value
    return (i == in_size - 1 && curr_layout.id == LAYOUT_STOP);
}