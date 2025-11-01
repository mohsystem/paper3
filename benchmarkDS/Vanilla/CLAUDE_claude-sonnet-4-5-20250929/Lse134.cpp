
#include <iostream>
#include <string>
#include <map>

using namespace std;

class Lse134 {
public:
    static string escapeHtml(const string& input) {
        string result;
        for (char c : input) {
            switch (c) {
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '&':
                    result += "&amp;";
                    break;
                case '"':
                    result += "&quot;";
                    break;
                case '\\'':
                    result += "&#x27;";
                    break;
                case '/':
                    result += "&#x2F;";
                    break;
                default:
                    result += c;
            }
        }
        return result;
    }
    
    static string hello(const string& username) {
        return "Hello " + escapeHtml(username);
    }
};

int main() {
    // Test cases
    cout << Lse134::hello("John") << endl;
    cout << Lse134::hello("Alice") << endl;
    cout << Lse134::hello("<script>alert('XSS')</script>") << endl;
    cout << Lse134::hello("User&Name") << endl;
    cout << Lse134::hello("Test'User\\"123") << endl;
    
    return 0;
}
