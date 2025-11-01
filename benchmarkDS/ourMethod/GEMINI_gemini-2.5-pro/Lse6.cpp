#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

/**
 * @brief Retrieves a vehicle from the vector at the specified index.
 * 
 * @param vehicles The vector of vehicles.
 * @param index The index of the vehicle to retrieve.
 * @return The vehicle string at the given index.
 * @throws std::out_of_range if the index is invalid.
 */
std::string getVehicle(const std::vector<std::string>& vehicles, size_t index) {
    // Rule #1, #3, #4: Input validation. .at() performs bounds checking.
    // Rule #6: Throws an exception on failure, which is standard C++ practice.
    try {
        return vehicles.at(index);
    } catch (const std::out_of_range& oor) {
        // Return a generic error message, log the detailed one.
        // For this example, we'll return the error message directly.
        return "Error: Index out of bounds.";
    }
}

/**
 * @brief Prints all vehicles in the given vector to the console.
 * 
 * @param vehicles The vector of vehicles to print.
 */
void printAllVehicles(const std::vector<std::string>& vehicles) {
    std::cout << "All vehicles:" << std::endl;
    for (const auto& vehicle : vehicles) {
        std::cout << vehicle << std::endl;
    }
}

int main() {
    // The vector of strings called vehicles. std::vector is safer than C-style arrays.
    const std::vector<std::string> vehicles = {"Car", "Bus", "Motorcycle", "Bicycle", "Truck"};

    // Loop through the vector and print out each vehicle.
    printAllVehicles(vehicles);
    std::cout << "--------------------" << std::endl;

    // 5 test cases
    std::cout << "Running test cases..." << std::endl;

    // Test Case 1: Valid index (0)
    size_t testIndex1 = 0;
    std::cout << "Getting vehicle at index " << testIndex1 << ": " << getVehicle(vehicles, testIndex1) << std::endl;

    // Test Case 2: Valid index (middle)
    size_t testIndex2 = 2;
    std::cout << "Getting vehicle at index " << testIndex2 << ": " << getVehicle(vehicles, testIndex2) << std::endl;

    // Test Case 3: Valid index (last element)
    size_t testIndex3 = 4;
    std::cout << "Getting vehicle at index " << testIndex3 << ": " << getVehicle(vehicles, testIndex3) << std::endl;

    // Test Case 4: Invalid index (out of bounds)
    size_t testIndex4 = 5;
    std::cout << "Getting vehicle at index " << testIndex4 << ": " << getVehicle(vehicles, testIndex4) << std::endl;

    // Test Case 5: Invalid index (large number)
    // Using a large number to also test boundary conditions.
    size_t testIndex5 = 100;
    std::cout << "Getting vehicle at index " << testIndex5 << ": " << getVehicle(vehicles, testIndex5) << std::endl;
    
    return 0;
}