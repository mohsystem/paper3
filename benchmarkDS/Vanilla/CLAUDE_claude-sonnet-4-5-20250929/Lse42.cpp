
#include <iostream>
#include <string>
#include <map>

class Lse42 {
private:
    std::map<std::string, std::string> session;
    
public:
    void setSession(const std::string& key, const std::string& value) {
        session[key] = value;
    }
    
    std::string getSession(const std::string& key) {
        if (session.find(key) != session.end()) {
            return session[key];
        }
        return "";
    }
    
    std::string info() {
        std::string username = getSession("username");
        std::string email = getSession("email");
        std::string ssn = getSession("ssn");
        
        return "Username: " + username + ", Email: " + email + ", SSN: " + ssn;
    }
};

int main() {
    Lse42 app;
    
    // Test Case 1
    app.setSession("username", "john_doe");
    app.setSession("email", "john@example.com");
    app.setSession("ssn", "123-45-6789");
    std::cout << "Test Case 1: " << app.info() << std::endl;
    
    // Test Case 2
    app.setSession("username", "jane_smith");
    app.setSession("email", "jane@example.com");
    app.setSession("ssn", "987-65-4321");
    std::cout << "Test Case 2: " << app.info() << std::endl;
    
    // Test Case 3
    app.setSession("username", "bob_jones");
    app.setSession("email", "bob@example.com");
    app.setSession("ssn", "111-22-3333");
    std::cout << "Test Case 3: " << app.info() << std::endl;
    
    // Test Case 4
    app.setSession("username", "alice_wonder");
    app.setSession("email", "alice@example.com");
    app.setSession("ssn", "444-55-6666");
    std::cout << "Test Case 4: " << app.info() << std::endl;
    
    // Test Case 5
    app.setSession("username", "charlie_brown");
    app.setSession("email", "charlie@example.com");
    app.setSession("ssn", "777-88-9999");
    std::cout << "Test Case 5: " << app.info() << std::endl;
    
    return 0;
}
