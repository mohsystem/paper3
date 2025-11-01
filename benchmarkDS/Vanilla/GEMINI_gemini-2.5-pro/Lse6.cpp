#include <iostream>
#include <vector>
#include <string>

/**
 * @brief Returns the vehicle at a given index from the vector.
 * @param vehicles The vector of vehicles.
 * @param index The index of the vehicle to retrieve.
 * @return The vehicle string or "Invalid index" if the index is out of bounds.
 */
std::string getVehicle(const std::vector<std::string>& vehicles, int index) {
    if (index >= 0 && index < vehicles.size()) {
        return vehicles.at(index);
    } else {
        return "Invalid index";
    }
}

/**
 * @brief Loops through the vector and prints each vehicle.
 * @param vehicles The vector of vehicles to print.
 */
void printAllVehicles(const std::vector<std::string>& vehicles) {
    std::cout << "All vehicles:" << std::endl;
    for (const std::string& vehicle : vehicles) {
        std::cout << vehicle << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    // Create a vector of strings called vehicles
    std::vector<std::string> vehicles = {"Car", "Bike", "Bus", "Train", "Plane"};

    // Loop through the vector and print out each vehicle
    printAllVehicles(vehicles);

    // 5 test cases for getting a vehicle by index
    std::cout << "--- Test Cases for getVehicle ---" << std::endl;
    
    // Test Case 1
    int index1 = 0;
    std::cout << "Vehicle at index " << index1 << ": " << getVehicle(vehicles, index1) << std::endl;

    // Test Case 2
    int index2 = 2;
    std::cout << "Vehicle at index " << index2 << ": " << getVehicle(vehicles, index2) << std::endl;

    // Test Case 3
    int index3 = 4;
    std::cout << "Vehicle at index " << index3 << ": " << getVehicle(vehicles, index3) << std::endl;

    // Test Case 4 (out of bounds - upper)
    int index4 = 5;
    std::cout << "Vehicle at index " << index4 << ": " << getVehicle(vehicles, index4) << std::endl;
    
    // Test Case 5 (out of bounds - lower)
    int index5 = -1;
    std::cout << "Vehicle at index " << index5 << ": " << getVehicle(vehicles, index5) << std::endl;

    return 0;
}