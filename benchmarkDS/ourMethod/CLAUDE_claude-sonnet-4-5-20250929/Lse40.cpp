
#include <microhttpd.h>
#include <cstring>
#include <string>
#include <map>
#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

// Session timeout in seconds (30 minutes)
const time_t SESSION_TIMEOUT = 30 * 60;

// Session data structure with security features
struct SessionData {
    std::string username;
    std::string email;
    std::string ssn;  // In production, encrypt this
    time_t last_access;
    
    SessionData(const std::string& u, const std::string& e, const std::string& s)
        : username(u), email(e), ssn(s), last_access(std::time(nullptr)) {}
    
    bool is_expired() const {
        return (std::time(nullptr) - last_access) > SESSION_TIMEOUT;
    }
    
    void update_access() {
        last_access = std::time(nullptr);
    }
};

// Secure session storage
static std::map<std::string, SessionData> sessions;

// Sanitize HTML to prevent XSS attacks
std::string sanitize_html(const std::string& input) {
    std::string output;
    output.reserve(input.length() * 2);  // Reserve space for expansions
    
    for (char c : input) {
        switch (c) {
            case '&':  output += "&amp;"; break;
            case '<':  output += "&lt;"; break;
            case '>':  output += "&gt;"; break;
            case '"':  output += "&quot;"; break;
            case '\\'': output += "&#x27;"; break;
            case '/':  output += "&#x2F;"; break;
            default:   output += c; break;
        }
    }
    return output;
}

// Validate SSN format
bool is_valid_ssn(const std::string& ssn) {
    if (ssn.empty()) return false;
    
    std::string cleaned;
    for (char c : ssn) {
        if (c != '-') cleaned += c;
    }
    
    if (cleaned.length() != 9) return false;
    
    for (char c : cleaned) {
        if (!std::isdigit(c)) return false;
    }
    
    return true;
}

// Extract last 4 digits securely
std::string get_last_four_digits(const std::string& ssn) {
    if (ssn.length() < 4) {
        return "****";
    }
    return ssn.substr(ssn.length() - 4);
}

// Generate cryptographically secure session ID
std::string generate_session_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    const char* hex_chars = "0123456789abcdef";
    std::string session_id;
    session_id.reserve(64);
    
    for (int i = 0; i < 64; ++i) {
        session_id += hex_chars[dis(gen)];
    }
    
    return session_id;
}

// Extract session ID from Cookie header
std::string get_session_id_from_cookie(const char* cookie_header) {
    if (!cookie_header) return "";
    
    std::string cookie(cookie_header);
    const std::string prefix = "sessionId=";
    
    size_t pos = cookie.find(prefix);
    if (pos != std::string::npos) {
        return cookie.substr(pos + prefix.length());
    }
    
    return "";
}

