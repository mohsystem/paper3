#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <limits>
#include <cerrno>
#include <cstdlib>

std::optional<std::string> get_vehicle(const std::vector<std::string>& vehicles, int index) {
    if (index >= 0 && static_cast<size_t>(index) < vehicles.size()) {
        return vehicles[static_cast<size_t>(index)];
    }
    return std::nullopt;
}

int main() {
    std::vector<std::string> vehicles { "Car", "Bike", "Truck", "Bus", "Scooter" };

    std::cout << "Available vehicles count: " << vehicles.size() << "\n";
    std::cout << "Enter an index (0 to " << (vehicles.size() - 1) << "): ";
    std::string line;
    if (!std::getline(std::cin, line)) {
        line.clear();
    }
    if (line.size() > 20) {
        std::cout << "Invalid index.\n";
    } else {
        const char* s = line.c_str();
        char* endptr = nullptr;
        errno = 0;
        long val = std::strtol(s, &endptr, 10);
        bool ok = (errno == 0) && (endptr != s) && (*endptr == '\0') &&
                  (val >= std::numeric_limits<int>::min()) && (val <= std::numeric_limits<int>::max());
        if (ok) {
            int idx = static_cast<int>(val);
            auto chosen = get_vehicle(vehicles, idx);
            if (chosen.has_value()) {
                std::cout << "Selected vehicle: " << chosen.value() << "\n";
            } else {
                std::cout << "Invalid index.\n";
            }
        } else {
            std::cout << "Invalid index.\n";
        }
    }

    std::cout << "All vehicles:\n";
    for (size_t i = 0; i < vehicles.size(); ++i) {
        std::cout << i << ": " << vehicles[i] << "\n";
    }

    std::cout << "Test cases:\n";
    std::vector<int> testIndices { 0, 2, 4, -1, 10 };
    for (int idx : testIndices) {
        auto result = get_vehicle(vehicles, idx);
        if (result.has_value()) {
            std::cout << "Index " << idx << " -> " << result.value() << "\n";
        } else {
            std::cout << "Index " << idx << " -> Invalid index.\n";
        }
    }

    return 0;
}