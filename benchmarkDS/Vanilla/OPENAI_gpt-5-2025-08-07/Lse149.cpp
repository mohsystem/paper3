#include <bits/stdc++.h>
using namespace std;

struct Message {
    string username;
    string message;
    long long ts;
};

static vector<Message> DB;

bool insert_user_message_in_db(const string& username, const string& message) {
    if (username.empty() || message.empty()) return false;
    Message m{username, message, (long long)time(nullptr)};
    DB.push_back(m);
    return true;
}

static string escapeHtml(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            default: out += c;
        }
    }
    return out;
}

string route_root() {
    string html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Messages</title></head><body>";
    html += "<h1>Messages</h1><ul>";
    for (auto &m : DB) {
        html += "<li><strong>" + escapeHtml(m.username) + ":</strong> " + escapeHtml(m.message) + "</li>";
    }
    html += "</ul>";
    html += "<h2>Post a message</h2>"
            "<form method='POST' action='/post'>"
            "Username: <input name='username' required/> <br/>"
            "Message: <input name='message' required/> <br/>"
            "<button type='submit'>Post</button>"
            "</form>";
    html += "</body></html>";
    return html;
}

string route_post(const string& username, const string& message) {
    insert_user_message_in_db(username, message);
    // Simulate redirect to "/"
    return "REDIRECT /";
}

int main() {
    // 5 test cases
    for (int i = 1; i <= 5; ++i) {
        string u = "user" + to_string(i);
        string m = "hello world " + to_string(i);
        string res = route_post(u, m);
        cout << "POST " << i << " response: " << res << "\n";
    }
    cout << route_root() << "\n";
    return 0;
}