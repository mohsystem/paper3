
#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <sstream>

using namespace std;

class Lse55 {
private:
    struct User {
        string password;
        string role;
    };
    
    map<string, User> users;
    map<string, string> sessions;
    
public:
    Lse55() {
        users["admin"] = {"admin123", "admin"};
        users["user1"] = {"pass123", "user"};
        users["user2"] = {"pass456", "user"};
    }
    
    // GET: Login page
    string login() {
        return "<html><body><h2>Login Page</h2>"
               "<form method='POST' action='/do_login'>"
               "Username: <input type='text' name='username'><br>"
               "Password: <input type='password' name='password'><br>"
               "<input type='submit' value='Login'>"
               "</form></body></html>";
    }
    
    // POST: Process login
    string do_login(const string& username, const string& password, string& sessionId) {
        if (users.find(username) != users.end()) {
            if (users[username].password == password) {
                stringstream ss;
                ss << time(0) << username;
                sessionId = ss.str();
                sessions[sessionId] = username;
                return "Login successful! Session ID: " + sessionId;
            }
        }
        return "Login failed! Invalid credentials.";
    }
    
    // GET: Index/Home page
    string index(const string& sessionId) {
        if (sessions.find(sessionId) != sessions.end()) {
            string username = sessions[sessionId];
            return "<html><body><h2>Welcome to the Index Page</h2>"
                   "<p>Hello, " + username + "!</p>"
                   "<a href='/user_page'>User Page</a><br>"
                   "<a href='/admin_page'>Admin Page</a>"
                   "</body></html>";
        }
        return "<html><body><h2>Index Page</h2>"
               "<p>Please <a href='/login'>login</a> to continue.</p>"
               "</body></html>";
    }
    
    // GET: User page
    string user_page(const string& sessionId) {
        if (sessions.find(sessionId) == sessions.end()) {
            return "<html><body><h2>Access Denied</h2>"
                   "<p>Please <a href='/login'>login</a> first.</p>"
                   "</body></html>";
        }
        
        string username = sessions[sessionId];
        return "<html><body><h2>User Page</h2>"
               "<p>Welcome, " + username + "!</p>"
               "<p>This is your user dashboard.</p>"
               "<a href='/index'>Back to Index</a>"
               "</body></html>";
    }
    
    // GET: Admin page
    string admin_page(const string& sessionId) {
        if (sessions.find(sessionId) == sessions.end()) {
            return "<html><body><h2>Access Denied</h2>"
                   "<p>Please <a href='/login'>login</a> first.</p>"
                   "</body></html>";
        }
        
        string username = sessions[sessionId];
        if (users[username].role != "admin") {
            return "<html><body><h2>Access Denied</h2>"
                   "<p>You don't have admin privileges.</p>"
                   "<a href='/index'>Back to Index</a>"
                   "</body></html>";
        }
        
        return "<html><body><h2>Admin Page</h2>"
               "<p>Welcome, Administrator " + username + "!</p>"
               "<p>This is the admin control panel.</p>"
               "<a href='/index'>Back to Index</a>"
               "</body></html>";
    }
};

int main() {
    Lse55 app;
    string sessionId;
    
    cout << "Web Application Functions Test Cases:\\n\\n";
    
    // Test Case 1: Login page
    cout << "Test Case 1: GET /login\\n";
    cout << app.login() << "\\n\\n";
    
    // Test Case 2: Login with admin credentials
    cout << "Test Case 2: POST /do_login (admin/admin123)\\n";
    cout << app.do_login("admin", "admin123", sessionId) << "\\n\\n";
    
    // Test Case 3: Index page with session
    cout << "Test Case 3: GET /index (with valid session)\\n";
    cout << app.index(sessionId) << "\\n\\n";
    
    // Test Case 4: User page with session
    cout << "Test Case 4: GET /user_page (with valid session)\\n";
    cout << app.user_page(sessionId) << "\\n\\n";
    
    // Test Case 5: Admin page with admin session
    cout << "Test Case 5: GET /admin_page (with admin session)\\n";
    cout << app.admin_page(sessionId) << "\\n\\n";
    
    return 0;
}
