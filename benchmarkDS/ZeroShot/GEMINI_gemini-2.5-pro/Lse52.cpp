/*
 * IMPORTANT: This is a single-file example using the header-only Crow library.
 * To compile:
 * 1. Download "crow_all.h" from https://github.com/CrowCpp/Crow
 * 2. Place "crow_all.h" in the same directory as this file.
 * 3. Compile with a C++17 compiler and link against Boost libraries and OpenSSL:
 *    g++ -std=c++17 -o Lse52 Lse52.cpp -lboost_system -lssl -lcrypto -lpthread
 *
 * This example implements security features like password hashing, session management,
 * and HTML escaping from scratch for demonstration purposes.
 */
#include "crow_all.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>
#include <random>
#include <iomanip>
#include <sstream>

// For SHA256 hashing - requires linking -lssl -lcrypto
#include <openssl/sha.h>
#include <openssl/rand.h>

// --- Thread-safe in-memory storage ---
// In a real application, use a database.
std::mutex mtx;

struct User {
    std::string username;
    std::string salt;
    std::string hashedPassword;
};
std::unordered_map<std::string, User> userDatabase;
std::unordered_map<std::string, std::string> sessionStore; // sessionId -> username


// --- Security Helper Functions ---

// Generates a random salt
std::string generate_salt(size_t length = 16) {
    std::vector<unsigned char> buffer(length);
    if (RAND_bytes(buffer.data(), length) != 1) {
        throw std::runtime_error("Failed to generate random bytes for salt.");
    }
    std::stringstream ss;
    for (unsigned char c : buffer) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return ss.str();
}

// Hashes a password with a given salt using SHA-256
std::string hash_password(const std::string& password, const std::string& salt) {
    std::string salted_password = salt + password;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salted_password.c_str(), salted_password.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// Constant-time memory comparison to prevent timing attacks
bool constant_time_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    // Using OpenSSL's constant time compare function is best
    return CRYPTO_memcmp(a.data(), b.data(), a.length()) == 0;
}

// Securely checks credentials
bool do_login_logic(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mtx);
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        // To prevent username enumeration, perform a dummy hash
        hash_password(password, generate_salt());
        return false;
    }

    const User& user = it->second;
    std::string provided_hash = hash_password(password, user.salt);
    return constant_time_compare(user.hashedPassword, provided_hash);
}

// Simple HTML escaping to prevent XSS
std::string html_escape(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&#39;");       break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    return buffer;
}


int main() {
    // --- Setup Users ---
    // In a real app, load from a database.
    {
        std::lock_guard<std::mutex> lock(mtx);
        std::string user_salt = generate_salt();
        userDatabase["admin"] = {"admin", user_salt, hash_password("password123", user_salt)};
    }
    
    // --- 5 Test Cases for Login Logic ---
    std::cout << "--- Running 5 Login Logic Test Cases ---" << std::endl;
    std::cout << std::boolalpha;
    std::cout << "1. Correct credentials ('admin', 'password123'): " << do_login_logic("admin", "password123") << std::endl;
    std::cout << "2. Wrong password ('admin', 'wrongpass'): " << do_login_logic("admin", "wrongpass") << std::endl;
    std::cout << "3. Wrong username ('user', 'password123'): " << do_login_logic("user", "password123") << std::endl;
    std::cout << "4. Empty username ('', 'password123'): " << do_login_logic("", "password123") << std::endl;
    std::cout << "5. Empty password ('admin', ''): " << do_login_logic("admin", "") << std::endl;
    std::cout << "----------------------------------------\n" << std::endl;

    // --- Start Web Server ---
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([](){
        return crow::response("<html><body><h1>Welcome</h1><p><a href='/login'>Login</a></p></body></html>");
    });

    CROW_ROUTE(app, "/login")
    ([](){
        std::string html = R"(
            <html><body>
            <h2>Login</h2>
            <form method='post' action='/do_login'>
              Username: <input type='text' name='username'><br>
              Password: <input type='password' name='password'><br>
              <input type='submit' value='Login'>
            </form>
            </body></html>
        )";
        return crow::response(html);
    });

    CROW_ROUTE(app, "/do_login").methods("POST"_method)
    ([&](const crow::request& req){
        auto params = crow::query_string(req.body);
        std::string username = params.get("username") ? params.get("username") : "";
        std::string password = params.get("password") ? params.get("password") : "";

        if (do_login_logic(username, password)) {
            std::string session_id = generate_salt(32);
            {
                std::lock_guard<std::mutex> lock(mtx);
                sessionStore[session_id] = username;
            }
            auto res = crow::response(302);
            res.add_header("Location", "/secret");
            // Set a secure, HttpOnly cookie. Add "; Secure" if using HTTPS.
            res.add_header("Set-Cookie", "session_id=" + session_id + "; HttpOnly; Path=/");
            return res;
        } else {
            return crow::response(401, "Invalid username or password. <a href='/login'>Try again</a>.");
        }
    });

    CROW_ROUTE(app, "/secret")
    ([&](const crow::request& req){
        auto session_id = req.get_header_value("Cookie");
        // Simple cookie parsing
        if (session_id.find("session_id=") != std::string::npos) {
            session_id = session_id.substr(session_id.find("session_id=") + 11);
        } else {
            session_id = "";
        }
        
        std::string username;
        {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = sessionStore.find(session_id);
            if (it != sessionStore.end()) {
                username = it->second;
            }
        }
        
        if (!username.empty()) {
            std::string body = "<html><body><h1>Secret settings</h1><p>Welcome, " + html_escape(username) + "!</p></body></html>";
            return crow::response(body);
        } else {
            auto res = crow::response(302);
            res.add_header("Location", "/login");
            return res;
        }
    });
    
    std::cout << "Server starting on http://127.0.0.1:18080" << std::endl;
    app.port(18080).run();

    return 0;
}