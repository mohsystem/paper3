#include <iostream>
#include <string>

static const std::string LOGIN_PATH = "/login";
static const std::string USER_PATH  = "/user";
static const std::string ADMIN_PATH = "/admin";

std::string secureRedirect(int isLoggedIn, int isAdmin) {
    if ((isLoggedIn != 0 && isLoggedIn != 1) || (isAdmin != 0 && isAdmin != 1)) {
        return LOGIN_PATH;
    }
    if (isLoggedIn == 0) {
        return LOGIN_PATH;
    }
    if (isAdmin == 1) {
        return ADMIN_PATH;
    }
    return USER_PATH;
}

int main() {
    std::cout << secureRedirect(0, 0) << std::endl; // /login
    std::cout << secureRedirect(1, 0) << std::endl; // /user
    std::cout << secureRedirect(1, 1) << std::endl; // /admin
    std::cout << secureRedirect(0, 1) << std::endl; // /login
    std::cout << secureRedirect(5, 1) << std::endl; // /login (invalid input)
    return 0;
}