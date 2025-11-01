
#include <iostream>
#include <string>
#include <memory>

// Enum to represent user roles
enum class UserRole {
    GUEST,
    USER,
    ADMIN
};

// Class to represent a user session
class UserSession {
private:
    bool isLoggedIn;
    UserRole role;
    std::string username;

public:
    UserSession(bool loggedIn, UserRole r, const std::string& user)
        : isLoggedIn(loggedIn), role(r), username(user) {}
    
    bool getIsLoggedIn() const {
        return isLoggedIn;
    }
    
    UserRole getRole() const {
        return role;
    }
    
    std::string getUsername() const {
        return username;
    }
};

/**
 * Determines the redirect page based on user session
 * @param session The user session pointer
 * @return The page to redirect to
 */
std::string determineRedirectPage(const UserSession* session) {
    // Input validation
    if (session == nullptr) {
        return "/login";
    }
    
    // Check if user is not logged in
    if (!session->getIsLoggedIn()) {
        return "/login";
    }
    
    // Check if user is admin
    if (session->getRole() == UserRole::ADMIN) {
        return "/admin";
    }
    
    // Check if user is a regular user
    if (session->getRole() == UserRole::USER) {
        return "/user";
    }
    
    // Default fallback to login
    return "/login";
}

int main() {
    // Test case 1: Not logged in
    UserSession session1(false, UserRole::GUEST, "");
    std::cout << "Test 1 - Not logged in: " << determineRedirectPage(&session1) << std::endl;
    
    // Test case 2: Logged in as regular user
    UserSession session2(true, UserRole::USER, "john_doe");
    std::cout << "Test 2 - Regular user: " << determineRedirectPage(&session2) << std::endl;
    
    // Test case 3: Logged in as admin
    UserSession session3(true, UserRole::ADMIN, "admin_user");
    std::cout << "Test 3 - Admin user: " << determineRedirectPage(&session3) << std::endl;
    
    // Test case 4: Null session
    std::cout << "Test 4 - Null session: " << determineRedirectPage(nullptr) << std::endl;
    
    // Test case 5: Logged in but with GUEST role
    UserSession session5(true, UserRole::GUEST, "guest_user");
    std::cout << "Test 5 - Logged in guest: " << determineRedirectPage(&session5) << std::endl;
    
    return 0;
}
