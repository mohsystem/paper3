
#include <iostream>
#include <string>
#include <map>

class Lse53 {
public:
    static std::string redirectUser(bool isLoggedIn, bool isAdmin) {
        if (!isLoggedIn) {
            return "login.html";
        } else if (isAdmin) {
            return "admin.html";
        } else {
            return "user.html";
        }
    }
    
    static std::string handleRedirect(std::map<std::string, bool>& session) {
        bool isLoggedIn = false;
        bool isAdmin = false;
        
        if (session.find("isLoggedIn") != session.end()) {
            isLoggedIn = session["isLoggedIn"];
        }
        
        if (session.find("isAdmin") != session.end()) {
            isAdmin = session["isAdmin"];
        }
        
        return redirectUser(isLoggedIn, isAdmin);
    }
};

int main() {
    // Test case 1: User not logged in
    std::cout << "Test 1 - Not logged in: " << Lse53::redirectUser(false, false) << std::endl;
    
    // Test case 2: User logged in but not admin
    std::cout << "Test 2 - Logged in, not admin: " << Lse53::redirectUser(true, false) << std::endl;
    
    // Test case 3: User logged in and is admin
    std::cout << "Test 3 - Logged in, is admin: " << Lse53::redirectUser(true, true) << std::endl;
    
    // Test case 4: Not logged in but somehow admin flag is true
    std::cout << "Test 4 - Not logged in, admin flag true: " << Lse53::redirectUser(false, true) << std::endl;
    
    // Test case 5: Regular user scenario with session
    std::map<std::string, bool> session;
    session["isLoggedIn"] = true;
    session["isAdmin"] = false;
    std::cout << "Test 5 - Regular user with session: " << Lse53::handleRedirect(session) << std::endl;
    
    return 0;
}
