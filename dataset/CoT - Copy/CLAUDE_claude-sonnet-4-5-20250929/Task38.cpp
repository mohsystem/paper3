
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <random>
#include <regex>

class Task38 {
private:
    static std::map<std::string, std::string> csrfTokens;

    static std::string generateCSRFToken() {
        const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2);
        
        std::string token;
        for (int i = 0; i < 32; ++i) {
            token += charset[distribution(generator)];
        }
        return token;
    }

    static std::string escapeHtml(const std::string& input) {
        std::string output;
        output.reserve(input.size());
        for (char c : input) {
            switch (c) {
                case '&': output += "&amp;"; break;
                case '<': output += "&lt;"; break;
                case '>': output += "&gt;"; break;
                case '"': output += "&quot;"; break;
                case '\\'': output += "&#x27;"; break;
                case '/': output += "&#x2F;"; break;
                default: output += c; break;
            }
        }
        return output;
    }

    static std::string sanitizeInput(std::string input, size_t maxLength) {
        input.erase(0, input.find_first_not_of(" \\t\\n\\r"));
        input.erase(input.find_last_not_of(" \\t\\n\\r") + 1);
        if (input.length() > maxLength) {
            input = input.substr(0, maxLength);
        }
        return input;
    }

    static bool isValidEmail(const std::string& email) {
        std::regex pattern(R"(^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$)");
        return std::regex_match(email, pattern);
    }

    static std::string urlDecode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%' && i + 2 < str.length()) {
                int value;
                std::istringstream is(str.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    result += static_cast<char>(value);
                    i += 2;
                } else {
                    result += str[i];
                }
            } else if (str[i] == '+') {
                result += ' ';
            } else {
                result += str[i];
            }
        }
        return result;
    }

public:
    static std::string getFormPage() {
        std::string csrfToken = generateCSRFToken();
        csrfTokens[csrfToken] = "valid";
        
        std::ostringstream html;
        html << "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
             << "<title>Secure Form</title></head><body>"
             << "<h1>User Input Form</h1>"
             << "<form method='POST' action='/submit'>"
             << "<input type='hidden' name='csrf_token' value='" << escapeHtml(csrfToken) << "'>"
             << "<label>Name: <input type='text' name='name' maxlength='100' required></label><br>"
             << "<label>Email: <input type='email' name='email' maxlength='100' required></label><br>"
             << "<label>Message: <textarea name='message' maxlength='500' required></textarea></label><br>"
             << "<input type='submit' value='Submit'>"
             << "</form></body></html>";
        
        return html.str();
    }

    static std::string processFormSubmission(const std::string& formData) {
        std::map<std::string, std::string> params;
        std::istringstream stream(formData);
        std::string pair;
        
        while (std::getline(stream, pair, '&')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = urlDecode(pair.substr(0, pos));
                std::string value = urlDecode(pair.substr(pos + 1));
                params[key] = value;
            }
        }
        
        std::string csrfToken = params["csrf_token"];
        if (csrfTokens.find(csrfToken) == csrfTokens.end()) {
            return "Error: Invalid CSRF token";
        }
        csrfTokens.erase(csrfToken);
        
        std::string name = sanitizeInput(params["name"], 100);
        std::string email = sanitizeInput(params["email"], 100);
        std::string message = sanitizeInput(params["message"], 500);
        
        if (!isValidEmail(email)) {
            return "Error: Invalid email format";
        }
        
        std::ostringstream html;
        html << "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
             << "<title>Form Submission Result</title></head><body>"
             << "<h1>Your Submitted Data</h1>"
             << "<p><strong>Name:</strong> " << escapeHtml(name) << "</p>"
             << "<p><strong>Email:</strong> " << escapeHtml(email) << "</p>"
             << "<p><strong>Message:</strong> " << escapeHtml(message) << "</p>"
             << "<a href='/'>Back to Form</a></body></html>";
        
        return html.str();
    }

    static void runTests() {
        std::cout << "Test 1: Generate form page" << std::endl;
        std::cout << getFormPage().substr(0, 50) << "..." << std::endl << std::endl;
        
        std::cout << "Test 2: Process valid submission" << std::endl;
        std::string token = generateCSRFToken();
        csrfTokens[token] = "valid";
        std::string formData = "csrf_token=" + token + "&name=John+Doe&email=john@example.com&message=Hello";
        std::cout << processFormSubmission(formData).substr(0, 100) << "..." << std::endl << std::endl;
        
        std::cout << "Test 3: Test XSS prevention" << std::endl;
        token = generateCSRFToken();
        csrfTokens[token] = "valid";
        formData = "csrf_token=" + token + "&name=%3Cscript%3Ealert%281%29%3C%2Fscript%3E&email=test@test.com&message=Test";
        std::cout << processFormSubmission(formData).substr(0, 100) << "..." << std::endl << std::endl;
        
        std::cout << "Test 4: Invalid CSRF token" << std::endl;
        formData = "csrf_token=invalid&name=Test&email=test@test.com&message=Test";
        std::cout << processFormSubmission(formData) << std::endl << std::endl;
        
        std::cout << "Test 5: Invalid email format" << std::endl;
        token = generateCSRFToken();
        csrfTokens[token] = "valid";
        formData = "csrf_token=" + token + "&name=Test&email=invalid-email&message=Test";
        std::cout << processFormSubmission(formData) << std::endl;
    }
};

std::map<std::string, std::string> Task38::csrfTokens;

int main() {
    Task38::runTests();
    return 0;
}
