#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

class Lse88 {
private:
    static std::map<std::string, double> prices;

public:
    /**
     * Parses a simple YAML string to calculate the total price of a product.
     *
     * @param yamlPayload A string containing product_code and quantity.
     * @return The total price formatted as a string, or an error message.
     */
    static std::string calculate_price(const std::string& yamlPayload) {
        std::map<std::string, std::string> data;
        std::stringstream ss(yamlPayload);
        std::string line;

        while (std::getline(ss, line)) {
            size_t colon_pos = line.find(':');
            if (colon_pos == std::string::npos) {
                continue; // Skip malformed lines
            }
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);

            // Trim leading whitespace
            key.erase(0, key.find_first_not_of(" \t\n\r"));
            value.erase(0, value.find_first_not_of(" \t\n\r"));
            // Trim trailing whitespace
            key.erase(key.find_last_not_of(" \t\n\r") + 1);
            value.erase(value.find_last_not_of(" \t\n\r") + 1);

            data[key] = value;
        }

        if (data.find("product_code") == data.end() || data.find("quantity") == data.end()) {
            return "Error: Invalid format or missing keys";
        }

        std::string productCode = data["product_code"];
        // Remove quotes if they exist
        if (!productCode.empty() && productCode.front() == '"' && productCode.back() == '"') {
            productCode = productCode.substr(1, productCode.length() - 2);
        }

        if (prices.find(productCode) == prices.end()) {
            return "Error: Product not found";
        }

        int quantity;
        try {
            quantity = std::stoi(data["quantity"]);
        } catch (const std::invalid_argument&) {
            return "Error: Invalid quantity";
        } catch (const std::out_of_range&) {
            return "Error: Invalid quantity";
        }

        double unitPrice = prices.at(productCode);
        double totalPrice = unitPrice * quantity;

        std::ostringstream resultStream;
        resultStream << std::fixed << std::setprecision(2) << totalPrice;
        return resultStream.str();
    }
    
    static void initialize_prices() {
        if (prices.empty()){
            prices["P101"] = 10.50;
            prices["P102"] = 25.00;
            prices["P103"] = 5.75;
        }
    }
};

std::map<std::string, double> Lse88::prices;

int main() {
    Lse88::initialize_prices();

    std::vector<std::string> testCases = {
        // Test Case 1: Standard valid case
        "product_code: \"P101\"\nquantity: 5",
        // Test Case 2: Another valid case (different order)
        "quantity: 10\nproduct_code: \"P103\"",
        // Test Case 3: Product not found
        "product_code: \"P999\"\nquantity: 2",
        // Test Case 4: Invalid quantity
        "product_code: \"P102\"\nquantity: abc",
        // Test Case 5: Malformed YAML (no colon on one line)
        "product_code \"P101\"\nquantity: 3"
    };

    std::cout << "CPP Test Cases:" << std::endl;
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input:\n---\n" << testCases[i] << "\n---" << std::endl;
        std::string result = Lse88::calculate_price(testCases[i]);
        std::cout << "Output: " << result << std::endl << std::endl;
    }

    return 0;
}