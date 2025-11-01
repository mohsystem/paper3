
#include <iostream>
#include <string>
#include <array>
#include <limits>
#include <stdexcept>

// Predefined array of vehicles with compile-time bounds
constexpr std::array<const char*, 5> VEHICLES = {"Car", "Truck", "Motorcycle", "Bus", "Van"};

// Function to safely get vehicle at index with bounds checking
std::string getVehicleAtIndex(int index) {
    // Input validation: check bounds to prevent buffer overflow
    // This prevents out of bounds access vulnerabilities
    if (index < 0 || static_cast<size_t>(index) >= VEHICLES.size()) {
        return "Error: Invalid index";
    }
    
    // Safe array access after bounds validation
    return std::string(VEHICLES[static_cast<size_t>(index)]);
}

int main() {
    try {
        // Prompt user for input
        std::cout << "Enter an index of a vehicle (0-" << (VEHICLES.size() - 1) << "): ";
        
        int index;
        
        // Input validation: ensure input is an integer
        // This prevents type confusion and injection attacks
        if (!(std::cin >> index)) {
            std::cout << "Error: Invalid input format" << std::endl;
            return 1;
        }
        
        // Clear any remaining input to prevent buffer issues
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
        
        // Get vehicle using validated index
        std::string result = getVehicleAtIndex(index);
        std::cout << result << std::endl;
        
    } catch (const std::exception&) {
        // Catch exceptions - fail closed without leaking details
        std::cout << "Error: An error occurred" << std::endl;
        return 1;
    }
    
    // Test cases
    std::cout << "\\n--- Test Cases ---" << std::endl;
    std::cout << "Test 1 (index 0): " << getVehicleAtIndex(0) << std::endl;
    std::cout << "Test 2 (index 2): " << getVehicleAtIndex(2) << std::endl;
    std::cout << "Test 3 (index 4): " << getVehicleAtIndex(4) << std::endl;
    std::cout << "Test 4 (index -1): " << getVehicleAtIndex(-1) << std::endl;
    std::cout << "Test 5 (index 10): " << getVehicleAtIndex(10) << std::endl;
    
    return 0;
}
