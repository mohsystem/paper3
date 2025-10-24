
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <openssl/sha.h>

class Task43 {
private:
    static const int SESSION_TIMEOUT_SECONDS = 1800; // 30 minutes
    static const int SESSION_ID_LENGTH = 32;
    
    struct Session {
        std::string session_id;
        std::string user_id;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point last_accessed;
        std::unordered_map<std::string, std::string> attributes;
        
        Session(const std::string& sid, const std::string& uid)
            : session_id(sid), user_id(uid),
              created_at(std::chrono::system_clock::now()),
              last_accessed(std::chrono::system_clock::now()) {}
        
        bool is_expired() const {
            auto now = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                now - last_accessed);
            return duration.count() >= SESSION_TIMEOUT_SECONDS;
        }
        
        void update_last_accessed() {
            last_accessed = std::chrono::system_clock::now();
        }
    };
    
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions;
    std::mutex sessions_mutex;
    
    std::string generate_secure_random() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned long long> dis;
        
        unsigned char random_bytes[SESSION_ID_LENGTH];
        for (int i = 0; i < SESSION_ID_LENGTH; ++i) {
            random_bytes[i] = static_cast<unsigned char>(dis(gen) % 256);
        }
        
        return hash_bytes(random_bytes, SESSION_ID_LENGTH);
    }
    
    std::string hash_bytes(const unsigned char* data, size_t len) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(data, len, hash);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
    
    void clean_expired_sessions() {
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = sessions.begin();
        while (it != sessions.end()) {
            if (it->second->is_expired()) {
                it->second->attributes.clear();
                it = sessions.erase(it);
            } else {
                ++it;
            }
        }
    }
    
public:
    std::string create_session(const std::string& user_id) {
        if (user_id.empty()) {
            throw std::invalid_argument("User ID cannot be empty");
        }
        
        clean_expired_sessions();
        
        std::string session_id = generate_secure_random();
        
        std::lock_guard<std::mutex> lock(sessions_mutex);
        sessions[session_id] = std::make_shared<Session>(session_id, user_id);
        
        return session_id;
    }
    
    bool validate_session(const std::string& session_id) {
        if (session_id.empty()) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = sessions.find(session_id);
        
        if (it == sessions.end()) {
            return false;
        }
        
        if (it->second->is_expired()) {
            it->second->attributes.clear();
            sessions.erase(it);
            return false;
        }
        
        it->second->update_last_accessed();
        return true;
    }
    
    std::string get_session_user_id(const std::string& session_id) {
        if (!validate_session(session_id)) {
            return "";
        }
        
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = sessions.find(session_id);
        return (it != sessions.end()) ? it->second->user_id : "";
    }
    
    void terminate_session(const std::string& session_id) {
        if (session_id.empty()) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = sessions.find(session_id);
        if (it != sessions.end()) {
            it->second->attributes.clear();
            sessions.erase(it);
        }
    }
    
    bool set_session_attribute(const std::string& session_id,
                               const std::string& key,
                               const std::string& value) {
        if (!validate_session(session_id) || key.empty() || value.empty()) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = sessions.find(session_id);
        if (it != sessions.end()) {
            it->second->attributes[key] = value;
            return true;
        }
        return false;
    }
    
    std::string get_session_attribute(const std::string& session_id,
                                      const std::string& key) {
        if (!validate_session(session_id) || key.empty()) {
            return "";
        }
        
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = sessions.find(session_id);
        if (it != sessions.end()) {
            auto attr_it = it->second->attributes.find(key);
            if (attr_it != it->second->attributes.end()) {
                return attr_it->second;
            }
        }
        return "";
    }
};

int main() {
    std::cout << "=== Secure Session Management Test Cases ===\\n\\n";
    
    Task43 manager;
    
    // Test Case 1: Create session for user
    std::cout << "Test 1: Create session\\n";
    std::string session_id1 = manager.create_session("user123");
    std::cout << "Session created: " << session_id1 << "\\n";
    std::cout << "Valid: " << (manager.validate_session(session_id1) ? "true" : "false") << "\\n\\n";
    
    // Test Case 2: Validate existing session
    std::cout << "Test 2: Validate session\\n";
    bool is_valid = manager.validate_session(session_id1);
    std::cout << "Session valid: " << (is_valid ? "true" : "false") << "\\n";
    std::cout << "User ID: " << manager.get_session_user_id(session_id1) << "\\n\\n";
    
    // Test Case 3: Set and get session attributes
    std::cout << "Test 3: Session attributes\\n";
    manager.set_session_attribute(session_id1, "role", "admin");
    manager.set_session_attribute(session_id1, "preference", "dark_mode");
    std::cout << "Role: " << manager.get_session_attribute(session_id1, "role") << "\\n";
    std::cout << "Preference: " << manager.get_session_attribute(session_id1, "preference") << "\\n\\n";
    
    // Test Case 4: Multiple sessions
    std::cout << "Test 4: Multiple sessions\\n";
    std::string session_id2 = manager.create_session("user456");
    std::string session_id3 = manager.create_session("user789");
    std::cout << "Session 2 created: " << session_id2 << "\\n";
    std::cout << "Session 3 created: " << session_id3 << "\\n";
    std::cout << "Session 1 valid: " << (manager.validate_session(session_id1) ? "true" : "false") << "\\n";
    std::cout << "Session 2 valid: " << (manager.validate_session(session_id2) ? "true" : "false") << "\\n";
    std::cout << "Session 3 valid: " << (manager.validate_session(session_id3) ? "true" : "false") << "\\n\\n";
    
    // Test Case 5: Terminate session
    std::cout << "Test 5: Terminate session\\n";
    manager.terminate_session(session_id1);
    std::cout << "Session 1 after termination: " << (manager.validate_session(session_id1) ? "true" : "false") << "\\n";
    std::cout << "Session 2 still valid: " << (manager.validate_session(session_id2) ? "true" : "false") << "\\n";
    std::cout << "Session 3 still valid: " << (manager.validate_session(session_id3) ? "true" : "false") << "\\n";
    
    // Cleanup
    manager.terminate_session(session_id2);
    manager.terminate_session(session_id3);
    
    return 0;
}
