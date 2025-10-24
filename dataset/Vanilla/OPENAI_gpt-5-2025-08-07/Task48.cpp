#include <bits/stdc++.h>
using namespace std;

class ChatServer {
public:
    bool registerClient(const string& name, shared_ptr<deque<string>> inbox) {
        lock_guard<mutex> g(mtx_);
        if (name.empty() || !inbox) return false;
        if (clients_.count(name)) return false;
        clients_[name] = inbox;
        return true;
    }

    bool unregisterClient(const string& name) {
        lock_guard<mutex> g(mtx_);
        return clients_.erase(name) > 0;
    }

    void broadcast(const string& from, const string& message) {
        string full = from + ": " + message;
        lock_guard<mutex> g(mtx_);
        for (auto& kv : clients_) {
            kv.second->push_back(full);
        }
    }

    vector<string> connectedClients() const {
        lock_guard<mutex> g(mtx_);
        vector<string> names;
        for (auto& kv : clients_) names.push_back(kv.first);
        return names;
    }

private:
    mutable mutex mtx_;
    unordered_map<string, shared_ptr<deque<string>>> clients_;
};

class ChatClient {
public:
    ChatClient(string name, shared_ptr<ChatServer> server)
        : name_(std::move(name)), server_(std::move(server)), inbox_(make_shared<deque<string>>()) {}

    bool connect() { return server_->registerClient(name_, inbox_); }
    bool disconnect() { return server_->unregisterClient(name_); }

    void send(const string& message) { server_->broadcast(name_, message); }

    vector<string> receiveAll() {
        vector<string> out;
        while (!inbox_->empty()) {
            out.push_back(inbox_->front());
            inbox_->pop_front();
        }
        return out;
    }

    const string& name() const { return name_; }

private:
    string name_;
    shared_ptr<ChatServer> server_;
    shared_ptr<deque<string>> inbox_;
};

static void test1() {
    cout << "Test1: Single client\n";
    auto srv = make_shared<ChatServer>();
    ChatClient a("Alice", srv);
    a.connect();
    a.send("Hello");
    auto msgs = a.receiveAll();
    cout << "Alice inbox:";
    for (auto& m : msgs) cout << " [" << m << "]";
    cout << "\n";
}

static void test2() {
    cout << "Test2: Two clients\n";
    auto srv = make_shared<ChatServer>();
    ChatClient a("Alice", srv), b("Bob", srv);
    a.connect(); b.connect();
    a.send("Hi Bob"); b.send("Hi Alice");
    auto ma = a.receiveAll(), mb = b.receiveAll();
    cout << "Alice inbox:"; for (auto& m : ma) cout << " [" << m << "]"; cout << "\n";
    cout << "Bob inbox:"; for (auto& m : mb) cout << " [" << m << "]"; cout << "\n";
}

static void test3() {
    cout << "Test3: Three clients\n";
    auto srv = make_shared<ChatServer>();
    ChatClient a("A", srv), b("B", srv), c("C", srv);
    a.connect(); b.connect(); c.connect();
    a.send("One"); b.send("Two"); c.send("Three");
    auto ma = a.receiveAll(), mb = b.receiveAll(), mc = c.receiveAll();
    cout << "A inbox:"; for (auto& m : ma) cout << " [" << m << "]"; cout << "\n";
    cout << "B inbox:"; for (auto& m : mb) cout << " [" << m << "]"; cout << "\n";
    cout << "C inbox:"; for (auto& m : mc) cout << " [" << m << "]"; cout << "\n";
}

static void test4() {
    cout << "Test4: Disconnect\n";
    auto srv = make_shared<ChatServer>();
    ChatClient a("A", srv), b("B", srv);
    a.connect(); b.connect();
    a.send("Before leave");
    b.disconnect();
    a.send("After leave");
    auto ma = a.receiveAll(), mb = b.receiveAll();
    cout << "A inbox:"; for (auto& m : ma) cout << " [" << m << "]"; cout << "\n";
    cout << "B inbox (only first expected):"; for (auto& m : mb) cout << " [" << m << "]"; cout << "\n";
}

static void test5() {
    cout << "Test5: Five clients\n";
    auto srv = make_shared<ChatServer>();
    vector<unique_ptr<ChatClient>> cs;
    for (int i = 0; i < 5; ++i) {
        cs.emplace_back(make_unique<ChatClient>("C" + to_string(i), srv));
        cs.back()->connect();
    }
    for (int i = 0; i < 5; ++i) cs[i]->send("Msg" + to_string(i));
    for (int i = 0; i < 5; ++i) {
        auto msgs = cs[i]->receiveAll();
        cout << cs[i]->name() << " inbox:";
        for (auto& m : msgs) cout << " [" << m << "]";
        cout << "\n";
    }
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    return 0;
}