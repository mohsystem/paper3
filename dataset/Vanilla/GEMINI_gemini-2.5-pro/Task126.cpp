#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // For std::transform

/**
 * Redirects a user based on a page string.
 *
 * @param page The string representing the desired page (e.g., "home", "about").
 * @return A string message indicating the redirection action.
 */
std::string redirectUser(std::string page) {
    // Convert page string to lower case for case-insensitive comparison
    std::transform(page.begin(), page.end(), page.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (page == "home") {
        return "Redirecting to Home Page...";
    } else if (page == "about") {
        return "Redirecting to About Us Page...";
    } else if (page == "contact") {
        return "Redirecting to Contact Page...";
    } else if (page == "products") {
        return "Redirecting to Products Page...";
    } else {
        return "Page not found. Redirecting to 404 Error Page...";
    }
}

int main() {
    std::vector<std::string> testCases = {"home", "about", "contact", "products", "login"};

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& testCase : testCases) {
        std::string result = redirectUser(testCase);
        std::cout << "Input: \"" << testCase << "\" -> Output: " << result << std::endl;
    }
    return 0;
}