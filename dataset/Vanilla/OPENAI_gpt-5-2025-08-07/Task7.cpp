#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

static const std::string USER_DB = "users.db";
static std::unordered_map<std::string, bool> sessions;

// Client: Build login request
std::string clientBuildLoginRequest(const std::string& username, const std::string& password) {
    return "LOGIN|" + username + "|" + password;
}

// Client: Send login
std::string clientSendLogin(const std::string& username, const std::string& password);

// Server: Ensure user DB
void ensureUserDB(const std::unordered_map<std::string, std::string>& users) {
    std::ofstream out(USER_DB, std::ios::trunc);
    for (const auto& kv : users) {
        out << kv.first << ":" << kv.second << "\n";
    }
}

// Server: Authenticate
bool authenticate(const std::string& username, const std::string& password) {
    std::ifstream in(USER_DB);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        auto pos = line.find(':');
        if (pos == std::string::npos) continue;
        std::string u = line.substr(0, pos);
        std::string p = line.substr(pos + 1);
        if (u == username && p == password) return true;
    }
    return false;
}

// Server: Process
std::string serverProcess(const std::string& request) {
    if (request.empty()) return "ERR|INVALID|EmptyRequest";
    std::vector<std::string> parts;
    {
        std::stringstream ss(request);
        std::string token;
        while (std::getline(ss, token, '|')) {
            parts.push_back(token);
        }
    }
    if (parts.empty()) return "ERR|INVALID|Malformed";
    std::string action = parts[0];
    for (auto& c : action) c = std::toupper(c);

    if (action == "LOGIN") {
        if (parts.size() != 3) return "ERR|LOGIN|InvalidArgs";
        const std::string& username = parts[1];
        const std::string& password = parts[2];
        if (authenticate(username, password)) {
            sessions[username] = true;
            return "OK|LOGIN|Welcome " + username;
        } else {
            return "ERR|LOGIN|Unauthorized";
        }
    } else if (action == "SEND") {
        if (parts.size() < 4) return "ERR|SEND|InvalidArgs";
        const std::string& from = parts[1];
        const std::string& to = parts[2];
        std::string message;
        // Reconstruct message preserving '|'
        for (size_t i = 3; i < parts.size(); ++i) {
            if (i > 3) message += "|";
            message += parts[i];
        }
        if (!sessions.count(from) || !sessions[from]) return "ERR|SEND|NotAuthenticated";
        std::ofstream log("chat.log", std::ios::app);
        if (!log) return "ERR|SEND|StorageError";
        log << "FROM:" << from << " TO:" << to << " MSG:" << message << "\n";
        return "OK|SEND|DeliveredTo|" + to;
    } else {
        return "ERR|INVALID|UnknownAction";
    }
}

std::string clientSendLogin(const std::string& username, const std::string& password) {
    std::string req = clientBuildLoginRequest(username, password);
    return serverProcess(req);
}

int main() {
    ensureUserDB({{"alice","password123"},{"bob","letmein"}});
    // Test 1
    std::cout << "Test1: " << clientSendLogin("alice", "password123") << std::endl;
    // Test 2
    std::cout << "Test2: " << clientSendLogin("alice", "wrong") << std::endl;
    // Test 3
    std::cout << "Test3: " << clientSendLogin("charlie", "nopass") << std::endl;
    // Test 4
    std::cout << "Test4: " << clientSendLogin("bob", "letmein") << std::endl;
    // Test 5
    std::cout << "Test5: " << serverProcess("SEND|alice|bob|Hello Bob!") << std::endl;
    return 0;
}