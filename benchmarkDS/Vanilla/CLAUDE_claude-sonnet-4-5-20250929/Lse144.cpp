
#include <iostream>
#include <string>
#include <cstring>

using namespace std;

// Simulate database connection (in real scenario, use actual database library)
bool checkEmailExists(const string& email) {
    // Mock database check
    return (email == "user1@example.com" || email == "user2@example.com");
}

int callUnsubscribeScript(const string& email) {
    // Simulate calling an unsubscribe script
    // In real scenario, this would execute an external script or perform database update
    try {
        // Simulate script execution (returns 0 for success, non-zero for failure)
        if (!email.empty() && email.find('@') != string::npos) {
            return 0; // Success
        }
        return 1; // Failure
    } catch (...) {
        return 1; // Failure
    }
}

int unsubscribeEmail(const string& email) {
    try {
        // Check if email exists in database
        bool emailExists = checkEmailExists(email);
        
        if (emailExists) {
            // Email exists, call unsubscribe script
            int scriptResult = callUnsubscribeScript(email);
            
            if (scriptResult == 0) {
                return 1; // Success
            } else {
                return 0; // Script failed
            }
        } else {
            // Email doesn't exist
            return 0;
        }
    } catch (...) {
        return 0;
    }
}

int main() {
    // Test cases
    cout << "Test Case 1: " << unsubscribeEmail("user1@example.com") << endl;
    cout << "Test Case 2: " << unsubscribeEmail("user2@example.com") << endl;
    cout << "Test Case 3: " << unsubscribeEmail("nonexistent@example.com") << endl;
    cout << "Test Case 4: " << unsubscribeEmail("invalid-email") << endl;
    cout << "Test Case 5: " << unsubscribeEmail("user5@example.com") << endl;
    
    return 0;
}
