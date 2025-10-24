
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Struct with bit fields
typedef struct {
    uint16_t version : 4;        // 4 bits (0-15)
    uint16_t priority : 3;       // 3 bits (0-7)
    uint16_t flag : 1;           // 1 bit (0-1)
    uint16_t payloadLength : 8;  // 8 bits (0-255)
} Task115;

void Task115_init(Task115* packet) {
    packet->version = 0;
    packet->priority = 0;
    packet->flag = 0;
    packet->payloadLength = 0;
}

bool Task115_setVersion(Task115* packet, int value) {
    if (value < 0 || value > 15) {
        printf("Error: Version must be 0-15\\n");
        return false;
    }
    packet->version = (uint16_t)(value & 0x0F);
    return true;
}

bool Task115_setPriority(Task115* packet, int value) {
    if (value < 0 || value > 7) {
        printf("Error: Priority must be 0-7\\n");
        return false;
    }
    packet->priority = (uint16_t)(value & 0x07);
    return true;
}

bool Task115_setFlag(Task115* packet, int value) {
    if (value < 0 || value > 1) {
        printf("Error: Flag must be 0 or 1\\n");
        return false;
    }
    packet->flag = (uint16_t)(value & 0x01);
    return true;
}

bool Task115_setPayloadLength(Task115* packet, int value) {
    if (value < 0 || value > 255) {
        printf("Error: PayloadLength must be 0-255\\n");
        return false;
    }
    packet->payloadLength = (uint16_t)(value & 0xFF);
    return true;
}

uint16_t Task115_packToInt(const Task115* packet) {
    return ((uint16_t)(packet->version & 0x0F) << 12) |
           ((uint16_t)(packet->priority & 0x07) << 9) |
           ((uint16_t)(packet->flag & 0x01) << 8) |
           (uint16_t)(packet->payloadLength & 0xFF);
}

void Task115_unpackFromInt(Task115* packet, uint16_t packed) {
    packet->version = (packed >> 12) & 0x0F;
    packet->priority = (packed >> 9) & 0x07;
    packet->flag = (packed >> 8) & 0x01;
    packet->payloadLength = packed & 0xFF;
}

void Task115_display(const Task115* packet) {
    printf("Version: %u, Priority: %u, Flag: %u, PayloadLength: %u (Packed: 0x%04X)\\n",
           packet->version, packet->priority, packet->flag, 
           packet->payloadLength, Task115_packToInt(packet));
}

int main() {
    printf("=== Bit Field Struct Demonstration ===\\n\\n");
    
    // Test Case 1: Normal values
    printf("Test Case 1: Setting normal values\\n");
    Task115 packet1;
    Task115_init(&packet1);
    Task115_setVersion(&packet1, 4);
    Task115_setPriority(&packet1, 3);
    Task115_setFlag(&packet1, 1);
    Task115_setPayloadLength(&packet1, 128);
    Task115_display(&packet1);
    printf("\\n");
    
    // Test Case 2: Maximum values
    printf("Test Case 2: Setting maximum values\\n");
    Task115 packet2;
    Task115_init(&packet2);
    Task115_setVersion(&packet2, 15);
    Task115_setPriority(&packet2, 7);
    Task115_setFlag(&packet2, 1);
    Task115_setPayloadLength(&packet2, 255);
    Task115_display(&packet2);
    printf("\\n");
    
    // Test Case 3: Minimum values
    printf("Test Case 3: Setting minimum values\\n");
    Task115 packet3;
    Task115_init(&packet3);
    Task115_setVersion(&packet3, 0);
    Task115_setPriority(&packet3, 0);
    Task115_setFlag(&packet3, 0);
    Task115_setPayloadLength(&packet3, 0);
    Task115_display(&packet3);
    printf("\\n");
    
    // Test Case 4: Invalid values (should show errors)
    printf("Test Case 4: Attempting invalid values\\n");
    Task115 packet4;
    Task115_init(&packet4);
    Task115_setVersion(&packet4, 20);
    Task115_setPriority(&packet4, 10);
    Task115_setFlag(&packet4, 2);
    Task115_setPayloadLength(&packet4, 300);
    Task115_display(&packet4);
    printf("\\n");
    
    // Test Case 5: Pack and unpack
    printf("Test Case 5: Pack and unpack operations\\n");
    Task115 packet5;
    Task115_init(&packet5);
    Task115_setVersion(&packet5, 7);
    Task115_setPriority(&packet5, 5);
    Task115_setFlag(&packet5, 1);
    Task115_setPayloadLength(&packet5, 200);
    printf("Original: ");
    Task115_display(&packet5);
    uint16_t packed = Task115_packToInt(&packet5);
    Task115 packet6;
    Task115_unpackFromInt(&packet6, packed);
    printf("Unpacked: ");
    Task115_display(&packet6);
    
    return 0;
}
