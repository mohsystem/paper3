#include <iostream>
#include <vector>
#include <string>

/**
 * Securely retrieves a vehicle from a vector by index.
 *
 * @param vehicles The vector of vehicle names.
 * @param index The index of the vehicle to retrieve.
 * @return The vehicle name at the given index or an error message if the index is invalid.
 */
std::string getVehicle(const std::vector<std::string>& vehicles, int index) {
    // Security check: Validate that the index is within the legitimate bounds of the vector.
    // The check for index >= 0 is crucial because `vehicles.size()` returns an unsigned type,
    // and a negative index would wrap around to a large positive number during comparison.
    // This prevents out-of-bounds access.
    if (index >= 0 && static_cast<size_t>(index) < vehicles.size()) {
        return vehicles[index];
    } else {
        return "Error: Index is out of bounds.";
    }
}

int main() {
    std::vector<std::string> vehicleVector = {"Car", "Motorcycle", "Truck", "Bus", "Bicycle"};

    std::cout << "--- 5 Test Cases ---" << std::endl;

    int testIndices[] = {
        0,  // Test Case 1: Valid index (lower bound)
        2,  // Test Case 2: Valid index (middle)
        4,  // Test Case 3: Valid index (upper bound)
        -1, // Test Case 4: Invalid index (negative)
        5   // Test Case 5: Invalid index (too high)
    };

    for (int index : testIndices) {
        std::cout << "Requesting vehicle at index " << index << ": "
                  << getVehicle(vehicleVector, index) << std::endl;
    }

    return 0;
}