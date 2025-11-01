#include <iostream>
#include <vector>
#include <string>

// Using a vector for safety and modern C++ practices.
const std::vector<std::string> VEHICLES = {"Car", "Bike", "Bus", "Truck", "Van"};

/**
 * @brief Gets a vehicle from the predefined list by its index.
 * 
 * @param index The index of the vehicle to retrieve.
 * @return The name of the vehicle as a std::string, or "Invalid index" if the index is out of bounds.
 */
std::string getVehicleByIndex(int index) {
    // Security Check: Validate that the index is within the valid range.
    // Using .size() which is unsigned, so we must check for negative index first.
    if (index < 0 || static_cast<size_t>(index) >= VEHICLES.size()) {
        return "Invalid index";
    }
    return VEHICLES[index];
}

int main() {
    // 5 Test Cases
    int testIndexes[] = {0, 2, 4, -1, 5};

    std::cout << "Running CPP Test Cases:" << std::endl;
    for (int index : testIndexes) {
        std::string vehicle = getVehicleByIndex(index);
        std::cout << "Index: " << index << " -> Vehicle: " << vehicle << std::endl;
    }

    return 0;
}