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

// Layout (begin)
// Constructors
Layout::Layout() : time(0), id(LAYOUT_0) {}

Layout::Layout(int in_time, LayoutId in_layout_id) : time(in_time), id(in_layout_id) {}

const bool Layout::POSSIBLE_LAYOUTS[LayoutId::NUM_LAYOUTS][NUM_MVP_HOOPS] = {
    {0, 0, 0}, // LAYOUT_0
    {1, 1, 1}, // LAYOUT_1
    {1, 0, 0}, // LAYOUT_2
    {0, 1, 0}, // LAYOUT_3
    {0, 0, 1}, // LAYOUT_4
    {1, 1, 0}, // LAYOUT_5
    {1, 0, 1}, // LAYOUT_6
    {0, 1, 1}, // LAYOUT_7
};
// Layout (end)


// MVPHoopsLayouts (begin)
// Constructors
MVPHoopsLayouts::MVPHoopsLayouts(const Layout* in_layouts_arr, const uint8_t in_size) : m_curr(0), m_next(1)
{
    if (!validate_layouts_arr(in_layouts_arr, in_size))
    {
        // Invalid - raise error
    }
    m_layouts_arr = in_layouts_arr;

    // Copy the bool array from the Layout obj id
    copy_layout(&m_layouts_arr[m_curr]);
}

// Iterate over the array to validate its layouts and check its size argument
bool MVPHoopsLayouts::validate_layouts_arr(const Layout* in_layouts_arr, const uint8_t in_size)
{
    if (!in_layouts_arr)
    {
        // Handle nullptr error
        return false;
    }
    else if (in_layouts_arr[0].id == Layout::LAYOUT_STOP || in_size < 2)
    {
        // Empty array or without a valid layouts
        return false;
    }

    uint8_t i = 0;
    Layout curr_layout = in_layouts_arr[0];

    // Iterate ultil the second to last element
    for(; i < in_size - 1 && curr_layout.id != Layout::LAYOUT_STOP; curr_layout = in_layouts_arr[++i])
    {
        if (curr_layout.id > Layout::NUM_LAYOUTS || curr_layout.id < 0) return false;
    }

    // Checks the in_size and if the last element in in_layouts_arr is the sentinel value
    return (i == in_size - 1 && curr_layout.id == Layout::LAYOUT_STOP);
}

// Copy bool array from Layout::POSSIBLE_LAYOUTS into m_curr_layout
void MVPHoopsLayouts::copy_layout(const Layout* in_layout)
{
    const bool* layout_ptr = in_layout->get_bool_layout();
    for (uint8_t i = 0; i < NUM_MVP_HOOPS; ++i)
        m_curr_layout[i] = layout_ptr[i];
}

// Han
MVPHoopsLayouts::MVPState MVPHoopsLayouts::update(const int in_time)
{
    if (in_time > m_layouts_arr[m_next].time)
    {
        if (m_layouts_arr[++m_curr].id == Layout::LAYOUT_STOP)
        {
            // End of the transitions
            return MVP_GAME_OVER;
        }

        copy_layout(&m_layouts_arr[m_next++]);
    }
    else if (in_time < m_layouts_arr[m_curr].time)
    {
        return MVP_HOLD;
    }

    return MVP_RUNNING;
}

void MVPHoopsLayouts::reset()
{
    m_curr = 0;
    m_next = 1;
    copy_layout(&m_layouts_arr[m_curr]);
}
// MVPHoopsLayouts (end)


// OSCPark (begin)
// Constructors
// Default
OSCPark::OSCPark() :  m_addr{'\0'}, m_type_tags{'\0'}, m_addr_len(0), m_type_len(0), m_values_len(0) {}

OSCPark::OSCPark(const uint8_t* in_buffer)
{
    init(in_buffer);
}

void OSCPark::init(const uint8_t* in_buffer)
{
    /*Serial.print("Raw bytes: ");
    for (int i = 0; i < 65; ++i)
    {  // Adjust size to print enough bytes
        Serial.print(in_buffer[i], HEX);
        Serial.print(" ");
    }*/
    const uint8_t* ptr = in_buffer;

    // Extract address pattern (null-terminated, 4-byte aligned)
    m_addr_len = strnlen((const char*)ptr, sizeof(m_addr));
    memcpy(m_addr, ptr, m_addr_len);
    m_addr[m_addr_len] = '\0';
    ptr += (m_addr_len + 1 + 3) & ~3;  // Move to 4-byte boundary (m_addr_len + 1 null char)

    // Extract type tag string (starts with ',')
    m_type_len = strnlen((const char*)ptr, sizeof(m_type_tags));
    memcpy(m_type_tags, ptr + 1, m_type_len); // Skip the byte containing ',' by copying from the ptr + 1
    m_type_tags[m_type_len] = '\0';
    ptr += (m_type_len + 1 + 3) & ~3;

    // Extract value (currently only support messages with one type_tag)
    m_value.setup(m_type_tags[0], ptr);
}

