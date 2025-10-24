
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Struct with bit fields for compact storage
// Security: Using fixed-width types for predictable behavior
struct DeviceStatus {
    // Use unsigned types to avoid implementation-defined behavior
    uint8_t power_on : 1;        // 1 bit: power state
    uint8_t error_flag : 1;      // 1 bit: error indicator
    uint8_t mode : 3;            // 3 bits: operation mode (0-7)
    uint8_t priority : 2;        // 2 bits: priority level (0-3)
    uint8_t reserved : 1;        // 1 bit: reserved
    uint8_t battery_level : 7;   // 7 bits: battery percentage (0-100)
    uint8_t _pad1 : 1;           // Padding
    uint16_t temperature : 10;   // 10 bits: temperature (0-1023)
    uint16_t _pad2 : 6;          // Padding
};

// Security: Initialize all fields to zero to prevent undefined behavior
void initDeviceStatus(struct DeviceStatus* status) {
    if (status == NULL) {
        return; // Security: NULL pointer check
    }
    // Security: Clear all memory to ensure no uninitialized data
    memset(status, 0, sizeof(struct DeviceStatus));
}

// Security: Validate input before setting battery level
bool setBatteryLevel(struct DeviceStatus* status, uint8_t level) {
    if (status == NULL) {
        return false; // Security: NULL pointer check
    }
    if (level > 100) {
        return false; // Security: Range validation
    }
    status->battery_level = level;
    return true;
}

// Security: Validate input before setting mode
bool setMode(struct DeviceStatus* status, uint8_t m) {
    if (status == NULL) {
        return false; // Security: NULL pointer check
    }
    if (m > 7) {
        return false; // Security: Only 3 bits (0-7)
    }
    status->mode = m;
    return true;
}

// Security: Validate input before setting temperature
bool setTemperature(struct DeviceStatus* status, uint16_t temp) {
    if (status == NULL) {
        return false; // Security: NULL pointer check
    }
    if (temp > 1023) {
        return false; // Security: Only 10 bits (0-1023)
    }
    status->temperature = temp;
    return true;
}

// Security: Validate input before setting priority
bool setPriority(struct DeviceStatus* status, uint8_t p) {
    if (status == NULL) {
        return false; // Security: NULL pointer check
    }
    if (p > 3) {
        return false; // Security: Only 2 bits (0-3)
    }
    status->priority = p;
    return true;
}

// Security: Safe print function with NULL check
void printDeviceStatus(const struct DeviceStatus* status) {
    if (status == NULL) {
        fprintf(stderr, "Error: NULL pointer\\n");
        return;
    }
    printf("Power: %s\\n", status->power_on ? "ON" : "OFF");
    printf("Error: %s\\n", status->error_flag ? "YES" : "NO");
    printf("Mode: %u\\n", (unsigned int)status->mode);
    printf("Priority: %u\\n", (unsigned int)status->priority);
    printf("Battery: %u%%\\n", (unsigned int)status->battery_level);
    printf("Temperature: %u\\n", (unsigned int)status->temperature);
    printf("---\\n");
}

int main(void) {
    // Test case 1: Default initialization
    printf("Test 1: Default initialization\\n");
    struct DeviceStatus device1;
    initDeviceStatus(&device1);
    printDeviceStatus(&device1);
    
    // Test case 2: Valid settings
    printf("Test 2: Setting valid values\\n");
    struct DeviceStatus device2;
    initDeviceStatus(&device2);
    device2.power_on = 1;
    device2.error_flag = 0;
    if (!setMode(&device2, 3)) {
        fprintf(stderr, "Failed to set mode\\n");
    }
    if (!setPriority(&device2, 2)) {
        fprintf(stderr, "Failed to set priority\\n");
    }
    if (!setBatteryLevel(&device2, 75)) {
        fprintf(stderr, "Failed to set battery level\\n");
    }
    if (!setTemperature(&device2, 512)) {
        fprintf(stderr, "Failed to set temperature\\n");
    }
    printDeviceStatus(&device2);
    
    // Test case 3: Boundary values
    printf("Test 3: Boundary values\\n");
    struct DeviceStatus device3;
    initDeviceStatus(&device3);
    device3.power_on = 1;
    device3.error_flag = 1;
    if (!setMode(&device3, 7)) { // Maximum for 3 bits
        fprintf(stderr, "Failed to set mode\\n");
    }
    if (!setPriority(&device3, 3)) { // Maximum for 2 bits
        fprintf(stderr, "Failed to set priority\\n");
    }
    if (!setBatteryLevel(&device3, 100)) { // Maximum battery
        fprintf(stderr, "Failed to set battery level\\n");
    }
    if (!setTemperature(&device3, 1023)) { // Maximum for 10 bits
        fprintf(stderr, "Failed to set temperature\\n");
    }
    printDeviceStatus(&device3);
    
    // Test case 4: Invalid input validation
    printf("Test 4: Invalid input handling\\n");
    struct DeviceStatus device4;
    initDeviceStatus(&device4);
    // These should fail and return false
    if (!setMode(&device4, 8)) { // Too large for 3 bits
        printf("Correctly rejected invalid mode (8)\\n");
    }
    if (!setBatteryLevel(&device4, 101)) { // Over 100%%
        printf("Correctly rejected invalid battery level (101)\\n");
    }
    if (!setTemperature(&device4, 1024)) { // Too large for 10 bits
        printf("Correctly rejected invalid temperature (1024)\\n");
    }
    if (!setPriority(&device4, 4)) { // Too large for 2 bits
        printf("Correctly rejected invalid priority (4)\\n");
    }
    printDeviceStatus(&device4);
    
    // Test case 5: Mixed operations
    printf("Test 5: Mixed operations\\n");
    struct DeviceStatus device5;
    initDeviceStatus(&device5);
    device5.power_on = 1;
    device5.error_flag = 0;
    if (setMode(&device5, 5) && setPriority(&device5, 1) && 
        setBatteryLevel(&device5, 50) && setTemperature(&device5, 256)) {
        printf("All values set successfully\\n");
    }
    printDeviceStatus(&device5);
    
    return 0;
}
