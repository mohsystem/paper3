#include <iostream>
#include <string>
#include <vector>
#include <sstream>

class Lse4 {
public:
    static std::string getVehicle(const std::string& input) {
        static const std::vector<std::string> vehicles = {"Car", "Truck", "Bike", "Bus", "Van"};
        try {
            size_t pos = 0;
            int idx = std::stoi(input, &pos);
            // Ensure entire string was a number (ignoring trailing whitespace)
            for (; pos < input.size(); ++pos) {
                if (!isspace(static_cast<unsigned char>(input[pos]))) {
                    return "Invalid integer";
                }
            }
            if (idx >= 0 && idx < static_cast<int>(vehicles.size())) {
                return vehicles[idx];
            } else {
                return "Invalid index";
            }
        } catch (...) {
            return "Invalid integer";
        }
    }
};

int main() {
    std::string userInput;
    std::cout << "Enter index of vehicle: ";
    std::getline(std::cin, userInput);
    std::cout << Lse4::getVehicle(userInput) << std::endl;

    // 5 test cases
    std::vector<std::string> tests = {"0", "1", "2", "3", "4"};
    for (const auto& t : tests) {
        std::cout << "Input: " << t << " -> " << Lse4::getVehicle(t) << std::endl;
    }
    return 0;
}