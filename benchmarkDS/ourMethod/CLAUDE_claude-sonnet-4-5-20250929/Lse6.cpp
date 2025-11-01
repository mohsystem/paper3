
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>

// Define maximum array size to prevent excessive memory usage
const int MAX_ARRAY_SIZE = 100;

// Returns vehicle at specified index with input validation
std::string getVehicleByIndex(const std::vector<std::string>& vehicles, int index) {
    // Validate input: check if index is within valid range
    // This prevents out-of-bounds access and potential buffer overflows
    if (index < 0 || index >= static_cast<int>(vehicles.size())) {
        return "Error: Invalid index";
    }
    
    // Return the vehicle at the specified index
    return vehicles[index];
}

// Prints all vehicles in the vector with validation
void printAllVehicles(const std::vector<std::string>& vehicles) {
    // Loop through vector and print each vehicle
    std::cout << "All vehicles:" << std::endl;
    for (const auto& vehicle : vehicles) {
        // Validate each element is not empty before printing
        if (!vehicle.empty()) {
            std::cout << vehicle << std::endl;
        }
    }
}

int main() {
    // Create vector of vehicle strings
    std::vector<std::string> vehicles = {"Car", "Truck", "Motorcycle", "Bus", "Bicycle"};
    
    // Test cases
    std::cout << "=== Test Case 1: Valid index 0 ===" << std::endl;
    std::cout << getVehicleByIndex(vehicles, 0) << std::endl;
    printAllVehicles(vehicles);
    
    std::cout << "\\n=== Test Case 2: Valid index 2 ===" << std::endl;
    std::cout << getVehicleByIndex(vehicles, 2) << std::endl;
    printAllVehicles(vehicles);
    
    std::cout << "\\n=== Test Case 3: Invalid negative index ===" << std::endl;
    std::cout << getVehicleByIndex(vehicles, -1) << std::endl;
    
    std::cout << "\\n=== Test Case 4: Invalid index too large ===" << std::endl;
    std::cout << getVehicleByIndex(vehicles, 10) << std::endl;
    
    std::cout << "\\n=== Test Case 5: Valid index at boundary ===" << std::endl;
    std::cout << getVehicleByIndex(vehicles, 4) << std::endl;
    
    // Interactive prompt with input validation
    try {
        std::cout << "\\n=== Interactive Mode ===" << std::endl;
        std::cout << "Enter vehicle index (0-" << (vehicles.size() - 1) << "): ";
        
        std::string input_line;
        if (std::getline(std::cin, input_line)) {
            // Validate input is a valid integer using stringstream
            std::stringstream ss(input_line);
            int user_index;
            
            // Check if conversion succeeds and entire string was consumed
            if (ss >> user_index && ss.eof()) {
                std::string result = getVehicleByIndex(vehicles, user_index);
                std::cout << "Result: " << result << std::endl;
            } else {
                // Generic error message to avoid leaking internal details
                std::cout << "Error: Invalid input format" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        // Generic error message to avoid leaking internal details
        std::cout << "Error: Unable to process input" << std::endl;
    } catch (...) {
        std::cout << "Error: An unexpected error occurred" << std::endl;
    }
    
    return 0;
}
