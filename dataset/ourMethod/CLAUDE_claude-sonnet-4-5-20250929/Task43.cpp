
// C++ Secure Session Management Web Application
// Uses OpenSSL for cryptographic operations and libmicrohttpd for HTTP server
// Compile: g++ -std=c++17 -o session_app session_app.cpp -lmicrohttpd -lssl -lcrypto -pthread

#include <microhttpd.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <cstring>
#include <ctime>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Session timeout in seconds (15 minutes)
constexpr time_t SESSION_TIMEOUT = 900;
constexpr size_t SESSION_ID_BYTES = 32;
constexpr size_t MAX_SESSIONS = 10000;

// Session data structure with secure cleanup
struct Session {
    std::string user_id;
    time_t created_at;
    time_t last_accessed;
    
    Session(const std::string& uid) : user_id(uid) {
        created_at = time(nullptr);
        last_accessed = created_at;
    }
    
    ~Session() {
        // Clear sensitive data on destruction (defense in depth)
        if (!user_id.empty()) {
            OPENSSL_cleanse(&user_id[0], user_id.size());
        }
    }
    
    bool is_expired() const {
        time_t now = time(nullptr);
        // Prevent integer overflow in time calculations
        if (now < last_accessed) return true;
        return (now - last_accessed) > SESSION_TIMEOUT;
    }
    
    void update_access_time() {
        last_accessed = time(nullptr);
    }
};

// Thread-safe session store
class SessionStore {
private:
    std::unordered_map<std::string, std::unique_ptr<Session>> sessions;
    std::mutex mtx;
    
public:
    // Generate cryptographically secure session ID
    // Uses OpenSSL RAND_bytes to ensure unpredictability
    std::string generate_session_id() {
        unsigned char random_bytes[SESSION_ID_BYTES];
        
        // Use cryptographically secure RNG (Rules#6, #7)
        if (RAND_bytes(random_bytes, SESSION_ID_BYTES) != 1) {
            return ""; // Failed to generate secure random
        }
        
        // Convert to hex string for safe URL transmission
        std::stringstream ss;
        for (size_t i = 0; i < SESSION_ID_BYTES; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(random_bytes[i]);
        }
        
        // Clear sensitive random bytes from memory (Rules#1)
        OPENSSL_cleanse(random_bytes, SESSION_ID_BYTES);
        
        return ss.str();
    }
    
    // Create new session with validation
    std::string create_session(const std::string& user_id) {
        std::lock_guard<std::mutex> lock(mtx);
        
        // Validate user_id input (Rules#1, #15)
        if (user_id.empty() || user_id.length() > 256) {
            return ""; // Invalid user_id
        }
        
        // Prevent DoS by limiting total sessions (Rules#1)
        if (sessions.size() >= MAX_SESSIONS) {
            cleanup_expired_sessions_unlocked();
            if (sessions.size() >= MAX_SESSIONS) {
                return ""; // Too many sessions
            }
        }
        
        std::string session_id = generate_session_id();
        if (session_id.empty()) {
            return ""; // Failed to generate ID
        }
        
        // Ensure uniqueness (extremely unlikely with 256-bit random)
        if (sessions.find(session_id) != sessions.end()) {
            return ""; // Collision (retry in production)
        }
        
        sessions[session_id] = std::make_unique<Session>(user_id);
        return session_id;
    }
    
    // Validate and retrieve session
    Session* get_session(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(mtx);
        
        // Validate session_id format (Rules#1, #15)
        if (session_id.length() != SESSION_ID_BYTES * 2) {
            return nullptr;
        }
        
        // Check if contains only hex characters
        if (!std::all_of(session_id.begin(), session_id.end(), ::isxdigit)) {
            return nullptr;
        }
        
        auto it = sessions.find(session_id);
        if (it == sessions.end()) {
            return nullptr;
        }
        
        Session* session = it->second.get();
        if (session->is_expired()) {
            sessions.erase(it); // Auto-cleanup expired session
            return nullptr;
        }
        
        session->update_access_time();
        return session;
    }
    
