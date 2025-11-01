
// C++ web applications typically require frameworks like Crow, Beast, or Pistache
// Below is a conceptual implementation using Crow framework

/*
#include "crow_all.h"
#include <string>
#include <unordered_map>

class Lse40 {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sessions;
    
public:
    std::string getLastFourDigits(const std::string& ssn) {
        if (ssn.length() >= 4) {
            return ssn.substr(ssn.length() - 4);
        }
        return "N/A";
    }
    
    std::string info(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        
        std::string username = "N/A";
        std::string email = "N/A";
        std::string last4SSN = "N/A";
        
        if (it != sessions.end()) {
            auto& session = it->second;
            if (session.count("username")) username = session["username"];
            if (session.count("email")) email = session["email"];
            if (session.count("ssn")) last4SSN = getLastFourDigits(session["ssn"]);
        }
        
        std::string html = "<html><body>";
        html += "<h1>User Information</h1>";
        html += "<p>Username: " + username + "</p>";
        html += "<p>Email: " + email + "</p>";
        html += "<p>Last 4 digits of SSN: " + last4SSN + "</p>";
        html += "</body></html>";
        
        return html;
    }
    
    void setSession(const std::string& sessionId, const std::string& username, 
                    const std::string& email, const std::string& ssn) {
        sessions[sessionId]["username"] = username;
        sessions[sessionId]["email"] = email;
        sessions[sessionId]["ssn"] = ssn;
    }
};

int main() {
    crow::SimpleApp app;
    Lse40 webApp;
    
    CROW_ROUTE(app, "/info")
    ([&webApp](const crow::request& req){
        std::string sessionId = "default_session";
        return webApp.info(sessionId);
    });
    
    // Test cases
    webApp.setSession("session1", "john_doe", "john@example.com", "123456789");
    webApp.setSession("session2", "jane_smith", "jane@example.com", "987654321");
    webApp.setSession("session3", "bob_jones", "bob@example.com", "555443333");
    webApp.setSession("session4", "alice_wilson", "alice@example.com", "111223333");
    webApp.setSession("session5", "charlie_brown", "charlie@example.com", "999887777");
    
    std::cout << "Test outputs:" << std::endl;
    std::cout << webApp.info("session1") << std::endl;
    std::cout << webApp.info("session2") << std::endl;
    
    // app.port(18080).multithreaded().run();
    
    return 0;
}
*/

// Note: This requires Crow framework. Compile with: g++ -std=c++14 -I/path/to/crow/include lse40.cpp -lpthread
