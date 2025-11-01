#include <iostream>
#include <string>
#include <vector>

// --- Mock Database ---
// A vector to store messages in the format "username: message"
static std::vector<std::string> mockDb;

// --- Mock Request Object ---
// Represents an incoming request with user and message data.
class Request {
public:
    Request(const std::string& user, const std::string& msg)
        : username(user), message(msg) {}

    std::string getUsername() const {
        return username;
    }

    std::string getMessage() const {
        return message;
    }

private:
    std::string username;
    std::string message;
};

/**
 * Simulates inserting a user's message into a database.
 * @param username The user who sent the message.
 * @param message The content of the message.
 */
void insert_user_message_in_db(const std::string& username, const std::string& message) {
    std::cout << "DATABASE: Inserting message for user '" << username << "'." << std::endl;
    mockDb.push_back(username + ": " + message);
}

/**
 * This function simulates being called when a user submits a message from a form.
 * It gets the username and message from the request.
 * It calls the insert_user_message_in_db() function.
 * It simulates redirecting the user back to the main page.
 * @param req The mock request object containing user data.
 */
void post(const Request& req) {
    std::cout << "\n--- Handling POST request ---" << std::endl;
    // 1. Get username and message from the request.
    std::string username = req.getUsername();
    std::string message = req.getMessage();
    std::cout << "Received post from '" << username << "' with message: '" << message << "'" << std::endl;

    // 2. Insert the message into the database.
    insert_user_message_in_db(username, message);
    
    // 3. Redirect the user back to the main page.
    std::cout << "ACTION: Redirecting user to the main page..." << std::endl;
    std::cout << "--- Request Handled ---" << std::endl;
}

/**
 * The main page would contain an HTML form like this:
 * 
 * <form action="/post_message" method="post">
 *   <label for="username">Username:</label><br>
 *   <input type="text" id="username" name="username"><br>
 *   <label for="message">Message:</label><br>
 *   <textarea id="message" name="message" rows="4" cols="50"></textarea><br><br>
 *   <input type="submit" value="Submit">
 * </form> 
 */
int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1
    Request req1("Alice", "Hello World!");
    post(req1);

    // Test Case 2
    Request req2("Bob", "This is a test message.");
    post(req2);

    // Test Case 3
    Request req3("Charlie", "How is everyone?");
    post(req3);

    // Test Case 4
    Request req4("Alice", "I'm posting again.");
    post(req4);

    // Test Case 5
    Request req5("David", "Final test case.");
    post(req5);

    std::cout << "\n--- Final State of Mock Database ---" << std::endl;
    for (size_t i = 0; i < mockDb.size(); ++i) {
        std::cout << i + 1 << ". " << mockDb[i] << std::endl;
    }

    return 0;
}