    // Terminate session securely
    bool terminate_session(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(mtx);
        
        // Validate input (Rules#1, #15)
        if (session_id.length() != SESSION_ID_BYTES * 2) {
            return false;
        }
        
        auto it = sessions.find(session_id);
        if (it == sessions.end()) {
            return false;
        }
        
        // Destructor will clear sensitive data
        sessions.erase(it);
        return true;
    }
    
    // Cleanup expired sessions (called periodically)
    void cleanup_expired_sessions() {
        std::lock_guard<std::mutex> lock(mtx);
        cleanup_expired_sessions_unlocked();
    }
    
private:
    void cleanup_expired_sessions_unlocked() {
        auto it = sessions.begin();
        while (it != sessions.end()) {
            if (it->second->is_expired()) {
                it = sessions.erase(it);
            } else {
                ++it;
            }
        }
    }
};

// Global session store
static SessionStore g_session_store;

// HTML-encode output to prevent XSS (Rules#1, #15)
std::string html_encode(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size() * 1.2); // Pre-allocate
    
    for (char c : data) {
        switch(c) {
            case '&':  buffer.append("&amp;");  break;
            case '\\"': buffer.append("&quot;"); break;
            case '\\'': buffer.append("&#39;");  break;
            case '<':  buffer.append("&lt;");   break;
            case '>':  buffer.append("&gt;");   break;
            default:   
                if (c >= 32 && c <= 126) {
                    buffer += c;
                }
                break;
        }
    }
    return buffer;
}

// Extract cookie value from header
std::string extract_cookie(const char* cookie_header, const std::string& name) {
    if (!cookie_header) return "";
    
    std::string cookies(cookie_header);
    std::string search = name + "=";
    size_t pos = cookies.find(search);
    
    if (pos == std::string::npos) return "";
    
    pos += search.length();
    size_t end = cookies.find(';', pos);
    
    std::string value = (end == std::string::npos) 
        ? cookies.substr(pos) 
        : cookies.substr(pos, end - pos);
    
    // Validate cookie value length (Rules#1, #15)
    if (value.length() > 256) return "";
    
    return value;
}

