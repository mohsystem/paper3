#include <iostream>
#include <string>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <array>

static const std::array<const char*, 5> VEHICLES = {"Car", "Truck", "Bike", "Bus", "Van"};

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

std::string getVehicleByIndex(const std::string& indexInput) {
    std::string s = trim(indexInput);
    if (s.empty()) {
        return "Invalid input: empty";
    }
    // Validate format: optional +/- followed by digits
    size_t pos = 0;
    if (s[0] == '+' || s[0] == '-') pos = 1;
    if (pos >= s.size()) return "Invalid input: not an integer";
    for (size_t i = pos; i < s.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return "Invalid input: not an integer";
        }
    }

    errno = 0;
    char* endptr = nullptr;
    long long val = std::strtoll(s.c_str(), &endptr, 10);
    if (errno == ERANGE) {
        return "Invalid index: out of range";
    }
    if (endptr == s.c_str() || *endptr != '\0') {
        return "Invalid input: not an integer";
    }
    if (val < 0) {
        return "Invalid index: must be between 0 and " + std::to_string(VEHICLES.size() - 1);
    }
    if (val >= static_cast<long long>(VEHICLES.size())) {
        return "Invalid index: must be between 0 and " + std::to_string(VEHICLES.size() - 1);
    }
    return std::string(VEHICLES[static_cast<size_t>(val)]);
}

int main() {
    const std::string tests[5] = {"2", " 0 ", "-1", "10", "abc"};
    for (const auto& t : tests) {
        std::cout << "Input: '" << t << "' => " << getVehicleByIndex(t) << std::endl;
    }
    return 0;
}