
// WARNING: This is a simplified demonstration. Real web applications require proper frameworks.
// This code demonstrates INSECURE practices for educational purposes only
#include <iostream>
#include <string>
#include <map>

class User {
public:
    std::string username;
    std::string email;
    std::string ssn;  // INSECURE: Never store SSN like this!
    
    User(const std::string& u, const std::string& e, const std::string& s)
        : username(u), email(e), ssn(s) {}
};

class Lse41 {
private:
    // Mock database - INSECURE
    User getCurrentUser() {
        // INSECURE: Hardcoded sensitive data
        return User("john_doe", "john@example.com", "123456789");
    }
    
public:
    std::string handleInfoRequest() {
        User user = getCurrentUser();
        std::string lastFour = user.ssn.substr(user.ssn.length() - 4);
        
        std::string html = "<html><body>\\n";
        html += "<h1>User Information</h1>\\n";
        html += "<p>Username: " + user.username + "</p>\\n";
        html += "<p>Email: " + user.email + "</p>\\n";
        html += "<p>SSN (Last 4): ***-**-" + lastFour + "</p>\\n";
        html += "</body></html>";
        
        return html;
    }
    
    static void runTestCases() {
        std::cout << "Test Cases:" << std::endl;
        
        User users[] = {
            User("john_doe", "john@example.com", "123456789"),
            User("jane_smith", "jane@example.com", "987654321"),
            User("bob_jones", "bob@example.com", "555443333"),
            User("alice_wonder", "alice@example.com", "111223333"),
            User("charlie_brown", "charlie@example.com", "999887777")
        };
        
        for (int i = 0; i < 5; i++) {
            std::cout << (i + 1) << ". User: " << users[i].username << std::endl;
            std::cout << "   Email: " << users[i].email << std::endl;
            std::string lastFour = users[i].ssn.substr(users[i].ssn.length() - 4);
            std::cout << "   Last 4 SSN: " << lastFour << std::endl;
        }
    }
};

int main() {
    Lse41::runTestCases();
    return 0;
}