// HTTP request handler
static enum MHD_Result handle_request(void* cls, struct MHD_Connection* connection,
                                     const char* url, const char* method,
                                     const char* version, const char* upload_data,
                                     size_t* upload_data_size, void** con_cls) {
    
    struct MHD_Response* response = nullptr;
    enum MHD_Result ret;
    int status_code = MHD_HTTP_OK;
    std::string content;
    
    // Only allow GET and POST (Rules#1)
    if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0) {
        content = "Method not allowed";
        status_code = MHD_HTTP_METHOD_NOT_ALLOWED;
        goto send_response;
    }
    
    // Route: Create session
    if (strcmp(url, "/create") == 0 && strcmp(method, "POST") == 0) {
        // In real app, validate credentials here
        const char* user_param = MHD_lookup_connection_value(connection, 
                                                             MHD_GET_ARGUMENT_KIND, 
                                                             "user");
        
        if (!user_param || strlen(user_param) == 0 || strlen(user_param) > 256) {
            content = "{\\"error\\":\\"Invalid user parameter\\"}";
            status_code = MHD_HTTP_BAD_REQUEST;
            goto send_response;
        }
        
        std::string session_id = g_session_store.create_session(user_param);
        
        if (session_id.empty()) {
            content = "{\\"error\\":\\"Failed to create session\\"}";
            status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
        } else {
            content = "{\\"status\\":\\"Session created\\"}";
            response = MHD_create_response_from_buffer(content.length(),
                                                       (void*)content.c_str(),
                                                       MHD_RESPMEM_MUST_COPY);
            
            // Set secure cookie with HttpOnly and SameSite (Rules#1, #13)
            std::string cookie = "session_id=" + session_id + 
                               "; HttpOnly; Secure; SameSite=Strict; Max-Age=900";
            MHD_add_response_header(response, "Set-Cookie", cookie.c_str());
            MHD_add_response_header(response, "Content-Type", "application/json");
            
            ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
            return ret;
        }
    }
    // Route: Check session
    else if (strcmp(url, "/check") == 0 && strcmp(method, "GET") == 0) {
        const char* cookie_header = MHD_lookup_connection_value(connection,
                                                                MHD_HEADER_KIND,
                                                                "Cookie");
        
        std::string session_id = extract_cookie(cookie_header, "session_id");
        Session* session = g_session_store.get_session(session_id);
        
        if (session) {
            // Don't leak user_id in logs (Rules#1)\n            content = "{\\"status\\":\\"Valid session\\",\\"user\\":\\"" + \n                     html_encode(session->user_id) + "\\"}";\n        } else {\n            content = "{\\"status\\":\\"No valid session\\"}";\n            status_code = MHD_HTTP_UNAUTHORIZED;\n        }\n    }\n    // Route: Terminate session\n    else if (strcmp(url, "/logout") == 0 && strcmp(method, "POST") == 0) {\n        const char* cookie_header = MHD_lookup_connection_value(connection,\n                                                                MHD_HEADER_KIND,\n                                                                "Cookie");\n        \n        std::string session_id = extract_cookie(cookie_header, "session_id");\n        \n        if (g_session_store.terminate_session(session_id)) {\n            content = "{\\"status\\":\\"Session terminated\\"}";\n            \n            response = MHD_create_response_from_buffer(content.length(),\n                                                       (void*)content.c_str(),\n                                                       MHD_RESPMEM_MUST_COPY);\n            \n            // Clear cookie (Rules#1)\n            std::string cookie = "session_id=; HttpOnly; Secure; SameSite=Strict; Max-Age=0";\n            MHD_add_response_header(response, "Set-Cookie", cookie.c_str());\n            MHD_add_response_header(response, "Content-Type", "application/json");\n            \n            ret = MHD_queue_response(connection, MHD_HTTP_OK, response);\n            MHD_destroy_response(response);\n            return ret;\n        } else {\n            content = "{\\"error\\":\\"No session to terminate\\"}";\n            status_code = MHD_HTTP_BAD_REQUEST;\n        }\n    }\n    else {\n        content = "{\\"error\\":\\"Not found\\"}";\n        status_code = MHD_HTTP_NOT_FOUND;\n    }\n    \nsend_response:\n    response = MHD_create_response_from_buffer(content.length(),\n                                               (void*)content.c_str(),\n                                               MHD_RESPMEM_MUST_COPY);\n    MHD_add_response_header(response, "Content-Type", "application/json");\n    ret = MHD_queue_response(connection, status_code, response);\n    MHD_destroy_response(response);\n    \n    return ret;\n}\n\nint main() {\n    // Initialize OpenSSL\n    OpenSSL_add_all_algorithms();\n    \n    // Start HTTP server on port 8080\n    struct MHD_Daemon* daemon = MHD_start_daemon(\n        MHD_USE_THREAD_PER_CONNECTION,\n        8080,\n        nullptr, nullptr,\n        &handle_request, nullptr,\n        MHD_OPTION_END\n    );\n    \n    if (!daemon) {\n        return 1;\n    }\n    \n    // Test cases\n    printf("Session Management Server running on port 8080\\n");\n    printf("\\nTest with curl:\\n");\n    printf("1. Create session: curl -X POST 'http://localhost:8080/create?user=testuser' -c cookies.txt\
");\n    printf("2. Check session: curl -X GET http://localhost:8080/check -b cookies.txt\
");\n    printf("3. Logout: curl -X POST http://localhost:8080/logout -b cookies.txt\
");\n    printf("4. Check again: curl -X GET http://localhost:8080/check -b cookies.txt\
");\n    printf("5. Invalid route: curl -X GET http://localhost:8080/invalid\
");\n    \n    printf("\
Press Enter to stop server...\
");
    getchar();
    
    MHD_stop_daemon(daemon);
    EVP_cleanup();
    
    return 0;
}
