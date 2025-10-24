#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // For std::transform
#include <cctype>    // For std::tolower

/**
 * @brief Redirects a user based on a string choice.
 * The comparison is case-insensitive.
 * 
 * @param choice The user's page selection (e.g., "home", "about").
 * @return A string indicating the redirection URL or an error message.
 */
std::string redirectUser(const std::string& choice) {
    if (choice.empty()) {
        return "Error: Page not found (404)";
    }
    
    std::string lower_choice = choice;
    // Convert the string to lowercase for case-insensitive comparison
    std::transform(lower_choice.begin(), lower_choice.end(), lower_choice.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (lower_choice == "home") {
        return "Redirecting to /home.html";
    } else if (lower_choice == "about") {
        return "Redirecting to /about.html";
    } else if (lower_choice == "contact") {
        return "Redirecting to /contact.html";
    } else if (lower_choice == "products") {
        return "Redirecting to /products.html";
    } else {
        return "Error: Page not found (404)";
    }
}

int main() {
    std::vector<std::string> testCases = {"home", "About", "products", "login", ""};
    
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& test : testCases) {
        std::cout << "Input: \"" << test << "\" -> Output: " << redirectUser(test) << std::endl;
    }
    
    return 0;
}