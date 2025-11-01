#include <iostream>
#include <string>

// Steps 1-5: secure and simple logic without external inputs.

std::string decideRedirect(bool isLoggedIn, bool isAdmin) {
    const std::string LOGIN = "/login";
    const std::string USER = "/user";
    const std::string ADMIN = "/admin";

    if (!isLoggedIn) return LOGIN;
    if (isAdmin) return ADMIN;
    return USER;
}

int main() {
    // 5 test cases
    struct Case { bool loggedIn; bool admin; };
    Case tests[5] = {
        {false, false}, // /login
        {false, true},  // /login
        {true, false},  // /user
        {true, true},   // /admin
        {true, false}   // /user
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << "Test " << (i + 1) << " ("
                  << (tests[i].loggedIn ? "true" : "false") << ", "
                  << (tests[i].admin ? "true" : "false") << "): "
                  << decideRedirect(tests[i].loggedIn, tests[i].admin) << "\n";
    }
    return 0;
}