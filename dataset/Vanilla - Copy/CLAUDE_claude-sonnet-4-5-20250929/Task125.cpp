
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

class Task125 {
private:
    std::map<std::string, std::vector<std::string>> userResources;
    std::map<std::string, std::string> userCredentials;
    
    void initializeData() {
        // Initialize user credentials
        userCredentials["admin"] = "admin123";
        userCredentials["user1"] = "pass1";
        userCredentials["user2"] = "pass2";
        userCredentials["guest"] = "guest123";
        userCredentials["manager"] = "manager456";
        
        // Initialize user resources
        userResources["admin"] = {"Dashboard", "Users", "Settings", "Reports", "Database"};
        userResources["user1"] = {"Dashboard", "Profile", "Documents"};
        userResources["user2"] = {"Dashboard", "Profile", "Messages"};
        userResources["guest"] = {"Dashboard", "Help"};
        userResources["manager"] = {"Dashboard", "Reports", "Team", "Projects"};
    }
    
public:
    Task125() {
        initializeData();
    }
    
    bool authenticate(const std::string& username, const std::string& password) {
        auto it = userCredentials.find(username);
        return it != userCredentials.end() && it->second == password;
    }
    
    std::vector<std::string> getAccessibleResources(const std::string& username) {
        auto it = userResources.find(username);
        if (it != userResources.end()) {
            return it->second;
        }
        return std::vector<std::string>();
    }
    
    bool canAccessResource(const std::string& username, const std::string& resource) {
        std::vector<std::string> resources = getAccessibleResources(username);
        return std::find(resources.begin(), resources.end(), resource) != resources.end();
    }
    
    std::string accessResource(const std::string& username, const std::string& password, 
                               const std::string& resource) {
        if (!authenticate(username, password)) {
            return "Authentication failed: Invalid username or password";
        }
        
        if (canAccessResource(username, resource)) {
            return "Access granted: " + username + " can access " + resource;
        } else {
            return "Access denied: " + username + " cannot access " + resource;
        }
    }
};

int main() {
    Task125 system;
    
    std::cout << "Test Case 1: Admin accessing Dashboard" << std::endl;
    std::cout << system.accessResource("admin", "admin123", "Dashboard") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2: User1 accessing Documents" << std::endl;
    std::cout << system.accessResource("user1", "pass1", "Documents") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 3: Guest trying to access Database" << std::endl;
    std::cout << system.accessResource("guest", "guest123", "Database") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4: Invalid credentials" << std::endl;
    std::cout << system.accessResource("user1", "wrongpass", "Dashboard") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5: Manager accessing Reports" << std::endl;
    std::cout << system.accessResource("manager", "manager456", "Reports") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Bonus: Listing all accessible resources for user1" << std::endl;
    std::vector<std::string> resources = system.getAccessibleResources("user1");
    std::cout << "user1 can access: ";
    for (size_t i = 0; i < resources.size(); i++) {
        std::cout << resources[i];
        if (i < resources.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    
    return 0;
}
