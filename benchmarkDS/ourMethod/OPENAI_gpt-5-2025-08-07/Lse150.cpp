#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <algorithm>

struct Message {
    std::string username;
    std::string message;
};

static std::vector<Message> DB;
static const size_t MAX_USERNAME_LEN = 32;
static const size_t MAX_MESSAGE_LEN = 500;

struct Response {
    int statusCode;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

static bool isValidUsername(const std::string& u) {
    if (u.empty() || u.size() > MAX_USERNAME_LEN) return false;
    for (char c : u) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_')) {
            return false;
        }
    }
    return true;
}

static std::string sanitizeMessage(const std::string& m, bool& ok) {
    ok = false;
    std::string out;
    out.reserve(std::min<size_t>(m.size(), MAX_MESSAGE_LEN));
    size_t count = 0;
    for (char c : m) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (c == '\n' || c == '\t') {
            out.push_back(c);
            count++;
        } else if (!(uc < 32)) {
            out.push_back(c);
            count++;
        }
        if (count > MAX_MESSAGE_LEN) {
            return std::string();
        }
    }
    // trim
    auto notSpace = [](unsigned char ch){ return !std::isspace(ch); };
    auto itStart = std::find_if(out.begin(), out.end(), notSpace);
    auto itEnd = std::find_if(out.rbegin(), out.rend(), notSpace).base();
    std::string trimmed = (itStart < itEnd) ? std::string(itStart, itEnd) : std::string();
    if (trimmed.empty()) return std::string();
    ok = true;
    return trimmed;
}

static std::string htmlEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out.push_back(c);
        }
    }
    return out;
}

static bool insert_user_message_in_db(const std::string& username, const std::string& message) {
    try {
        DB.push_back({username, message});
        return true;
    } catch (...) {
        return false;
    }
}

// Simulated Request
struct Request {
    std::unordered_map<std::string, std::string> form;
    std::string getParam(const std::string& k) const {
        auto it = form.find(k);
        if (it == form.end()) return std::string();
        const std::string& v = it->second;
        if (v.size() > 8192) return v.substr(0, 8192);
        return v;
    }
};

// The post handler
Response post(const Request& req) {
    std::string rawUser = req.getParam("username");
    std::string rawMsg = req.getParam("message");

    // trim username
    auto ltrim = [](std::string& s){
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
    };
    auto rtrim = [](std::string& s){
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    };

    ltrim(rawUser); rtrim(rawUser);

    bool ok = false;
    std::string sanitized = sanitizeMessage(rawMsg, ok);
    if (!isValidUsername(rawUser) || !ok) {
        return {400, {{"Content-Type","text/plain; charset=UTF-8"}}, "Invalid input"};
    }

    if (!insert_user_message_in_db(rawUser, sanitized)) {
        return {500, {{"Content-Type","text/plain; charset=UTF-8"}}, "Internal error"};
    }

    return {303, {{"Location","/"}}, ""};
}

std::string getMainPage() {
    std::string html;
    html.reserve(2048);
    html += "<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"UTF-8\">";
    html += "<meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<title>Secure Message Board</title></head><body>";
    html += "<h1>Message Board</h1>";
    html += "<form method=\"POST\" action=\"/post\" autocomplete=\"off\">";
    html += "<label>Username: <input type=\"text\" name=\"username\" required maxlength=\"";
    html += std::to_string(MAX_USERNAME_LEN);
    html += "\" pattern=\"[A-Za-z0-9_-]+\"></label><br>";
    html += "<label>Message:<br><textarea name=\"message\" required maxlength=\"";
    html += std::to_string(MAX_MESSAGE_LEN);
    html += "\" rows=\"4\" cols=\"40\"></textarea></label><br>";
    html += "<button type=\"submit\">Send</button></form><hr><h2>Recent Messages</h2><ul>";
    for (const auto& m : DB) {
        html += "<li><strong>";
        html += htmlEscape(m.username);
        html += ":</strong> ";
        html += htmlEscape(m.message);
        html += "</li>";
    }
    html += "</ul></body></html>";
    return html;
}

int main() {
    // 1) Valid post
    Request r1; r1.form = {{"username","alice"},{"message","Hello, world!"}};
    Response a = post(r1);
    std::cout << "HTTP/1.1 " << a.statusCode << "\n";
    for (auto& kv : a.headers) std::cout << kv.first << ": " << kv.second << "\n";
    std::cout << "\n" << a.body << "\n";

    // 2) Valid post with HTML needing escape
    Request r2; r2.form = {{"username","Bob_123"},{"message","Hi <script>alert('x')</script> & welcome!"}};
    Response b = post(r2);
    std::cout << "HTTP/1.1 " << b.statusCode << "\n";
    for (auto& kv : b.headers) std::cout << kv.first << ": " << kv.second << "\n";
    std::cout << "\n" << b.body << "\n";

    // 3) Invalid username
    Request r3; r3.form = {{"username","bad!user"},{"message","This should fail."}};
    Response c = post(r3);
    std::cout << "HTTP/1.1 " << c.statusCode << "\n";
    for (auto& kv : c.headers) std::cout << kv.first << ": " << kv.second << "\n";
    std::cout << "\n" << c.body << "\n";

    // 4) Too long message
    Request r4; r4.form = {{"username","charlie"},{"message", std::string(600, 'x')}};
    Response d = post(r4);
    std::cout << "HTTP/1.1 " << d.statusCode << "\n";
    for (auto& kv : d.headers) std::cout << kv.first << ": " << kv.second << "\n";
    std::cout << "\n" << d.body << "\n";

    // 5) Empty after trim
    Request r5; r5.form = {{"username","david"},{"message","   \n\t   "}};
    Response e = post(r5);
    std::cout << "HTTP/1.1 " << e.statusCode << "\n";
    for (auto& kv : e.headers) std::cout << kv.first << ": " << kv.second << "\n";
    std::cout << "\n" << e.body << "\n";

    std::cout << "---- MAIN PAGE HTML ----\n";
    std::cout << getMainPage() << "\n";
    std::cout << "---- END PAGE ----\n";
    return 0;
}