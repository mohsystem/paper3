
#include <iostream>
#include <string>
#include <map>
#include <algorithm>

std::string redirectUser(std::string userInput) {
    std::map<std::string, std::string> pageRoutes;
    pageRoutes["home"] = "https://example.com/home";
    pageRoutes["about"] = "https://example.com/about";
    pageRoutes["contact"] = "https://example.com/contact";
    pageRoutes["products"] = "https://example.com/products";
    pageRoutes["services"] = "https://example.com/services";
    
    if (userInput.empty() || userInput.find_first_not_of(' ') == std::string::npos) {
        return "Error: Invalid input";
    }
    
    // Trim and convert to lowercase
    userInput.erase(0, userInput.find_first_not_of(' '));
    userInput.erase(userInput.find_last_not_of(' ') + 1);
    std::transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower);
    
    if (pageRoutes.find(userInput) != pageRoutes.end()) {
        return "Redirecting to: " + pageRoutes[userInput];
    } else {
        return "Error: Page not found. Redirecting to: " + pageRoutes["home"];
    }
}

int main() {
    std::cout << "=== Page Redirect System - Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Valid page - home
    std::string test1 = "home";
    std::cout << "Test 1 - Input: '" << test1 << "'" << std::endl;
    std::cout << "Result: " << redirectUser(test1) << std::endl << std::endl;
    
    // Test Case 2: Valid page - about
    std::string test2 = "about";
    std::cout << "Test 2 - Input: '" << test2 << "'" << std::endl;
    std::cout << "Result: " << redirectUser(test2) << std::endl << std::endl;
    
    // Test Case 3: Valid page with mixed case
    std::string test3 = "PRODUCTS";
    std::cout << "Test 3 - Input: '" << test3 << "'" << std::endl;
    std::cout << "Result: " << redirectUser(test3) << std::endl << std::endl;
    
    // Test Case 4: Invalid page
    std::string test4 = "unknown";
    std::cout << "Test 4 - Input: '" << test4 << "'" << std::endl;
    std::cout << "Result: " << redirectUser(test4) << std::endl << std::endl;
    
    // Test Case 5: Empty input
    std::string test5 = "";
    std::cout << "Test 5 - Input: '" << test5 << "'" << std::endl;
    std::cout << "Result: " << redirectUser(test5) << std::endl << std::endl;
    
    return 0;
}
