#include <iostream>
#include <vector>
#include <string>

class Lse148 {
public:
    struct Message { std::string username; std::string content; };
    static std::vector<Message> db;

    static bool insert_user_message_in_db(const std::string& username, const std::string& message) {
        db.push_back({username, message});
        return true;
    }

    static std::string post(const std::string& username, const std::string& message) {
        insert_user_message_in_db(username, message);
        return "REDIRECT:/";
    }

    static std::string display_messages() {
        std::string out;
        for (size_t i = 0; i < db.size(); ++i) {
            out += std::to_string(i + 1) + ". " + db[i].username + ": " + db[i].content;
            if (i + 1 < db.size()) out += "\n";
        }
        return out;
    }
};

std::vector<Lse148::Message> Lse148::db;

int main() {
    std::cout << Lse148::post("alice", "Hello world") << std::endl;
    std::cout << Lse148::post("bob", "Hi Alice") << std::endl;
    std::cout << Lse148::post("carol", "Good morning") << std::endl;
    std::cout << Lse148::post("dave", "How are you?") << std::endl;
    std::cout << Lse148::post("eve", "All good!") << std::endl;
    std::cout << "Main page messages:" << std::endl;
    std::cout << Lse148::display_messages() << std::endl;
    return 0;
}