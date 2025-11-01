#include <iostream>
#include <string>

std::string getRedirect(bool isLoggedIn, bool isAdmin) {
    if (!isLoggedIn) return "login";
    if (isAdmin) return "admin";
    return "user";
}

int main() {
    std::cout << getRedirect(false, false) << std::endl; // Expected: login
    std::cout << getRedirect(false, true)  << std::endl; // Expected: login
    std::cout << getRedirect(true, false)  << std::endl; // Expected: user
    std::cout << getRedirect(true, true)   << std::endl; // Expected: admin
    std::cout << getRedirect(true, false)  << std::endl; // Expected: user
    return 0;
}