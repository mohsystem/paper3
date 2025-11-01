#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <limits>

static const std::vector<std::string> VEHICLES = {
    "Car", "Bike", "Truck", "Bus", "Scooter",
    "Van", "Train", "Boat", "Plane", "Tractor"
};

static bool isNumericFormat(const std::string& s) {
    if (s.empty() || s.size() > 20) return false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') {
        i = 1;
        if (i >= s.size()) return false;
    }
    for (; i < s.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(s[i]);
        if (!std::isdigit(ch)) return false;
    }
    return true;
}

std::string getVehicleByIndex(const std::string& input) {
    // Trim leading/trailing whitespace
    size_t start = 0, end = input.size();
    while (start < end && std::isspace(static_cast<unsigned char>(input[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) --end;
    std::string s = input.substr(start, end - start);

    if (!isNumericFormat(s)) {
        return "Invalid input";
    }

    try {
        std::size_t pos = 0;
        long long val = std::stoll(s, &pos, 10);
        if (pos != s.size()) {
            return "Invalid input";
        }
        if (val < 0 || val >= static_cast<long long>(VEHICLES.size())) {
            return "Index out of range";
        }
        return VEHICLES[static_cast<std::size_t>(val)];
    } catch (...) {
        return "Invalid input";
    }
}

int main() {
    const std::vector<std::string> tests = {"0", "3", "10", "-1", "2"};
    for (const auto& t : tests) {
        std::cout << "Input: " << t << " -> Output: " << getVehicleByIndex(t) << "\n";
    }
    return 0;
}