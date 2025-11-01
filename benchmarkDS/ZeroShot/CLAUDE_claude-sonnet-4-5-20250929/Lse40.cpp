
#include <iostream>
#include <string>
#include <map>
#include <memory>

class Session {
private:
    std::map<std::string, std::string> data;
    
public:
    void setAttribute(const std::string& key, const std::string& value) {
        data[key] = value;
    }
    
    std::string getAttribute(const std::string& key) const {
        auto it = data.find(key);
        if (it != data.end()) {
            return it->second;
        }
        return "";
    }
    
    bool hasAttribute(const std::string& key) const {
        return data.find(key) != data.end();
    }
};

class Lse40 {
private:
    std::shared_ptr<Session> session;
    
public:
    Lse40(std::shared_ptr<Session> sess) : session(sess) {}
    
    std::string getLast4Digits(const std::string& ssn) {
        if (ssn.length() >= 4) {
            return ssn.substr(ssn.length() - 4);
        }
        return "N/A";
    }
    
    std::string info() {
        std::string username = session->getAttribute("username");
        std::string email = session->getAttribute("email");
        std::string ssn = session->getAttribute("ssn");
        
        if (username.empty()) username = "N/A";
        if (email.empty()) email = "N/A";
        
        std::string last4SSN = getLast4Digits(ssn);
        
        std::string html = "<html><body>\\n";
        html += "<h1>User Information</h1>\\n";
        html += "<p>Username: " + username + "</p>\\n";
        html += "<p>Email: " + email + "</p>\\n";
        html += "<p>Last 4 digits of SSN: " + last4SSN + "</p>\\n";
        html += "</body></html>";
        
        return html;
    }
};

int main() {
    // Test Case 1
    std::cout << "Test Case 1:" << std::endl;
    auto session1 = std::make_shared<Session>();
    session1->setAttribute("username", "john_doe");
    session1->setAttribute("email", "john@example.com");
    session1->setAttribute("ssn", "123456789");
    Lse40 app1(session1);
    std::cout << app1.info() << std::endl << std::endl;
    
    // Test Case 2
    std::cout << "Test Case 2:" << std::endl;
    auto session2 = std::make_shared<Session>();
    session2->setAttribute("username", "jane_smith");
    session2->setAttribute("email", "jane@example.com");
    session2->setAttribute("ssn", "987654321");
    Lse40 app2(session2);
    std::cout << app2.info() << std::endl << std::endl;
    
    // Test Case 3
    std::cout << "Test Case 3:" << std::endl;
    auto session3 = std::make_shared<Session>();
    session3->setAttribute("username", "bob_wilson");
    session3->setAttribute("email", "bob@example.com");
    session3->setAttribute("ssn", "555443333");
    Lse40 app3(session3);
    std::cout << app3.info() << std::endl << std::endl;
    
    // Test Case 4 (Empty session)
    std::cout << "Test Case 4 (Empty session):" << std::endl;
    auto session4 = std::make_shared<Session>();
    Lse40 app4(session4);
    std::cout << app4.info() << std::endl << std::endl;
    
    // Test Case 5 (Short SSN)
    std::cout << "Test Case 5 (Short SSN):" << std::endl;
    auto session5 = std::make_shared<Session>();
    session5->setAttribute("username", "alice");
    session5->setAttribute("email", "alice@test.com");
    session5->setAttribute("ssn", "12");
    Lse40 app5(session5);
    std::cout << app5.info() << std::endl;
    
    return 0;
}
