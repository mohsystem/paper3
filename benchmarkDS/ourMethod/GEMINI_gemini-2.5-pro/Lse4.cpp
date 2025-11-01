#include <iostream>
#include <vector>
#include <string>
#include <limits>

// Define the list of vehicles as a global constant vector.
const std::vector<std::string> VEHICLES = {"Car", "Bus", "Motorcycle", "Airplane", "Boat"};

/**
 * @brief Retrieves a vehicle from the vector by its index.
 * 
 * @param index The index of the vehicle to retrieve.
 * @return The vehicle name if the index is valid, otherwise an error message.
 */
std::string getVehicle(int index) {
    // Rule #1, #4: Ensure vector access is within boundaries.
    // Using .at() would throw an exception, but a manual check is also clear and safe.
    if (index >= 0 && static_cast<size_t>(index) < VEHICLES.size()) {
        return VEHICLES[static_cast<size_t>(index)];
    } else {
        return "Error: Index is out of bounds.";
    }
}

/**
 * @brief Processes a string input, converts it to an integer, and gets the vehicle.
 * 
 * @param input The string input representing the index.
 * @return The result of the getVehicle call or an error message if parsing fails.
 */
std::string processInput(const std::string& input) {
    try {
        // Rule #4: Validate input by converting to the expected type.
        // std::stoi performs conversion and throws an exception on failure.
        size_t pos;
        int index = std::stoi(input, &pos);

        // Also check if the entire string was consumed to avoid partial matches like "123xyz"
        if (pos != input.length()) {
            return "Error: Input contains non-numeric characters.";
        }
        
        return getVehicle(index);
    } catch (const std::invalid_argument&) {
        // Rule #7: Handle exceptions for invalid number formats.
        return "Error: Input is not a valid integer.";
    } catch (const std::out_of_range&) {
        // Rule #7: Handle exceptions for numbers outside the range of int.
        return "Error: Input number is out of range.";
    }
}

int main() {
    // In a real application, you would get user input:
    // std::string userInput;
    // std::cout << "Enter an index of a vehicle: ";
    // std::getline(std::cin, userInput);
    // if (std::cin.good()) {
    //     std::cout << "Vehicle: " << processInput(userInput) << std::endl;
    // }

    // For demonstration, here are 5 hardcoded test cases.
    std::vector<std::string> testInputs = {"2", "0", "4", "10", "-1", "Bus"};

    std::cout << "Running test cases..." << std::endl;
    for (size_t i = 0; i < testInputs.size(); ++i) {
        std::cout << "\nTest Case " << i + 1 << ":" << std::endl;
        std::cout << "Input: \"" << testInputs[i] << "\"" << std::endl;
        std::string result = processInput(testInputs[i]);
        std::cout << "Output: " << result << std::endl;
    }

    return 0;
}