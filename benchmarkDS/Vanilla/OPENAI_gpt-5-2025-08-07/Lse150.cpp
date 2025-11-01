#include <iostream>
#include <vector>
#include <string>
#include <sstream>

class Lse150 {
public:
    static void insert_user_message_in_db(const std::string& username, const std::string& message) {
        DB.emplace_back(username, message);
    }

    static std::string post(const std::string& username, const std::string& message) {
        insert_user_message_in_db(username, message);
        return "REDIRECT:/";
    }

    static std::string main_page() {
        std::ostringstream sb;
        sb << "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Main</title></head><body>";
        sb << "<h1>Message Board</h1>";
        sb << "<form method=\"post\" action=\"/post\">";
        sb << "Username: <input type=\"text\" name=\"username\"/><br/>";
        sb << "Message: <textarea name=\"message\"></textarea><br/>";
        sb << "<button type=\"submit\">Submit</button>";
        sb << "</form><hr/><ul>";
        for (const auto& p : DB) {
            sb << "<li><strong>" << escapeHtml(p.first) << ":</strong> " << escapeHtml(p.second) << "</li>";
        }
        sb << "</ul></body></html>";
        return sb.str();
    }

private:
    static std::vector<std::pair<std::string, std::string>> DB;

    static std::string escapeHtml(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                case '\'': out += "&#39;"; break;
                default: out += c; break;
            }
        }
        return out;
    }
};

std::vector<std::pair<std::string, std::string>> Lse150::DB;

int main() {
    std::cout << Lse150::main_page() << std::endl;
    std::cout << Lse150::post("alice", "Hello!") << std::endl;
    std::cout << Lse150::post("bob", "Hi there") << std::endl;
    std::cout << Lse150::post("carol", "This is a test") << std::endl;
    std::cout << Lse150::post("dave", "Another message") << std::endl;
    std::cout << Lse150::post("eve", "Last one") << std::endl;
    std::cout << Lse150::main_page() << std::endl;
    return 0;
}