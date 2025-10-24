#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <algorithm>

class Client {
public:
    Client(int id, const std::string& name) : id_(id), name_(name) {}

    int id() const { return id_; }
    const std::string& name() const { return name_; }

    void receive(const std::string& msg) {
        std::lock_guard<std::mutex> lg(mu_);
        if (inbox_.size() > 10000) {
            inbox_.clear(); // prevent unbounded growth
        }
        inbox_.push_back(msg);
    }

    std::vector<std::string> fetch_all() {
        std::lock_guard<std::mutex> lg(mu_);
        std::vector<std::string> out;
        out.swap(inbox_);
        return out;
    }

private:
    int id_;
    std::string name_;
    std::vector<std::string> inbox_;
    std::mutex mu_;
};

class ChatServer {
public:
    ChatServer() : next_id_(1) {}

    int register_client(const std::string& name) {
        std::string safe_name = sanitize_name(name);
        if (safe_name.empty()) {
            safe_name = "user" + std::to_string(next_id_);
        }
        std::lock_guard<std::mutex> lg(mu_);
        int id = next_id_++;
        clients_.emplace(id, Client(id, safe_name));
        return id;
    }

    bool disconnect_client(int client_id) {
        std::lock_guard<std::mutex> lg(mu_);
        return clients_.erase(client_id) > 0;
    }

    int get_client_count() {
        std::lock_guard<std::mutex> lg(mu_);
        return static_cast<int>(clients_.size());
    }

    std::vector<std::string> get_client_messages(int client_id) {
        std::lock_guard<std::mutex> lg(mu_);
        auto it = clients_.find(client_id);
        if (it == clients_.end()) return {};
        return it->second.fetch_all();
    }

    int send_from(int client_id, const std::string& message) {
        std::vector<std::reference_wrapper<Client>> snapshot;
        std::string payload;
        {
            std::lock_guard<std::mutex> lg(mu_);
            auto it = clients_.find(client_id);
            if (it == clients_.end()) return 0;
            std::string sanitized = sanitize_message(message);
            if (sanitized.empty()) return 0;
            payload = it->second.name() + ": " + sanitized;
            snapshot.reserve(clients_.size());
            for (auto& kv : clients_) {
                snapshot.push_back(std::ref(kv.second));
            }
        }
        int count = 0;
        for (Client& c : snapshot) {
            c.receive(payload);
            count++;
        }
        return count;
    }

private:
    static std::string sanitize_message(const std::string& msg) {
        std::string out;
        out.reserve(256);
        int maxLen = 256;
        for (char ch : msg) {
            if ((int)out.size() >= maxLen) break;
            unsigned char u = static_cast<unsigned char>(ch);
            if (u >= 32 && u != 127) {
                out.push_back(ch);
            } else if (ch == '\n' || ch == '\r' || ch == '\t') {
                out.push_back(' ');
            }
        }
        // trim
        while (!out.empty() && (out.back() == ' ')) out.pop_back();
        size_t start = 0;
        while (start < out.size() && out[start] == ' ') start++;
        return out.substr(start);
    }

    static std::string sanitize_name(const std::string& name) {
        std::string out;
        out.reserve(32);
        for (char ch : name) {
            if ((int)out.size() >= 32) break;
            unsigned char u = static_cast<unsigned char>(ch);
            if (u >= 33 && u < 127 && ch != ':') {
                out.push_back(ch);
            }
        }
        // trim
        while (!out.empty() && (out.back() == ' ')) out.pop_back();
        size_t start = 0;
        while (start < out.size() && out[start] == ' ') start++;
        return out.substr(start);
    }

    int next_id_;
    std::unordered_map<int, Client> clients_;
    std::mutex mu_;
};

static void print_inbox(const std::string& label, const std::vector<std::string>& msgs) {
    std::cout << label << " (" << msgs.size() << "):\n";
    for (const auto& m : msgs) {
        std::cout << "  " << m << "\n";
    }
}

int main() {
    ChatServer server;

    int alice = server.register_client("Alice");
    int bob = server.register_client("Bob");
    int charlie = server.register_client("Charlie");

    std::cout << "Test 1: Single client receives own message\n";
    server.send_from(alice, "Hello everyone!");
    print_inbox("Alice inbox after send", server.get_client_messages(alice));
    print_inbox("Bob inbox", server.get_client_messages(bob));
    print_inbox("Charlie inbox", server.get_client_messages(charlie));
    std::cout << "----\n";

    std::cout << "Test 2: Control chars sanitized\n";
    server.send_from(bob, std::string("Hi\nthere\tall!"));
    print_inbox("Alice inbox", server.get_client_messages(alice));
    print_inbox("Bob inbox", server.get_client_messages(bob));
    print_inbox("Charlie inbox", server.get_client_messages(charlie));
    std::cout << "----\n";

    std::cout << "Test 3: Long message truncated\n";
    std::string longMsg(400, 'X');
    server.send_from(charlie, longMsg);
    print_inbox("Alice inbox", server.get_client_messages(alice));
    std::cout << "----\n";

    std::cout << "Test 4: Empty/invalid message rejected\n";
    int count = server.send_from(alice, std::string("\x01\x02\x03", 3));
    std::cout << "Broadcast count (should be 0): " << count << "\n";
    print_inbox("Bob inbox", server.get_client_messages(bob));
    std::cout << "----\n";

    std::cout << "Test 5: Disconnect behavior\n";
    server.disconnect_client(charlie);
    server.send_from(bob, "After disconnect");
    print_inbox("Alice inbox", server.get_client_messages(alice));
    print_inbox("Charlie inbox (should be empty)", server.get_client_messages(charlie));
    std::cout << "Active clients: " << server.get_client_count() << "\n";

    return 0;
}