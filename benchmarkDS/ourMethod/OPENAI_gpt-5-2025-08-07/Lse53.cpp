#include <iostream>
#include <string>

static std::string getRedirectTarget(bool isLoggedIn, bool isAdmin) {
    if (!isLoggedIn) {
        return "/login";
    }
    if (isAdmin) {
        return "/admin";
    }
    return "/user";
}

static void runTest(bool isLoggedIn, bool isAdmin) {
    const std::string result = getRedirectTarget(isLoggedIn, isAdmin);
    std::cout << "isLoggedIn=" << (isLoggedIn ? "true" : "false")
              << ", isAdmin=" << (isAdmin ? "true" : "false")
              << " -> " << result << "\n";
}

int main() {
    // 5 test cases
    runTest(false, false); // /login
    runTest(true, false);  // /user
    runTest(true, true);   // /admin
    runTest(false, true);  // /login
    runTest(true, true);   // /admin
    return 0;
}