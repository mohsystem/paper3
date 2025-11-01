#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <limits>
#include <cstdlib>

std::optional<std::string> getVehicle(const std::vector<std::string>& vehicles, int index) {
    if (index < 0 || static_cast<size_t>(index) >= vehicles.size()) {
        return std::nullopt;
    }
    return vehicles[static_cast<size_t>(index)];
}

void printVehicles(const std::vector<std::string>& vehicles) {
    for (size_t i = 0; i < vehicles.size(); ++i) {
        std::cout << "Vehicle[" << i << "]: " << vehicles[i] << "\n";
    }
}

int main() {
    std::vector<std::string> vehicles = {"Car", "Bike", "Truck", "Boat", "Plane"};

    // Optional interactive prompt controlled by environment variable
    if (std::getenv("RUN_INTERACTIVE")) {
        std::cout << "Enter vehicle index: ";
        std::string line;
        if (std::getline(std::cin, line)) {
            try {
                size_t pos = 0;
                long long val = std::stoll(line, &pos, 10);
                while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) ++pos;
                if (pos != line.size() || val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) {
                    std::cout << "Invalid input. Please enter a valid integer index.\n";
                } else {
                    int idx = static_cast<int>(val);
                    auto res = getVehicle(vehicles, idx);
                    if (res.has_value()) {
                        std::cout << "Selected vehicle: " << res.value() << "\n";
                    } else {
                        std::cout << "Invalid index.\n";
                    }
                }
            } catch (...) {
                std::cout << "Invalid input. Please enter an integer index.\n";
            }
        } else {
            std::cout << "No input received.\n";
        }
    } else {
        std::cout << "RUN_INTERACTIVE not set; skipping interactive prompt.\n";
    }

    // 5 test cases
    int testIndices[5] = {0, 4, 2, -1, 10};
    for (int idx : testIndices) {
        auto res = getVehicle(vehicles, idx);
        std::cout << "Test index " << idx << ": " << (res.has_value() ? res.value() : std::string("Invalid index")) << "\n";
    }

    // Loop and print all vehicles
    printVehicles(vehicles);

    return 0;
}