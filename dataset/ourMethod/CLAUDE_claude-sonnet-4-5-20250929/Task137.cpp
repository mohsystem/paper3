
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <limits>
#include <cstring>

// Security: Immutable product structure to prevent tampering
struct Product {
    int id;
    std::string name;
    std::string category;
    double price;
    int stock;
};

// Security: Static product database - in production, use parameterized queries
class ProductDatabase {
private:
    std::vector<Product> products;
    
public:
    ProductDatabase() {
        // Initialize with sample data
        products = {
            {1, "Laptop", "Electronics", 999.99, 10},
            {2, "Mouse", "Electronics", 29.99, 50},
            {3, "Keyboard", "Electronics", 79.99, 30},
            {4, "Monitor", "Electronics", 299.99, 15},
            {5, "Desk", "Furniture", 399.99, 8}
        };
    }
    
    // Security: Validate ID range to prevent injection/overflow
    std::vector<Product> queryById(int id) {
        std::vector<Product> results;
        // Input validation: ID must be positive and within reasonable bounds
        if (id <= 0 || id > 1000000) {
            return results; // Return empty for invalid ID
        }
        
        for (const auto& product : products) {
            if (product.id == id) {
                results.push_back(product);
                break; // ID should be unique
            }
        }
        return results;
    }
    
    // Security: Sanitize name input to prevent injection attacks
    std::vector<Product> queryByName(const std::string& name) {
        std::vector<Product> results;
        // Input validation: Check length to prevent DoS via excessive string
        if (name.empty() || name.length() > 100) {
            return results; // Return empty for invalid name
        }
        
        // Security: Convert to lowercase for case-insensitive comparison
        std::string nameLower = name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        
        for (const auto& product : products) {
            std::string productNameLower = product.name;
            std::transform(productNameLower.begin(), productNameLower.end(), 
                          productNameLower.begin(),
                          [](unsigned char c) { return std::tolower(c); });
            
            // Security: Exact match only to prevent partial injection
            if (productNameLower == nameLower) {
                results.push_back(product);
            }
        }
        return results;
    }
    
    // Security: Validate category input
    std::vector<Product> queryByCategory(const std::string& category) {
        std::vector<Product> results;
        // Input validation: Check length
        if (category.empty() || category.length() > 50) {
            return results;
        }
        
        std::string catLower = category;
        std::transform(catLower.begin(), catLower.end(), catLower.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        
        for (const auto& product : products) {
            std::string productCatLower = product.category;
            std::transform(productCatLower.begin(), productCatLower.end(),
                          productCatLower.begin(),
                          [](unsigned char c) { return std::tolower(c); });
            
            if (productCatLower == catLower) {
                results.push_back(product);
            }
        }
        return results;
    }
};

// Security: Sanitize user input to prevent injection attacks
std::string sanitizeInput(const std::string& input) {
    std::string sanitized;
    // Limit length to prevent DoS
    size_t maxLen = 100;
    size_t len = std::min(input.length(), maxLen);
    
    for (size_t i = 0; i < len; ++i) {
        char c = input[i];
        // Allow only alphanumeric, space, and basic punctuation
        if (std::isalnum(static_cast<unsigned char>(c)) || 
            c == ' ' || c == '-' || c == '_') {
            sanitized += c;
        }
    }
    return sanitized;
}

// Security: Safe integer parsing with overflow checks
bool parseInteger(const std::string& str, int& result) {
    if (str.empty() || str.length() > 10) {
        return false; // Prevent overflow
    }
    
    // Check for valid integer format
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') {
        start = 1;
    }
    
    for (size_t i = start; i < str.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
            return false;
        }
    }
    
    try {
        long long val = std::stoll(str);
        // Security: Check for integer overflow
        if (val < std::numeric_limits<int>::min() || 
            val > std::numeric_limits<int>::max()) {
            return false;
        }
        result = static_cast<int>(val);
        return true;
    } catch (...) {
        return false;
    }
}

void displayProducts(const std::vector<Product>& products) {
    if (products.empty()) {
        std::cout << "No products found.\\n";
        return;
    }
    
    std::cout << "\\n=== Product Details ===\\n";
    for (const auto& p : products) {
        std::cout << "ID: " << p.id << "\\n";
        std::cout << "Name: " << p.name << "\\n";
        std::cout << "Category: " << p.category << "\\n";
        std::cout << "Price: $" << p.price << "\\n";
        std::cout << "Stock: " << p.stock << "\\n";
        std::cout << "----------------------\\n";
    }
}

std::string processQuery(ProductDatabase& db, const std::string& queryType, 
                        const std::string& queryValue) {
    // Security: Sanitize all inputs
    std::string sanitizedType = sanitizeInput(queryType);
    std::string sanitizedValue = sanitizeInput(queryValue);
    
    // Convert query type to lowercase for comparison
    std::transform(sanitizedType.begin(), sanitizedType.end(), sanitizedType.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    
    std::vector<Product> results;
    
    if (sanitizedType == "id") {
        int id;
        if (parseInteger(sanitizedValue, id)) {
            results = db.queryById(id);
        } else {
            return "Invalid ID format";
        }
    } else if (sanitizedType == "name") {
        results = db.queryByName(sanitizedValue);
    } else if (sanitizedType == "category") {
        results = db.queryByCategory(sanitizedValue);
    } else {
        return "Invalid query type. Use: id, name, or category";
    }
    
    if (results.empty()) {
        return "No products found";
    }
    
    return "Success";
}

int main() {
    ProductDatabase db;
    
    // Test case 1: Query by valid ID
    std::cout << "Test 1: Query by ID (1)\\n";
    std::vector<Product> result1 = db.queryById(1);
    displayProducts(result1);
    
    // Test case 2: Query by name
    std::cout << "\\nTest 2: Query by name (Mouse)\\n";
    std::vector<Product> result2 = db.queryByName("Mouse");
    displayProducts(result2);
    
    // Test case 3: Query by category
    std::cout << "\\nTest 3: Query by category (Electronics)\\n";
    std::vector<Product> result3 = db.queryByCategory("Electronics");
    displayProducts(result3);
    
    // Test case 4: Invalid ID (negative)
    std::cout << "\\nTest 4: Query by invalid ID (-1)\\n";
    std::vector<Product> result4 = db.queryById(-1);
    displayProducts(result4);
    
    // Test case 5: Non-existent product
    std::cout << "\\nTest 5: Query by non-existent name (Tablet)\\n";
    std::vector<Product> result5 = db.queryByName("Tablet");
    displayProducts(result5);
    
    return 0;
}
