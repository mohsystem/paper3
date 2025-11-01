#include <iostream>
#include <vector>
#include <string>

/**
 * @brief Retrieves a vehicle from the vector at a specified index.
 * 
 * Includes bounds checking to prevent out-of-bounds access.
 * 
 * @param vehicles The vector of vehicle strings.
 * @param index The index of the vehicle to retrieve.
 * @return The vehicle string at the given index, or an error message if invalid.
 */
std::string getVehicle(const std::vector<std::string>& vehicles, int index) {
    if (index >= 0 && index < vehicles.size()) {
        return vehicles[index];
    } else {
        return "Error: Index " + std::to_string(index) + " is out of bounds.";
    }
}

/**
 * @brief Loops through the vector and prints each vehicle.
 * 
 * @param vehicles The vector of vehicle strings to print.
 */
void printAllVehicles(const std::vector<std::string>& vehicles) {
    std::cout << "\n--- All Vehicles ---" << std::endl;
    for (const std::string& vehicle : vehicles) {
        std::cout << vehicle << std::endl;
    }
    std::cout << "--------------------" << std::endl;
}

int main() {
    std::vector<std::string> vehicles = {"Car", "Bike", "Bus", "Train", "Plane"};

    // Print all vehicles
    printAllVehicles(vehicles);

    // 5 Test Cases
    std::cout << "\n--- Test Cases ---" << std::endl;
    int testIndices[] = {1, 3, 0, -1, 5};

    for (int i = 0; i < 5; ++i) {
        int index = testIndices[i];
        std::cout << "Test Case " << (i + 1) << ": Getting vehicle at index " << index << std::endl;
        std::string vehicle = getVehicle(vehicles, index);
        std::cout << "Result: " << vehicle << std::endl;
    }

    return 0;
}