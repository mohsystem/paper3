#include <iostream>
#include <vector>
#include <string>

/**
 * @brief Securely retrieves a vehicle from the vector by index.
 * 
 * @param vehicles The vector of vehicles.
 * @param index The index to retrieve.
 * @return The vehicle at the specified index or an error message if the index is invalid.
 */
std::string getVehicleAtIndex(const std::vector<std::string>& vehicles, int index) {
    // Security check: Ensure the index is within the valid range.
    if (index >= 0 && index < vehicles.size()) {
        return vehicles[index];
    } else {
        return "Error: Invalid index provided.";
    }
}

/**
 * @brief Loops through the vector and prints each vehicle.
 * 
 * @param vehicles The vector of vehicles to print.
 */
void printAllVehicles(const std::vector<std::string>& vehicles) {
    std::cout << "\n--- All Vehicles ---" << std::endl;
    for (const auto& vehicle : vehicles) {
        std::cout << vehicle << std::endl;
    }
    std::cout << "--------------------" << std::endl;
}

int main() {
    std::vector<std::string> vehicles = {"Car", "Bike", "Bus", "Train", "Plane"};

    // --- 5 Test Cases ---
    std::cout << "--- Test Cases ---" << std::endl;
    
    // Test Case 1: Valid index (first element)
    int index1 = 0;
    std::cout << "Requesting vehicle at index " << index1 << ": " << getVehicleAtIndex(vehicles, index1) << std::endl;

    // Test Case 2: Valid index (middle element)
    int index2 = 2;
    std::cout << "Requesting vehicle at index " << index2 << ": " << getVehicleAtIndex(vehicles, index2) << std::endl;
    
    // Test Case 3: Valid index (last element)
    int index3 = 4;
    std::cout << "Requesting vehicle at index " << index3 << ": " << getVehicleAtIndex(vehicles, index3) << std::endl;
    
    // Test Case 4: Invalid index (out of bounds - too high)
    int index4 = 10;
    std::cout << "Requesting vehicle at index " << index4 << ": " << getVehicleAtIndex(vehicles, index4) << std::endl;
    
    // Test Case 5: Invalid index (out of bounds - negative)
    int index5 = -1;
    std::cout << "Requesting vehicle at index " << index5 << ": " << getVehicleAtIndex(vehicles, index5) << std::endl;
    
    std::cout << "------------------" << std::endl;

    // Loop through and print all vehicles
    printAllVehicles(vehicles);

    return 0;
}