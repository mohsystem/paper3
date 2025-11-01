#include <iostream>
#include <vector>
#include <string>
#include <limits>

// Step 1: Problem understanding - vehicles array, prompt, return selected, print all.
// Step 2: Security - validate input, handle parsing and bounds safely.
// Step 3 & 5: Secure coding and final secure output.

bool getVehicleAt(const std::vector<std::string>& vehicles, int index, std::string& outVehicle) {
    if (index < 0 || static_cast<size_t>(index) >= vehicles.size()) return false;
    outVehicle = vehicles[static_cast<size_t>(index)];
    return true;
}

void printVehicles(const std::vector<std::string>& vehicles) {
    for (size_t i = 0; i < vehicles.size(); ++i) {
        std::cout << "vehicles[" << i << "] = " << vehicles[i] << "\n";
    }
}

int main() {
    std::vector<std::string> vehicles = {"Car", "Bike", "Truck", "Bus", "Scooter"};

    std::cout << "Enter vehicle index (0-" << (vehicles.size() - 1) << "): ";
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "No input provided.\n";
    } else {
        // Safe parse
        int idx = -1;
        try {
            size_t pos = 0;
            int val = std::stoi(line, &pos);
            if (pos != line.size()) {
                std::cout << "Invalid input. Please enter an integer.\n";
            } else {
                idx = val;
            }
        } catch (...) {
            std::cout << "Invalid input. Please enter an integer.\n";
        }

        std::string selected;
        if (getVehicleAt(vehicles, idx, selected)) {
            std::cout << "Selected vehicle: " << selected << "\n";
        } else {
            std::cout << "No vehicle found at the provided index.\n";
        }
    }

    std::cout << "All vehicles:\n";
    printVehicles(vehicles);

    // 5 test cases
    std::cout << "\nTest cases:\n";
    int testIndices[5] = {0, 2, 4, -1, 10};
    for (int i = 0; i < 5; ++i) {
        std::string out;
        bool ok = getVehicleAt(vehicles, testIndices[i], out);
        std::cout << "Test " << (i + 1) << " - index " << testIndices[i] << ": " << (ok ? out : std::string("Invalid index")) << "\n";
    }

    return 0;
}