#include <iostream>
#include <vector>
#include <string>

// Note: C++ does not have a standard library for creating HTTP servers/API endpoints.
// Doing so from scratch requires extensive socket programming and HTTP parsing, which
// is beyond the scope of a single-file example. Libraries like Boost.Beast,
// Pistache, or cpp-httplib are typically used for this purpose.
//
// This code simulates the core logic of an API endpoint: receiving user data
// (as a simple string), parsing it, and storing it in an in-memory database.

// A simple structure to hold user data
struct User {
    std::string name;
    std::string email;
};

// Our in-memory "database"
std::vector<User> userDatabase;

// Function to simulate handling an API request and storing data
// It performs very basic parsing on a "key:value,key:value" string
void storeUserData(const std::string& requestBody) {
    std::cout << "Processing request body: " << requestBody << std::endl;
    try {
        User newUser;
        size_t name_start = requestBody.find("name:") + 5;
        size_t name_end = requestBody.find(",");
        newUser.name = requestBody.substr(name_start, name_end - name_start);

        size_t email_start = requestBody.find("email:") + 6;
        newUser.email = requestBody.substr(email_start);

        // Trim whitespace (simple version)
        newUser.name.erase(0, newUser.name.find_first_not_of(" \t\n\r"));
        newUser.name.erase(newUser.name.find_last_not_of(" \t\n\r") + 1);
        newUser.email.erase(0, newUser.email.find_first_not_of(" \t\n\r"));
        newUser.email.erase(newUser.email.find_last_not_of(" \t\n\r") + 1);
        
        userDatabase.push_back(newUser);
        std::cout << "Successfully stored user: " << newUser.name << std::endl;
    } catch (const std::out_of_range& oor) {
        std::cerr << "Error parsing request body: " << requestBody << std::endl;
    }
}

void printDatabase() {
    std::cout << "\n--- Current Database State ---" << std::endl;
    if (userDatabase.empty()) {
        std::cout << "Database is empty." << std::endl;
    } else {
        for (size_t i = 0; i < userDatabase.size(); ++i) {
            std::cout << "User " << i + 1 << ": Name = " << userDatabase[i].name
                      << ", Email = " << userDatabase[i].email << std::endl;
        }
    }
    std::cout << "----------------------------" << std::endl;
}

int main() {
    std::cout << "Simulating API endpoint data storage." << std::endl;
    
    // --- 5 Test Cases ---
    std::cout << "\nRunning 5 test cases...\n" << std::endl;
    storeUserData("name:Alice, email:alice@example.com");
    storeUserData("name: Bob, email: bob@example.com");
    storeUserData("name: Charlie, email: charlie@example.com");
    storeUserData("name:  David , email:david@example.com");
    storeUserData("name:Eve, email: eve@example.com");

    // Print the final state of the database to verify
    printDatabase();

    return 0;
}