void OSCPark::Value::setup(const char in_type_tag, const uint8_t* in_ptr)
{
    uint8_t str_size = 0;
    char* str_buffer = nullptr;

    switch (in_type_tag)
    {
        case 'i':
            Serial.println("INT");
            type_tag = 'i';
            memcpy(&data.i_value, in_ptr, sizeof(data.i_value));
            data.i_value = __builtin_bswap32(data.i_value);
            break;
        case 'f':
            Serial.println("FLOAT");
            type_tag = 'f';
            memcpy(&data.f_value, in_ptr, sizeof(data.f_value));
            uint32_t temp = __builtin_bswap32(*reinterpret_cast<uint32_t*>(&data.f_value));
            data.f_value = *reinterpret_cast<float*>(&temp);
            break;
        case 's':
            Serial.println("STRING");
            type_tag = 's';
            str_size = strnlen((const char*)in_ptr, 255);
            str_buffer = new char[str_size + 1];
            memcpy(str_buffer, in_ptr, str_size);
            str_buffer[str_size] = '\0';
            data.s_value = str_buffer;
            break;
        default:
            Serial.println("No valid type flag parsed...");
            break;
    }
}

void OSCPark::clear()
{
    m_addr[0] = '\0';
    m_type_tags[0] = '\0';
    m_addr_len = 0;
    m_type_len = 0;
    m_values_len = 0;

    switch (m_value.type_tag)
    {
        case 'i':
            m_value.data.i_value = 0;
            break;
        case 'f':
            m_value.data.f_value = 0;
            break;
        case 's':
            delete[] m_value.data.s_value;
            m_value.data.s_value = nullptr;
            break;
    }
    m_value.type_tag = '\0';
}

// Prints to the Serial Monitor the characters representation of the OSC message in the obj
void OSCPark::print() const
{
    if (m_addr_len <= 0)
    {
        Serial.println("OSCPark obj is empty...");
    }
    else
    {
        char osc_message_buffer[255];

        // Currently only support messages with one type_tag
        Serial.print("The type tag is: ");
        Serial.println(m_type_tags[0]);
        switch (m_type_tags[0])
        {
            case 'i':
                snprintf((char*)osc_message_buffer, sizeof(osc_message_buffer), "%s,%s(%ld)", m_addr, m_type_tags, m_value.data.i_value);
                break;
            case 'f':
                // Convert float to string with fixed precision
                char float_str[7];  // Allocate a buffer for the string representation of the float (4 decimals + '.' + the integer part + '\0')
                dtostrf(m_value.data.f_value, 1, 4, float_str);  // dtostrf converts float to string with 4 decimals
                snprintf((char*)osc_message_buffer, sizeof(osc_message_buffer), "%s,%s(%s)", m_addr, m_type_tags, float_str);
                break;
            case 's':
                snprintf((char*)osc_message_buffer, sizeof(osc_message_buffer), "%s,%s(%s)", m_addr, m_type_tags, m_value.data.s_value);
                break;
            default:
                snprintf((char*)osc_message_buffer, sizeof(osc_message_buffer), "%s,%s(error parsing the value)", m_addr, m_type_tags);
                break;
        }
        // Parentheses added after the type tags for clarity
        Serial.println(osc_message_buffer);
    }
}

// Prints to the Serial Monitor the info of each member var of the obj
void OSCPark::info() const
{
    char buffer[255];

    snprintf((char*)buffer, sizeof(buffer), "Address: %s", m_addr);
    Serial.println(buffer);

    snprintf((char*)buffer, sizeof(buffer), "Type tags: %s", m_type_tags);
    Serial.println(buffer);

    // Currently only support messages with one type_tag
    switch (m_type_tags[0])
    {
        case 'i':
            snprintf((char*)buffer, sizeof(buffer), "m_value.data.i_value: %ld", m_value.data.i_value);
            break;
        case 'f':
            // Convert float to string with fixed precision
            char float_str[7];  // Allocate a buffer for the string representation of the float (4 decimals + '.' + the integer part + '\0')
            dtostrf(m_value.data.f_value, 1, 4, float_str);  // dtostrf converts float to string with 6 decimals
            snprintf((char*)buffer, sizeof(buffer), "m_value.data.f_value: %s", float_str);
            break;
        case 's':
            snprintf((char*)buffer, sizeof(buffer), "m_value.data.s_value: %s", m_value.data.s_value);
            break;
    }
    Serial.println(buffer);

    snprintf((char*)buffer, sizeof(buffer), "m_addr_len: %d", m_addr_len);
    Serial.println(buffer);

    snprintf((char*)buffer, sizeof(buffer), "m_type_len: %d", m_type_len);
    Serial.println(buffer);
}
// OSCPark (end)