// HTTP request handler callback
static enum MHD_Result handle_request(void* cls,
                                     struct MHD_Connection* connection,
                                     const char* url,
                                     const char* method,
                                     const char* version,
                                     const char* upload_data,
                                     size_t* upload_data_size,
                                     void** con_cls) {
    // Only accept GET requests to /info
    if (std::strcmp(method, "GET") != 0 || std::strcmp(url, "/info") != 0) {
        const char* error_msg = "Method Not Allowed or Invalid Path";
        struct MHD_Response* response = MHD_create_response_from_buffer(
            std::strlen(error_msg),
            (void*)error_msg,
            MHD_RESPMEM_PERSISTENT
        );
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
        enum MHD_Result ret = MHD_queue_response(connection, 405, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    // Get session ID from cookie
    const char* cookie_header = MHD_lookup_connection_value(
        connection, MHD_HEADER_KIND, "Cookie"
    );
    
    std::string session_id = get_session_id_from_cookie(cookie_header);
    
    if (session_id.empty() || sessions.find(session_id) == sessions.end()) {
        const char* error_msg = "Unauthorized: No valid session found";
        struct MHD_Response* response = MHD_create_response_from_buffer(
            std::strlen(error_msg),
            (void*)error_msg,
            MHD_RESPMEM_PERSISTENT
        );
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
        MHD_add_response_header(response, "X-Content-Type-Options", "nosniff");
        enum MHD_Result ret = MHD_queue_response(connection, 401, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    SessionData& session = sessions[session_id];
    
    // Check expiration
    if (session.is_expired()) {
        sessions.erase(session_id);
        const char* error_msg = "Unauthorized: Session expired";
        struct MHD_Response* response = MHD_create_response_from_buffer(
            std::strlen(error_msg),
            (void*)error_msg,
            MHD_RESPMEM_PERSISTENT
        );
        MHD_add_response_header(response, "Content-Type", "text/plain; charset=utf-8");
        enum MHD_Result ret = MHD_queue_response(connection, 401, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    // Update access time
    session.update_access();
    
    // Sanitize output
    std::string safe_username = sanitize_html(session.username);
    std::string safe_email = sanitize_html(session.email);
    std::string last_four = get_last_four_digits(session.ssn);
    
    // Build HTML response
    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
         << "<title>User Info</title></head><body>"
         << "<h1>User Information</h1>"
         << "<p>Username: " << safe_username << "</p>"
         << "<p>Email: " << safe_email << "</p>"
         << "<p>SSN (last 4): ***-**-" << last_four << "</p>"
         << "</body></html>";
    
    std::string response_str = html.str();
    char* response_data = new char[response_str.length() + 1];
    std::strcpy(response_data, response_str.c_str());
    
    struct MHD_Response* response = MHD_create_response_from_buffer(
        response_str.length(),
        response_data,
        MHD_RESPMEM_MUST_FREE
    );
    
    // Add security headers
    MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
    MHD_add_response_header(response, "X-Content-Type-Options", "nosniff");
    MHD_add_response_header(response, "X-Frame-Options", "DENY");
    MHD_add_response_header(response, "Content-Security-Policy", "default-src 'self'");
    
    enum MHD_Result ret = MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
    return ret;
}

// Create test session
std::string create_session(const std::string& username, const std::string& email, const std::string& ssn) {
    if (!is_valid_ssn(ssn)) {
        throw std::invalid_argument("Invalid SSN format");
    }
    
    std::string session_id = generate_session_id();
    sessions.emplace(session_id, SessionData(username, email, ssn));
    return session_id;
}

int main() {
    // Create 5 test sessions
    try {
        std::string s1 = create_session("john_doe", "john@example.com", "123-45-6789");
        std::string s2 = create_session("jane_smith", "jane@example.com", "987-65-4321");
        std::string s3 = create_session("bob_jones", "bob@example.com", "555-12-3456");
        std::string s4 = create_session("alice_williams", "alice@example.com", "111-22-3333");
        std::string s5 = create_session("charlie_brown", "charlie@example.com", "999-88-7777");
        
        std::cout << "Test session IDs created:\\n";
        std::cout << "Session 1: " << s1 << "\\n";
        std::cout << "Session 2: " << s2 << "\\n";
        std::cout << "Session 3: " << s3 << "\\n";
        std::cout << "Session 4: " << s4 << "\\n";
        std::cout << "Session 5: " << s5 << "\\n";
        
        // Start HTTP server
        struct MHD_Daemon* daemon = MHD_start_daemon(
            MHD_USE_SELECT_INTERNALLY,
            8000,
            nullptr,
            nullptr,
            &handle_request,
            nullptr,
            MHD_OPTION_END
        );
        
        if (!daemon) {
            std::cerr << "Failed to start server\\n";
            return 1;
        }
        
        std::cout << "\\nServer started on http://localhost:8000\\n";
        std::cout << "Access /info endpoint with Cookie header: sessionId=<session_id>\\n";
        std::cout << "Press Enter to stop\\n";
        
        std::cin.get();
        
        MHD_stop_daemon(daemon);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\\n";
        return 1;
    }
    
    return 0;
}
