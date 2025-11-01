#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>

struct Message {
    std::string username;
    std::string message;
};

static std::vector<Message> DB;
static const size_t MAX_USERNAME_LEN = 32;
static const size_t MAX_MESSAGE_LEN = 280;
static const size_t MAX_MESSAGES = 1000;

static bool validate_username(const std::string& username) {
    if (username.empty() || username.size() > MAX_USERNAME_LEN) return false;
    for (unsigned char c : username) {
        if (!(std::isalnum(c) || c == '_' || c == '-')) return false;
    }
    return true;
}

static std::string sanitize_message(const std::string& message) {
    std::string out;
    out.reserve(std::min(message.size(), MAX_MESSAGE_LEN));
    for (unsigned char c : message) {
        if (c == '\n' || c == '\t' || (c >= 0x20 && c != 0x7F)) {
            out.push_back(static_cast<char>(c));
            if (out.size() >= MAX_MESSAGE_LEN) break;
        }
    }
    // trim
    while (!out.empty() && std::isspace(static_cast<unsigned char>(out.back())) && out.back() != '\n' && out.back() != '\t') out.pop_back();
    return out;
}

static bool validate_message(const std::string& message) {
    return !message.empty() && message.size() <= MAX_MESSAGE_LEN;
}

static bool insert_user_message_in_db(const std::string& username, const std::string& message) {
    if (DB.size() >= MAX_MESSAGES) {
        DB.erase(DB.begin());
    }
    DB.push_back({username, message});
    return true;
}

std::string post(const std::string& username, const std::string& message) {
    std::string u = username;
    // trim username
    u.erase(u.begin(), std::find_if(u.begin(), u.end(), [](unsigned char ch){ return !std::isspace(ch); }));
    u.erase(std::find_if(u.rbegin(), u.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), u.end());

    std::string m = sanitize_message(message);
    if (!validate_username(u)) return "ERROR: invalid username";
    if (!validate_message(m)) return "ERROR: invalid message";
    if (!insert_user_message_in_db(u, m)) return "ERROR: internal error";
    return "REDIRECT: /";
}

std::string display_messages() {
    std::string out;
    for (const auto& msg : DB) {
        std::string u = msg.username;
        std::string m = msg.message;
        u.erase(std::remove(u.begin(), u.end(), '\r'), u.end());
        m.erase(std::remove(m.begin(), m.end(), '\r'), m.end());
        out.append(u).append(": ").append(m).append("\n");
    }
    return out;
}

int main() {
    // Test 1: valid
    std::cout << post("alice", "Hello world!") << "\n";
    // Test 2: long message
    std::cout << post("bob_123", std::string(400, 'x')) << "\n";
    // Test 3: invalid username
    std::cout << post("bad user!", "This should fail") << "\n";
    // Test 4: empty message
    std::cout << post("charlie", "   ") << "\n";
    // Test 5: control + HTML
    std::cout << post("dana", std::string("<script>alert(1)</script>\x01\x02ok")) << "\n";

    std::cout << "=== Messages ===\n";
    std::cout << display_messages();
    return 0;
}