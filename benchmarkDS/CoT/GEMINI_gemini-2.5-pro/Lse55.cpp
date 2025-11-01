#include <iostream>
#include <string>
#include <map>
#include <stdexcept>

// Forward declaration of the main class
class Lse55;

// Helper struct to represent a user
struct User {
    std::string password;
    std::string role;
};

// Main class to simulate the web application logic
class Lse55 {
private:
    // --- In-memory data stores for simulation ---

    // A simple in-memory map to store users and their details.
    // In a real application, passwords must be securely hashed and stored in a database.
    std::map<std::string, User> userDatabase;

    // A simple in-memory map to simulate user sessions.
    // Maps a session ID (integer for simplicity) to a username.
    std::map<int, std::string> sessionDatabase;
    int nextSessionId = 1000; // Simple session ID generator

public:
    // Constructor to populate the user database
    Lse55() {
        // NOTE: Storing plain text passwords is a major security vulnerability.
        // Use a strong hashing library like Argon2, scrypt, or bcrypt in a real application.
        userDatabase["user1"] = {"pass123", "user"};
        userDatabase["admin"] = {"admin123", "admin"};
    }

    // --- Simulated Web Request Handlers ---

    /**
     * Function 1: login (GET request)
     * Simulates a GET request to the login page.
     * @return A string representing the HTML content of the login page.
     */
    std::string login() {
        std::cout << "GET /login -> Displaying login form." << std::endl;
        return "<html><body><form action='/do_login' method='post'>...</form></body></html>";
    }

    /**
     * Function 2: do_login (POST request)
     * Simulates a POST request to process login credentials.
     * @param username The username from the form.
     * @param password The password from the form.
     * @return A session ID on success, or -1 on failure.
     */
    int do_login(const std::string& username, const std::string& password) {
        std::cout << "POST /do_login -> Attempting to log in user: " << username << std::endl;
        // Basic input validation
        if (username.empty() || password.empty()) {
            std::cout << "Login failed: Username or password is empty." << std::endl;
            return -1;
        }

        auto it = userDatabase.find(username);
        // In a real application, compare a hash of the provided password with the stored hash.
        if (it != userDatabase.end() && it->second.password == password) {
            int sessionId = nextSessionId++;
            sessionDatabase[sessionId] = username;
            std::cout << "Login successful for user '" << username << "'. Session ID: " << sessionId << std::endl;
            return sessionId;
        } else {
            std::cout << "Login failed: Invalid credentials for user '" << username << "'." << std::endl;
            return -1;
        }
    }

    /**
     * Function 3: index (GET request)
     * Simulates a GET request to the public index page.
     * @return A string representing the content of the index page.
     */
    std::string index() {
        std::cout << "GET / -> Displaying public index page." << std::endl;
        return "Welcome to the public index page!";
    }

    /**
     * Function 4: user_page (GET request)
     * Simulates a GET request to a page for authenticated users.
     * @param sessionId The session ID of the user.
     * @return The user page content on success, or an error message.
     */
    std::string user_page(int sessionId) {
        std::cout << "GET /user -> Access attempt with session: " << sessionId << std::endl;
        auto it = sessionDatabase.find(sessionId);
        if (it != sessionDatabase.end()) {
            std::string username = it->second;
            return "Welcome to the user page, " + username + "!";
        } else {
            return "Unauthorized: Please log in to see this page.";
        }
    }

    /**
     * Function 5: admin_page (GET request)
     * Simulates a GET request to a page for admin users.
     * @param sessionId The session ID of the user.
     * @return The admin page content on success, or an error message.
     */
    std::string admin_page(int sessionId) {
        std::cout << "GET /admin -> Access attempt with session: " << sessionId << std::endl;
        auto session_it = sessionDatabase.find(sessionId);
        if (session_it != sessionDatabase.end()) {
            std::string username = session_it->second;
            auto user_it = userDatabase.find(username);
            if (user_it != userDatabase.end() && user_it->second.role == "admin") {
                return "Welcome to the ADMIN page, " + username + "!";
            }
        }
        return "Forbidden: You do not have admin privileges to see this page.";
    }
};

// --- Main method with test cases ---
int main() {
    Lse55 webApp;

    std::cout << "--- Test Case 1: Accessing a protected page without logging in ---" << std::endl;
    std::cout << "Response: " << webApp.user_page(-1) << std::endl; // Use -1 for invalid session
    std::cout << std::endl;

    std::cout << "--- Test Case 2: A failed login attempt with wrong password ---" << std::endl;
    int failedSessionId = webApp.do_login("user1", "wrongpassword");
    std::cout << "Session ID returned: " << failedSessionId << std::endl;
    std::cout << "Accessing user page with failed session: " << webApp.user_page(failedSessionId) << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 3: A successful user login and accessing the user page ---" << std::endl;
    int userSessionId = webApp.do_login("user1", "pass123");
    std::cout << "Session ID returned: " << userSessionId << std::endl;
    std::cout << "Response: " << webApp.user_page(userSessionId) << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 4: A logged-in user trying to access the admin page (should fail) ---" << std::endl;
    std::cout << "Response: " << webApp.admin_page(userSessionId) << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 5: A successful admin login and accessing the admin page ---" << std::endl;
    int adminSessionId = webApp.do_login("admin", "admin123");
    std::cout << "Session ID returned: " << adminSessionId << std::endl;
    std::cout << "Accessing user page: " << webApp.user_page(adminSessionId) << std::endl;
    std::cout << "Accessing admin page: " << webApp.admin_page(adminSessionId) << std::endl;
    std::cout << std::endl;

    return 0;
}