
#include <iostream>
#include <string>
#include <map>
#include <algorithm>

std::string redirectUser(std::string userInput) {
    std::map<std::string, std::string> validPages;
    validPages["home"] = "/home";
    validPages["about"] = "/about";
    validPages["contact"] = "/contact";
    validPages["services"] = "/services";
    validPages["products"] = "/products";
    
    if (userInput.empty()) {
        return "Invalid input. Redirecting to default page: /home";
    }
    
    // Trim whitespace
    userInput.erase(0, userInput.find_first_not_of(" \\t\\n\\r"));
    userInput.erase(userInput.find_last_not_of(" \\t\\n\\r") + 1);
    
    // Convert to lowercase
    std::transform(userInput.begin(), userInput.end(), userInput.begin(), ::tolower);
    
    if (validPages.find(userInput) != validPages.end()) {
        return "Redirecting to: " + validPages[userInput];
    } else {
        return "Page not found. Redirecting to default page: /home";
    }
}

int main() {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    std::cout << redirectUser("home") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2:" << std::endl;
    std::cout << redirectUser("about") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 3:" << std::endl;
    std::cout << redirectUser("invalidpage") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4:" << std::endl;
    std::cout << redirectUser("") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5:" << std::endl;
    std::cout << redirectUser("PRODUCTS") << std::endl;
    std::cout << std::endl;
    
    return 0;
}
