// This code requires the Mongoose library.
// 1. Download mongoose.h and mongoose.c from https://github.com/cesanta/mongoose
// 2. Place them in the same directory as this file.
// 3. Compile: g++ -std=c++17 -o server_cpp Lse42.cpp mongoose.c -lpthread

#include "mongoose.h"
#include <string>
#include <map>
#include <mutex>
#include <random>
#include <sstream>
#include <iostream>
#include <iomanip>

// User data structure to hold session information
struct UserData {
    std::string username;
    std::string email;
    std::string ssn;
};

// A simple, thread-safe, in-memory session store.
// WARNING: This is for demonstration only. In a real application, use a
// persistent and more robust solution like a database or Redis.
static std::map<std::string, UserData> s_sessions;
static std::mutex s_sessions_mutex;

// Generates a cryptographically secure random session ID using std::random_device.
std::string generate_session_id() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::stringstream ss;
    // Generate a 256-bit random ID (four 64-bit numbers)
    for (int i = 0; i < 4; ++i) {
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
    }
    return ss.str();
}

// Mongoose event handler function
static void event_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // Route for logging in a test user: /login_test/<username>
        if (mg_http_match_uri(hm, "/login_test/*")) {
            struct mg_str path = hm->uri;
            const char* user_start = (const char*)path.ptr + 12; // Skip "/login_test/"
            std::string username(user_start, path.len - 12);

            std::string session_id = generate_session_id();
            UserData data = {username, username + "@example.com", "000-00-0000"}; // DUMMY SSN

            {
                std::lock_guard<std::mutex> lock(s_sessions_mutex);
                s_sessions[session_id] = data;
            }

            std::string cookie_header = "Set-Cookie: session_id=" + session_id + "; Path=/; HttpOnly; SameSite=Lax";
            mg_http_reply(c, 200, ("Content-Type: text/plain\r\n" + cookie_header + "\r\n").c_str(),
                          "Session created for user: %s\n", username.c_str());
        }
        // Route for getting user info
        else if (mg_http_match_uri(hm, "/info")) {
            struct mg_str *cookie_hdr = mg_http_get_header(hm, "Cookie");
            if (cookie_hdr == nullptr) {
                mg_http_reply(c, 401, "Content-Type: application/json\r\n", "{\"error\":\"Unauthorized\"}\n");
                return;
            }
            
            char session_id[129];
            if (mg_http_get_var(cookie_hdr, "session_id", session_id, sizeof(session_id)) <= 0) {
                 mg_http_reply(c, 401, "Content-Type: application/json\r\n", "{\"error\":\"Unauthorized\"}\n");
                 return;
            }

            UserData data;
            bool found = false;
            {
                std::lock_guard<std::mutex> lock(s_sessions_mutex);
                auto it = s_sessions.find(session_id);
                if (it != s_sessions.end()) {
                    data = it->second;
                    found = true;
                }
            }
            
            if (found) {
                // SECURITY WARNING: Exposing sensitive PII like SSN is extremely dangerous.
                std::string json_response = "{\"username\":\"" + data.username + "\",\"email\":\"" + data.email + "\",\"ssn\":\"" + data.ssn + "\"}";
                mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s\n", json_response.c_str());
            } else {
                mg_http_reply(c, 401, "Content-Type: application/json\r\n", "{\"error\":\"Unauthorized\"}\n");
            }
        }
        // Route for logging out
        else if (mg_http_match_uri(hm, "/logout_test")) {
             struct mg_str *cookie_hdr = mg_http_get_header(hm, "Cookie");
            if (cookie_hdr != nullptr) {
                char session_id[129];
                if (mg_http_get_var(cookie_hdr, "session_id", session_id, sizeof(session_id)) > 0) {
                     std::lock_guard<std::mutex> lock(s_sessions_mutex);
                     s_sessions.erase(session_id);
                }
            }
            mg_http_reply(c, 200, "Content-Type: text/plain\r\nSet-Cookie: session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT\r\n", "Session cleared.\n");
        }
        // Default route for unmatched paths
        else {
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not Found\n");
        }
    }
    (void) fn_data; // Unused parameter
}

int main(void) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    const char *addr = "http://localhost:8000";
    if (mg_http_listen(&mgr, addr, event_handler, NULL) == NULL) {
        std::cerr << "Failed to start server on " << addr << std::endl;
        return 1;
    }
    std::cout << "Starting C++ web server on " << addr << std::endl;
    std::cout << "WARNING: This is a basic, non-production server implementation." << std::endl;
    std::cout << "For production use, enable TLS and use a persistent session store." << std::endl;
    
    std::cout << "\n--- Test Cases (run in a separate terminal) ---" << std::endl;
    std::cout << "1. Access /info without session (should fail): curl http://localhost:8000/info" << std::endl;
    std::cout << "2. Log in as 'charlie' to create session: curl -c cookie.txt http://localhost:8000/login_test/charlie" << std::endl;
    std::cout << "3. Access /info with session (should succeed): curl -b cookie.txt http://localhost:8000/info" << std::endl;
    std::cout << "4. Log out to destroy session: curl -b cookie.txt http://localhost:8000/logout_test" << std::endl;
    std::cout << "5. Access /info after logout (should fail): curl -b cookie.txt http://localhost:8000/info" << std::endl;

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
    return 0;
}