#include <iostream>
#include <vector>
#include <string>
#include <sstream>

// Struct to hold message data
struct Message {
    std::string username;
    std::string message;
};

class Lse149 {
private:
    // In-memory vector to simulate a database
    std::vector<Message> database;

public:
    /**
     * Escapes HTML special characters in a string to prevent Cross-Site Scripting (XSS).
     */
    std::string escapeHtml(const std::string& input) {
        std::stringstream escaped;
        for (char c : input) {
            switch (c) {
                case '&':  escaped << "&amp;";   break;
                case '\"': escaped << "&quot;";  break;
                case '\'': escaped << "&#x27;";  break;
                case '<':  escaped << "&lt;";    break;
                case '>':  escaped << "&gt;";    break;
                default:   escaped << c;         break;
            }
        }
        return escaped.str();
    }

    /**
     * Simulates inserting a user message into the database.
     * In a real application, this function must use a prepared statement to prevent SQL injection.
     */
    void insert_user_message_in_db(const std::string& username, const std::string& message) {
        // SQL INJECTION PREVENTION NOTE:
        // If using a real database, use a library that supports prepared statements 
        // (e.g., libpqxx for PostgreSQL, mysql-connector-c++ for MySQL).
        // Example with libpqxx:
        // pqxx::work W(C);
        // W.exec_params("INSERT INTO messages (username, message) VALUES ($1, $2)", username, message);
        // W.commit();
        
        // Basic validation: do not insert empty strings.
        if (!username.empty() && !message.empty()) {
            database.push_back({username, message});
        }
    }

    /**
     * Simulates fetching all messages and rendering them as an HTML string.
     * Calls escapeHtml on all user-provided data to prevent XSS.
     */
    std::string display_messages() {
        if (database.empty()) {
            return "<h1>Messages</h1>\n<ul>\n<li>No messages yet.</li>\n</ul>";
        }
        std::stringstream html;
        html << "<h1>Messages</h1>\n<ul>\n";
        for (const auto& msg : database) {
            std::string safeUsername = escapeHtml(msg.username);
            std::string safeMessage = escapeHtml(msg.message);
            html << "<li><b>" << safeUsername << ":</b> " << safeMessage << "</li>\n";
        }
        html << "</ul>";
        return html.str();
    }

    /**
     * Simulates a request handler for the "/post" route.
     */
    void routePost(const std::string& username, const std::string& message) {
        std::cout << "--- Handling POST /post ---" << std::endl;
        std::cout << "Received: username=" << username << ", message=" << message << std::endl;
        insert_user_message_in_db(username, message);
        std::cout << "-> Inserted data into DB." << std::endl;
        std::cout << "-> Redirecting to /" << std::endl;
        std::cout << "-------------------------" << std::endl;
    }

    /**
     * Simulates a request handler for the "/" route.
     */
    void routeGet() {
        std::cout << "--- Handling GET / ---" << std::endl;
        std::cout << "-> Displaying messages:" << std::endl;
        std::cout << display_messages() << std::endl;
        std::cout << "----------------------" << std::endl;
    }
};

int main() {
    Lse149 app;

    // --- 5 Test Cases ---

    std::cout << ">>> Test Case 1: Post a regular message" << std::endl;
    app.routePost("Alice", "Hello World!");
    app.routeGet();

    std::cout << "\n>>> Test Case 2: Post another message" << std::endl;
    app.routePost("Bob", "This is a test.");
    app.routeGet();

    std::cout << "\n>>> Test Case 3: Post a message with HTML characters (XSS attempt)" << std::endl;
    app.routePost("Eve", "<script>alert('xss')</script>");
    app.routeGet(); // Output should show escaped HTML: &lt;script&gt;...

    std::cout << "\n>>> Test Case 4: Post a message with other special characters" << std::endl;
    app.routePost("Charlie", "Me & You > Them");
    app.routeGet(); // Output should show escaped characters: Me &amp; You &gt; Them

    std::cout << "\n>>> Test Case 5: Post empty data (should be handled gracefully)" << std::endl;
    app.routePost("", "An empty username");
    app.routePost("David", "");
    std::cout << "-> View after invalid posts (should be unchanged):" << std::endl;
    app.routeGet();

    return 0;
}