#include <NBAPark.h>
#include <OSCMessage.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <string.h> // strcmp

BasketSensor basket(2, 3);
uint8_t ball_count;

uint8_t board_mac[] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x04};
IPAddress board_ip(172, 30, 6, 200);
IPAddress pc_ip(172, 30, 6, 58);
int resolume_port = 7001;

EthernetUDP udp;
//IPAddress mvp4_pc_ip(172, 30, 4, 104) // NPG-MVP004 >NBAPARKDINAMICO/>172.30.4.104

uint8_t osc_message_buffer[255];

// Prototypes
void send_osc_resolume(uint8_t in_ball_count);
bool mvp_game_start(const uint8_t in_buffer);.


void setup()
{
    Ethernet.begin(board_mac, board_ip);
    udp.begin(resolume_port);

    Serial.begin(115200);

    ball_count = 0;
    send_osc_resolume(ball_count);
}

void loop()
{
    int packet_size = udp.parsePacket();
    if (packet_size)
    {
        udp.read(osc_message_buffer, 255);
        if (mvp_game_start(osc_message_buffer))
        {
            Serial.println("GO!");
        }
    }

    if (basket.ball_detected())
    {
        Serial.print("Ball detected! count: ");
        Serial.println(++ball_count);
        send_osc_resolume(ball_count);
        delay(20);
    }

    delay(100);
}

void send_osc_resolume(uint8_t in_ball_count)
{
    // Construct the OSC address using snprintf to write into a buffer and inserting the ball count variable
    snprintf((char*)osc_message_buffer, sizeof(osc_message_buffer), "/composition/layers/2/clips/%d/connect", in_ball_count + 2);

    OSCMessage msg((char*)osc_message_buffer);

    udp.beginPacket(pc_ip, resolume_port);
    msg.send(udp);
    udp.endPacket();
}

bool mvp_game_start(const uint8_t* in_buffer)
{
    const uint8_t* ptr = in_buffer;

    // Extract and compare address (null-terminated, 4-byte aligned)
    char address[64];
    uint8_t addr_len = strnlen((const char*)ptr, sizeof(address));
    memcpy(address, ptr, addr_len);
    address[addr_len] = '\0';

    ptr += (addr_len + 4) & ~3; // Move to 4-byte boundary
    uint8_t type_len = strnlen((const char*)ptr, 1);
    ptr += (type_len + 4) & ~3;

    char res_clip_addr[] = "/composition/layers/1/clips/2/connected";
    if (strcmp(address, res_clip_addr) == 0)
    {
        int32_t value;
        memcpy(&value, ptr, sizeof(value));
        value = __builtin_bswap32(value);  // Convert from big-endian
        return value == 3;
    }
    return false;
}