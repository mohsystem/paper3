
#include <iostream>
#include <string>
#include <vector>
#include <ctime>

using namespace std;

struct Message {
    string username;
    string message;
    time_t timestamp;
    
    Message(const string& uname, const string& msg) 
        : username(uname), message(msg), timestamp(time(nullptr)) {}
};

class Lse148 {
private:
    static vector<Message> messages;
    
public:
    // Function to insert user message in database
    static void insert_user_message_in_db(const string& username, const string& message) {
        messages.push_back(Message(username, message));
        cout << "Message inserted: " << username << " - " << message << endl;
    }
    
    // Post method to handle form submission
    static void post(const string& username, const string& message) {
        if (!username.empty() && !message.empty()) {
            insert_user_message_in_db(username, message);
        }
        // Redirect to main page (simulated)
        cout << "Redirecting to main page..." << endl;
    }
    
    // Get method to display messages
    static void get() {
        cout << "=== Messages ===" << endl;
        for (const auto& msg : messages) {
            cout << msg.username << ": " << msg.message << endl;
        }
        cout << "================" << endl;
    }
    
    static void displayMessages() {
        get();
    }
};

// Initialize static member
vector<Message> Lse148::messages;

int main() {
    // Test cases
    cout << "Test Case 1: Adding first message" << endl;
    Lse148::post("Alice", "Hello World!");
    
    cout << "\\nTest Case 2: Adding second message" << endl;
    Lse148::post("Bob", "Hi there!");
    
    cout << "\\nTest Case 3: Adding message with special characters" << endl;
    Lse148::post("Charlie", "Test @#$ message!");
    
    cout << "\\nTest Case 4: Adding empty username" << endl;
    Lse148::post("", "Anonymous message");
    
    cout << "\\nTest Case 5: Adding long message" << endl;
    Lse148::post("David", "This is a very long message to test the system's ability to handle lengthy text content.");
    
    cout << "\\nDisplaying all messages:" << endl;
    Lse148::displayMessages();
    
    return 0;
}
