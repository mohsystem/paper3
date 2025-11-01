#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

static const std::vector<std::string> VEHICLES = {"Car", "Bus", "Truck", "Bike", "Scooter", "Van", "SUV"};

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

// Accepts the index input as a parameter, returns the vehicle or an error message
std::string getVehicleAtIndex(const std::string& indexInput) {
    std::string s = trim(indexInput);
    if (s.empty()) {
        return "Error: empty index input";
    }
    try {
        size_t pos = 0;
        int idx = std::stoi(s, &pos, 10);
        if (pos != s.size()) {
            return "Error: invalid index format";
        }
        if (idx < 0 || idx >= static_cast<int>(VEHICLES.size())) {
            return "Error: index out of bounds";
        }
        return VEHICLES[static_cast<size_t>(idx)];
    } catch (...) {
        return "Error: invalid index format";
    }
}

int main() {
    std::vector<std::string> tests = {"0", "2", "6", "-1", "abc"};
    for (const auto& t : tests) {
        std::cout << "Input: \"" << t << "\" -> " << getVehicleAtIndex(t) << "\n";
    }
    return 0;
}