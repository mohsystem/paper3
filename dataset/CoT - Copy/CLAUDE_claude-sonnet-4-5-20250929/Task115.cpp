
#include <iostream>
#include <cstdint>
#include <iomanip>

// Struct with bit fields
struct Task115 {
    uint16_t version : 4;        // 4 bits (0-15)
    uint16_t priority : 3;       // 3 bits (0-7)
    uint16_t flag : 1;           // 1 bit (0-1)
    uint16_t payloadLength : 8;  // 8 bits (0-255)
    
    Task115() : version(0), priority(0), flag(0), payloadLength(0) {}
    
    bool setVersion(int value) {
        if (value < 0 || value > 15) {
            std::cout << "Error: Version must be 0-15" << std::endl;
            return false;
        }
        version = static_cast<uint16_t>(value & 0x0F);
        return true;
    }
    
    bool setPriority(int value) {
        if (value < 0 || value > 7) {
            std::cout << "Error: Priority must be 0-7" << std::endl;
            return false;
        }
        priority = static_cast<uint16_t>(value & 0x07);
        return true;
    }
    
    bool setFlag(int value) {
        if (value < 0 || value > 1) {
            std::cout << "Error: Flag must be 0 or 1" << std::endl;
            return false;
        }
        flag = static_cast<uint16_t>(value & 0x01);
        return true;
    }
    
    bool setPayloadLength(int value) {
        if (value < 0 || value > 255) {
            std::cout << "Error: PayloadLength must be 0-255" << std::endl;
            return false;
        }
        payloadLength = static_cast<uint16_t>(value & 0xFF);
        return true;
    }
    
    uint16_t getVersion() const { return version; }
    uint16_t getPriority() const { return priority; }
    uint16_t getFlag() const { return flag; }
    uint16_t getPayloadLength() const { return payloadLength; }
    
    uint16_t packToInt() const {
        return (static_cast<uint16_t>(version & 0x0F) << 12) |
               (static_cast<uint16_t>(priority & 0x07) << 9) |
               (static_cast<uint16_t>(flag & 0x01) << 8) |
               static_cast<uint16_t>(payloadLength & 0xFF);
    }
    
    void unpackFromInt(uint16_t packed) {
        version = (packed >> 12) & 0x0F;
        priority = (packed >> 9) & 0x07;
        flag = (packed >> 8) & 0x01;
        payloadLength = packed & 0xFF;
    }
    
    void display() const {
        std::cout << "Version: " << version 
                  << ", Priority: " << priority 
                  << ", Flag: " << flag 
                  << ", PayloadLength: " << payloadLength 
                  << " (Packed: 0x" << std::hex << std::setw(4) 
                  << std::setfill('0') << packToInt() << std::dec << ")" 
                  << std::endl;
    }
};

int main() {
    std::cout << "=== Bit Field Struct Demonstration ===" << std::endl << std::endl;
    
    // Test Case 1: Normal values
    std::cout << "Test Case 1: Setting normal values" << std::endl;
    Task115 packet1;
    packet1.setVersion(4);
    packet1.setPriority(3);
    packet1.setFlag(1);
    packet1.setPayloadLength(128);
    packet1.display();
    std::cout << std::endl;
    
    // Test Case 2: Maximum values
    std::cout << "Test Case 2: Setting maximum values" << std::endl;
    Task115 packet2;
    packet2.setVersion(15);
    packet2.setPriority(7);
    packet2.setFlag(1);
    packet2.setPayloadLength(255);
    packet2.display();
    std::cout << std::endl;
    
    // Test Case 3: Minimum values
    std::cout << "Test Case 3: Setting minimum values" << std::endl;
    Task115 packet3;
    packet3.setVersion(0);
    packet3.setPriority(0);
    packet3.setFlag(0);
    packet3.setPayloadLength(0);
    packet3.display();
    std::cout << std::endl;
    
    // Test Case 4: Invalid values (should show errors)
    std::cout << "Test Case 4: Attempting invalid values" << std::endl;
    Task115 packet4;
    packet4.setVersion(20);
    packet4.setPriority(10);
    packet4.setFlag(2);
    packet4.setPayloadLength(300);
    packet4.display();
    std::cout << std::endl;
    
    // Test Case 5: Pack and unpack
    std::cout << "Test Case 5: Pack and unpack operations" << std::endl;
    Task115 packet5;
    packet5.setVersion(7);
    packet5.setPriority(5);
    packet5.setFlag(1);
    packet5.setPayloadLength(200);
    std::cout << "Original: ";
    packet5.display();
    uint16_t packed = packet5.packToInt();
    Task115 packet6;
    packet6.unpackFromInt(packed);
    std::cout << "Unpacked: ";
    packet6.display();
    
    return 0;
}
