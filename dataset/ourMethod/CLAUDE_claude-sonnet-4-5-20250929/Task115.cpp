
#include <iostream>
#include <cstdint>
#include <cstring>
#include <stdexcept>

// Struct with bit fields for compact storage of flags and small integers
// Security: Using fixed-width types for predictable bit field behavior
struct DeviceStatus {
    // Use unsigned types for bit fields to avoid implementation-defined behavior
    uint8_t power_on : 1;        // 1 bit: power state (0=off, 1=on)
    uint8_t error_flag : 1;      // 1 bit: error indicator
    uint8_t mode : 3;            // 3 bits: operation mode (0-7)
    uint8_t priority : 2;        // 2 bits: priority level (0-3)
    uint8_t reserved : 1;        // 1 bit: reserved for future use
    uint8_t battery_level : 7;   // 7 bits: battery percentage (0-100)
    uint8_t _pad1 : 1;           // Padding bit
    uint16_t temperature : 10;   // 10 bits: temperature sensor (0-1023)
    uint16_t _pad2 : 6;          // Padding bits
    
    // Constructor with default values - Security: Initialize all fields
    DeviceStatus() : power_on(0), error_flag(0), mode(0), priority(0), 
                     reserved(0), battery_level(0), _pad1(0), 
                     temperature(0), _pad2(0) {}
    
    // Validate and set battery level - Security: Input validation
    bool setBatteryLevel(uint8_t level) {
        if (level > 100) {
            return false; // Invalid input
        }
        battery_level = level;
        return true;
    }
    
    // Validate and set mode - Security: Range check
    bool setMode(uint8_t m) {
        if (m > 7) {
            return false; // Only 3 bits available (0-7)
        }
        mode = m;
        return true;
    }
    
    // Validate and set temperature - Security: Range check
    bool setTemperature(uint16_t temp) {
        if (temp > 1023) {
            return false; // Only 10 bits available (0-1023)
        }
        temperature = temp;
        return true;
    }
    
    // Validate and set priority - Security: Range check
    bool setPriority(uint8_t p) {
        if (p > 3) {
            return false; // Only 2 bits available (0-3)
        }
        priority = p;
        return true;
    }
};

// Helper function to safely print device status
void printDeviceStatus(const DeviceStatus& status) {
    std::cout << "Power: " << (status.power_on ? "ON" : "OFF") << "\\n";
    std::cout << "Error: " << (status.error_flag ? "YES" : "NO") << "\\n";
    std::cout << "Mode: " << static_cast<int>(status.mode) << "\\n";
    std::cout << "Priority: " << static_cast<int>(status.priority) << "\\n";
    std::cout << "Battery: " << static_cast<int>(status.battery_level) << "%\\n";
    std::cout << "Temperature: " << status.temperature << "\\n";
    std::cout << "---\\n";
}

int main() {
    try {
        // Test case 1: Default initialization
        std::cout << "Test 1: Default initialization\\n";
        DeviceStatus device1;
        printDeviceStatus(device1);
        
        // Test case 2: Valid settings
        std::cout << "Test 2: Setting valid values\\n";
        DeviceStatus device2;
        device2.power_on = 1;
        device2.error_flag = 0;
        if (!device2.setMode(3)) {
            std::cerr << "Failed to set mode\\n";
        }
        if (!device2.setPriority(2)) {
            std::cerr << "Failed to set priority\\n";
        }
        if (!device2.setBatteryLevel(75)) {
            std::cerr << "Failed to set battery level\\n";
        }
        if (!device2.setTemperature(512)) {
            std::cerr << "Failed to set temperature\\n";
        }
        printDeviceStatus(device2);
        
        // Test case 3: Boundary values
        std::cout << "Test 3: Boundary values\\n";
        DeviceStatus device3;
        device3.power_on = 1;
        device3.error_flag = 1;
        if (!device3.setMode(7)) { // Maximum for 3 bits
            std::cerr << "Failed to set mode\\n";
        }
        if (!device3.setPriority(3)) { // Maximum for 2 bits
            std::cerr << "Failed to set priority\\n";
        }
        if (!device3.setBatteryLevel(100)) { // Maximum battery
            std::cerr << "Failed to set battery level\\n";
        }
        if (!device3.setTemperature(1023)) { // Maximum for 10 bits
            std::cerr << "Failed to set temperature\\n";
        }
        printDeviceStatus(device3);
        
        // Test case 4: Invalid input validation
        std::cout << "Test 4: Invalid input handling\\n";
        DeviceStatus device4;
        // These should fail and return false
        if (!device4.setMode(8)) { // Too large for 3 bits
            std::cout << "Correctly rejected invalid mode (8)\\n";
        }
        if (!device4.setBatteryLevel(101)) { // Over 100%
            std::cout << "Correctly rejected invalid battery level (101)\\n";
        }
        if (!device4.setTemperature(1024)) { // Too large for 10 bits
            std::cout << "Correctly rejected invalid temperature (1024)\\n";
        }
        if (!device4.setPriority(4)) { // Too large for 2 bits
            std::cout << "Correctly rejected invalid priority (4)\\n";
        }
        printDeviceStatus(device4);
        
        // Test case 5: Mixed operations
        std::cout << "Test 5: Mixed operations\\n";
        DeviceStatus device5;
        device5.power_on = 1;
        device5.error_flag = 0;
        if (device5.setMode(5) && device5.setPriority(1) && 
            device5.setBatteryLevel(50) && device5.setTemperature(256)) {
            std::cout << "All values set successfully\\n";
        }
        printDeviceStatus(device5);
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\\n";
        return 1;
    }
    
    return 0;
}
