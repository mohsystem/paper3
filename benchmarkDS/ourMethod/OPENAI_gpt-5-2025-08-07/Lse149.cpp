#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

struct Message {
    std::string username;
    std::string message;
};

class InMemoryDB {
public:
    explicit InMemoryDB(size_t maxMessages = 1000) : maxMessages_(maxMessages ? maxMessages : 1) {}

    bool insert_user_message_in_db(const std::string& username, const std::string& message) {
        if (!isValidUsername(username)) return false;
        if (!isValidMessage(message)) return false;
        if (messages_.size() >= maxMessages_) {
            messages_.erase(messages_.begin());
        }
        messages_.push_back(Message{username, message});
        return true;
    }

    std::vector<Message> getAll() const {
        return messages_;
    }

private:
    static bool isValidUsername(const std::string& u) {
        if (u.empty() || u.size() > 20) return false;
        for (char c : u) {
            if (!(c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
                return false;
            }
        }
        return true;
    }

    static bool isValidMessage(const std::string& m) {
        return !m.empty() && m.size() <= 280;
    }

    std::vector<Message> messages_;
    size_t maxMessages_;
};

struct Request {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> form;
};

struct Response {
    int statusCode;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string toString() const {
        std::string out = "Status: " + std::to_string(statusCode) + "\n";
        for (const auto& kv : headers) {
            out += kv.first + ": " + kv.second + "\n";
        }
        out += "\n";
        out += body;
        return out;
    }
};

static std::string escapeHtml(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default:
                if (static_cast<unsigned char>(c) >= 0x20 || c == '\n' || c == '\t')
                    out.push_back(c);
                // else skip control
        }
    }
    return out;
}

class App {
public:
    explicit App(InMemoryDB& db) : db_(db) {}

    Response handle(const Request& req) {
        if (req.method == "GET" && req.path == "/") return handleRoot();
        if (req.method == "POST" && req.path == "/post") return handlePost(req);
        return text(404, "Not Found");
    }

private:
    Response handleRoot() {
        auto msgs = db_.getAll();
        std::string html = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Messages</title></head><body>";
        html += "<h1>Message Board</h1>";
        html += "<form method=\"POST\" action=\"/post\" onsubmit=\"return /^[A-Za-z0-9_]{1,20}$/.test(this.username.value);\">";
        html += "Username: <input name=\"username\" maxlength=\"20\" required pattern=\"[A-Za-z0-9_]+\"> ";
        html += "Message: <input name=\"message\" maxlength=\"280\" required> ";
        html += "<button type=\"submit\">Post</button></form>";
        html += "<h2>Messages</h2><ul>";
        for (const auto& m : msgs) {
            html += "<li><strong>" + escapeHtml(m.username) + ":</strong> " + escapeHtml(m.message) + "</li>";
        }
        html += "</ul></body></html>";
        return makeHtml(200, html);
    }

    Response handlePost(const Request& req) {
        auto itU = req.form.find("username");
        auto itM = req.form.find("message");
        std::string username = (itU == req.form.end() ? "" : trim(itU->second));
        std::string message = (itM == req.form.end() ? "" : trim(itM->second));

        if (!db_.insert_user_message_in_db(username, message)) {
            return text(400, "Invalid input");
        }
        Response r{303, {}, ""};
        r.headers["Location"] = "/";
        return r;
    }

    static std::string trim(const std::string& s) {
        size_t b = 0, e = s.size();
        while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\n' || s[b] == '\r')) b++;
        while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\n' || s[e - 1] == '\r')) e--;
        return s.substr(b, e - b);
    }

    static Response text(int code, const std::string& body) {
        Response r{code, {}, body};
        r.headers["Content-Type"] = "text/plain; charset=utf-8";
        return r;
    }

    static Response makeHtml(int code, const std::string& body) {
        Response r{code, {}, body};
        r.headers["Content-Type"] = "text/html; charset=utf-8";
        return r;
    }

    InMemoryDB& db_;
};

int main() {
    InMemoryDB db(1000);
    App app(db);

    // Test 1: Valid post then GET
    {
        Request req{"POST", "/post", {{"username", "alice"}, {"message", "Hello, world!"}}};
        std::cout << "Test 1 POST /post\n" << app.handle(req).toString() << "\n";
        std::cout << "GET / after Test 1\n" << app.handle(Request{"GET", "/", {}}).toString() << "\n";
        std::cout << "------------------------------------------------------------\n";
    }

    // Test 2: Invalid username
    {
        Request req{"POST", "/post", {{"username", "bob!"}, {"message", "Should fail"}}};
        std::cout << "Test 2 POST /post invalid username\n" << app.handle(req).toString() << "\n";
        std::cout << "------------------------------------------------------------\n";
    }

    // Test 3: Too long message
    {
        std::string longMsg(281, 'x');
        Request req{"POST", "/post", {{"username", "charlie"}, {"message", longMsg}}};
        std::cout << "Test 3 POST /post long message\n" << app.handle(req).toString() << "\n";
        std::cout << "------------------------------------------------------------\n";
    }

    // Test 4: XSS attempt
    {
        Request req{"POST", "/post", {{"username", "dana"}, {"message", "<script>alert(1)</script> ok"}}};
        std::cout << "Test 4 POST /post XSS attempt\n" << app.handle(req).toString() << "\n";
        std::cout << "GET / after Test 4\n" << app.handle(Request{"GET", "/", {}}).toString() << "\n";
        std::cout << "------------------------------------------------------------\n";
    }

    // Test 5: Another valid post
    {
        Request req{"POST", "/post", {{"username", "eve_123"}, {"message", "Good day!"}}};
        std::cout << "Test 5 POST /post valid again\n" << app.handle(req).toString() << "\n";
        std::cout << "GET / after Test 5\n" << app.handle(Request{"GET", "/", {}}).toString() << "\n";
        std::cout << "------------------------------------------------------------\n";
    }

    return 0;
}