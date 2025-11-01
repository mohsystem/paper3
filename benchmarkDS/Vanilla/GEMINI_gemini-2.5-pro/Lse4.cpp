#include <iostream>
#include <vector>
#include <string>

/**
 * Accesses a vector of vehicles at a given index.
 *
 * @param vehicles A vector of vehicle names.
 * @param index The index to access.
 * @return The vehicle name at the specified index, or an error message if the index is out of bounds.
 */
std::string getVehicleByIndex(const std::vector<std::string>& vehicles, int index) {
    if (index >= 0 && index < vehicles.size()) {
        return vehicles[index];
    } else {
        return "Error: Index is out of bounds.";
    }
}

int main() {
    std::vector<std::string> vehicles = {"car", "bus", "bike", "train", "plane"};

    // Test cases
    std::cout << "Index 0: " << getVehicleByIndex(vehicles, 0) << std::endl;  // Expected: car
    std::cout << "Index 2: " << getVehicleByIndex(vehicles, 2) << std::endl;  // Expected: bike
    std::cout << "Index 4: " << getVehicleByIndex(vehicles, 4) << std::endl;  // Expected: plane
    std::cout << "Index -1: " << getVehicleByIndex(vehicles, -1) << std::endl; // Expected: Error
    std::cout << "Index 5: " << getVehicleByIndex(vehicles, 5) << std::endl;  // Expected: Error

    return 0;
}