#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

/**
 * Returns the price for a given product code.
 * @param product_code The code of the product.
 * @return The price of the product, or 0.0 if not found.
 */
double get_price_for_product_code(const std::string& product_code) {
    static const std::map<std::string, double> price_map = {
        {"P101", 10.50},
        {"P202", 25.00},
        {"P303", 5.75}
    };
    
    auto it = price_map.find(product_code);
    if (it != price_map.end()) {
        return it->second;
    }
    return 0.0;
}

/**
 * A very simple parser for a specific YAML format ("key: value").
 * @param yaml_payload The YAML string to parse.
 * @return A map of key-value pairs.
 */
std::map<std::string, std::string> parse_simple_yaml(const std::string& yaml_payload) {
    std::map<std::string, std::string> data;
    std::stringstream ss(yaml_payload);
    std::string line;
    while (std::getline(ss, line)) {
        std::string key, value;
        size_t delimiter_pos = line.find(':');
        if (delimiter_pos != std::string::npos) {
            key = line.substr(0, delimiter_pos);
            value = line.substr(delimiter_pos + 1);
            
            // Trim leading whitespace from value
            size_t first = value.find_first_not_of(" \t");
            if (std::string::npos != first) {
                value = value.substr(first);
            }
            data[key] = value;
        }
    }
    return data;
}

/**
 * Simulates a web route that calculates the total price from a YAML payload.
 * @param yaml_payload A string in a simple YAML format containing product_code and quantity.
 * @return A YAML string containing the total_price.
 */
std::string calculate_price(const std::string& yaml_payload) {
    // Load YAML into a dictionary (map)
    std::map<std::string, std::string> data = parse_simple_yaml(yaml_payload);
    
    // Extract product code and quantity
    std::string product_code;
    if (data.count("product_code")) {
        product_code = data["product_code"];
    }
    
    int quantity = 0;
    if (data.count("quantity")) {
        try {
            quantity = std::stoi(data["quantity"]);
        } catch (const std::exception& e) {
            quantity = 0; // Default to 0 on conversion error
        }
    }
    
    // Calculate the total price
    double price_per_unit = get_price_for_product_code(product_code);
    double total_price = price_per_unit * quantity;
    
    // Format the result as a YAML payload
    std::stringstream result_ss;
    result_ss << "total_price: " << std::fixed << std::setprecision(2) << total_price << "\n";
    
    return result_ss.str();
}

int main() {
    std::vector<std::string> test_cases = {
        "product_code: P101\nquantity: 5",    // Expected: 52.50
        "product_code: P202\nquantity: 1",    // Expected: 25.00
        "product_code: P303\nquantity: 100",  // Expected: 575.00
        "product_code: P999\nquantity: 10",   // Expected: 0.00 (product not found)
        "product_code: P101\nquantity: 0"     // Expected: 0.00
    };
    
    for (size_t i = 0; i < test_cases.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input YAML:\n---\n" << test_cases[i] << "\n---" << std::endl;
        std::string output_yaml = calculate_price(test_cases[i]);
        std::cout << "Output YAML:\n" << output_yaml << std::endl;
    }
    
    return 0;
}