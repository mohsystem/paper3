
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Security: Use constant-time comparison to prevent timing attacks
bool constant_time_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) return false;
    volatile unsigned char result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Security: Generate cryptographically secure random salt
std::string generate_salt(size_t length = 16) {
    std::vector<unsigned char> salt(length);
    if (RAND_bytes(salt.data(), length) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
    std::string result;
    result.reserve(length * 2);
    for (unsigned char c : salt) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", c);
        result += buf;
    }
    return result;
}

// Security: PBKDF2-HMAC-SHA256 with at least 210000 iterations
std::string pbkdf2_hash(const std::string& password, const std::string& salt_hex) {
    const int iterations = 210000;
    const int key_length = 32;
    
    // Security: Convert hex salt back to bytes
    std::vector<unsigned char> salt;
    for (size_t i = 0; i < salt_hex.length(); i += 2) {
        std::string byte_str = salt_hex.substr(i, 2);
        salt.push_back(static_cast<unsigned char>(strtol(byte_str.c_str(), nullptr, 16)));
    }
    
    std::vector<unsigned char> key(key_length);
    
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt.data(), salt.size(),
                          iterations, EVP_sha256(),
                          key_length, key.data()) != 1) {
        throw std::runtime_error("PBKDF2 key derivation failed");
    }
    
    std::string result;
    result.reserve(key_length * 2);
    for (unsigned char c : key) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", c);
        result += buf;
    }
    
    // Security: Clear sensitive data
    OPENSSL_cleanse(key.data(), key.size());
    return result;
}

// Security: Input validation - username/password constraints
bool validate_username(const std::string& username) {
    if (username.empty() || username.length() > 50) return false;
    // Security: Only allow alphanumeric and underscore
    return std::all_of(username.begin(), username.end(), 
        [](char c) { return std::isalnum(c) || c == '_'; });
}

bool validate_password(const std::string& password) {
    // Security: Password policy - minimum 8 characters
    if (password.length() < 8 || password.length() > 128) return false;
    return true;
}

// Security: Sanitize path to prevent directory traversal
std::string sanitize_path(const std::string& base_dir, const std::string& filename) {
    // Security: Reject paths with traversal attempts
    if (filename.find("..") != std::string::npos ||
        filename.find("/") != std::string::npos ||
        filename.find("\\\\") != std::string::npos) {\n        throw std::runtime_error("Invalid filename");\n    }\n    return base_dir + "/" + filename;\n}\n\nclass SecureServer {\nprivate:\n    int server_fd;\n    std::string users_dir;\n    \n    // Security: Store credentials securely on filesystem with salt and hash\n    bool store_user(const std::string& username, const std::string& password) {\n        try {\n            // Security: Validate inputs\n            if (!validate_username(username) || !validate_password(password)) {\n                return false;\n            }\n            \n            std::string salt = generate_salt(16);\n            std::string hash = pbkdf2_hash(password, salt);\n            \n            // Security: Use sanitized path\n            std::string user_file = sanitize_path(users_dir, username + ".dat");\n            \n            // Security: Write to temp file first, then atomic rename (TOCTOU mitigation)\n            std::string temp_file = user_file + ".tmp";\n            std::ofstream ofs(temp_file, std::ios::binary);\n            if (!ofs) {\n                return false;\n            }\n            \n            ofs << salt << "\
" << hash << "\
";\n            ofs.close();\n            \n            // Security: Atomic rename to prevent TOCTOU\n            if (rename(temp_file.c_str(), user_file.c_str()) != 0) {\n                unlink(temp_file.c_str());\n                return false;\n            }\n            \n            return true;\n        } catch (const std::exception& e) {\n            return false;\n        }\n    }\n    \n    // Security: Authenticate user with constant-time comparison\n    bool authenticate_user(const std::string& username, const std::string& password) {\n        try {\n            // Security: Validate inputs\n            if (!validate_username(username) || !validate_password(password)) {\n                return false;\n            }\n            \n            // Security: Use sanitized path\n            std::string user_file = sanitize_path(users_dir, username + ".dat");\n            \n            std::ifstream ifs(user_file, std::ios::binary);\n            if (!ifs) {\n                return false;\n            }\n            \n            std::string stored_salt, stored_hash;\n            std::getline(ifs, stored_salt);\n            std::getline(ifs, stored_hash);\n            ifs.close();\n            \n            // Security: Validate stored data format\n            if (stored_salt.empty() || stored_hash.empty()) {\n                return false;\n            }\n            \n            std::string computed_hash = pbkdf2_hash(password, stored_salt);\n            \n            // Security: Constant-time comparison to prevent timing attacks\n            return constant_time_compare(computed_hash, stored_hash);\n        } catch (const std::exception& e) {\n            return false;\n        }\n    }\n    \n    // Security: Parse and validate message format\n    bool parse_message(const std::string& msg, std::string& cmd, \n                      std::string& username, std::string& password) {\n        // Security: Input validation - check message length\n        if (msg.length() > 4096) return false;\n        \n        std::istringstream iss(msg);\n        std::string line;\n        \n        // Security: Parse with bounds checking\n        if (!std::getline(iss, line)) return false;\n        if (line.substr(0, 5) != "CMD: ") return false;\n        cmd = line.substr(5);\n        \n        if (!std::getline(iss, line)) return false;\n        if (line.substr(0, 10) != "USERNAME: ") return false;\n        username = line.substr(10);\n        \n        if (!std::getline(iss, line)) return false;\n        if (line.substr(0, 10) != "PASSWORD: ") return false;\n        password = line.substr(10);\n        \n        // Security: Validate extracted fields\n        if (!validate_username(username) || !validate_password(password)) {\n            return false;\n        }\n        \n        return true;\n    }\n    \n    std::string handle_request(const std::string& request) {\n        std::string cmd, username, password;\n        \n        // Security: Parse and validate request\n        if (!parse_message(request, cmd, username, password)) {\n            return "ERROR: Invalid request format\
";\n        }\n        \n        // Security: Process commands with validation\n        if (cmd == "LOGIN") {\n            if (authenticate_user(username, password)) {\n                return "SUCCESS: Login successful\
";\n            } else {\n                return "ERROR: Authentication failed\
";\n            }\n        } else if (cmd == "REGISTER") {\n            if (store_user(username, password)) {\n                return "SUCCESS: Registration successful\
";\n            } else {\n                return "ERROR: Registration failed\
";\n            }\n        } else {\n            return "ERROR: Unknown command\
";\n        }\n    }\n    \npublic:\n    SecureServer(const std::string& dir) : server_fd(-1), users_dir(dir) {}\n    \n    ~SecureServer() {\n        if (server_fd >= 0) {\n            close(server_fd);\n        }\n    }\n    \n    bool start(int port) {\n        // Security: Create socket with error checking\n        server_fd = socket(AF_INET, SOCK_STREAM, 0);\n        if (server_fd < 0) {\n            return false;\n        }\n        \n        int opt = 1;\n        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {\n            close(server_fd);\n            server_fd = -1;\n            return false;\n        }\n        \n        struct sockaddr_in address;\n        std::memset(&address, 0, sizeof(address));\n        address.sin_family = AF_INET;\n        address.sin_addr.s_addr = INADDR_ANY;\n        address.sin_port = htons(port);\n        \n        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {\n            close(server_fd);\n            server_fd = -1;\n            return false;\n        }\n        \n        if (listen(server_fd, 5) < 0) {\n            close(server_fd);\n            server_fd = -1;\n            return false;\n        }\n        \n        return true;\n    }\n    \n    void run() {\n        while (true) {\n            struct sockaddr_in client_addr;\n            socklen_t client_len = sizeof(client_addr);\n            \n            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);\n            if (client_fd < 0) {\n                continue;\n            }\n            \n            // Security: Buffer with bounds checking\n            const size_t buffer_size = 4096;\n            std::vector<char> buffer(buffer_size);\n            \n            ssize_t bytes_read = recv(client_fd, buffer.data(), buffer_size - 1, 0);\n            if (bytes_read > 0) {\n                buffer[bytes_read] = '\\0';\n                std::string request(buffer.data(), bytes_read);\n                std::string response = handle_request(request);\n                send(client_fd, response.c_str(), response.length(), 0);\n            }\n            \n            close(client_fd);\n        }\n    }\n};\n\nclass SecureClient {\nprivate:\n    int sock_fd;\n    \npublic:\n    SecureClient() : sock_fd(-1) {}\n    \n    ~SecureClient() {\n        if (sock_fd >= 0) {\n            close(sock_fd);\n        }\n    }\n    \n    bool connect_to_server(const std::string& host, int port) {\n        sock_fd = socket(AF_INET, SOCK_STREAM, 0);\n        if (sock_fd < 0) {\n            return false;\n        }\n        \n        struct sockaddr_in server_addr;\n        std::memset(&server_addr, 0, sizeof(server_addr));\n        server_addr.sin_family = AF_INET;\n        server_addr.sin_port = htons(port);\n        \n        if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {\n            close(sock_fd);\n            sock_fd = -1;\n            return false;\n        }\n        \n        if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {\n            close(sock_fd);\n            sock_fd = -1;\n            return false;\n        }\n        \n        return true;\n    }\n    \n    std::string send_request(const std::string& cmd, const std::string& username, \n                            const std::string& password) {\n        // Security: Validate inputs before sending\n        if (!validate_username(username) || !validate_password(password)) {\n            return "ERROR: Invalid credentials format\
";\n        }\n        \n        // Security: Construct message with clear format\n        std::ostringstream oss;\n        oss << "CMD: " << cmd << "\
"\n            << "USERNAME: " << username << "\
"\n            << "PASSWORD: " << password << "\
";\n        \n        std::string message = oss.str();\n        \n        if (send(sock_fd, message.c_str(), message.length(), 0) < 0) {\n            return "ERROR: Failed to send request\
";\n        }\n        \n        // Security: Buffer with bounds checking\n        const size_t buffer_size = 4096;\n        std::vector<char> buffer(buffer_size);\n        \n        ssize_t bytes_read = recv(sock_fd, buffer.data(), buffer_size - 1, 0);\n        if (bytes_read <= 0) {\n            return "ERROR: Failed to receive response\
";\n        }\n        \n        buffer[bytes_read] = '\\0';\n        return std::string(buffer.data(), bytes_read);\n    }\n};\n\nint main() {\n    std::cout << "=== Secure Chat System Test ===" << std::endl;\n    \n    // Test case 1: Server setup and client registration\n    std::cout << "\
Test 1: Server initialization and user registration" << std::endl;\n    {\n        SecureServer server("./users_data");\n        if (!server.start(8080)) {\n            std::cerr << "Failed to start server" << std::endl;\n            return 1;\n        }\n        \n        // Run server in background (simplified for testing)\n        std::cout << "Server started on port 8080" << std::endl;\n    }\n    \n    // Test case 2: Client registration\n    std::cout << "\
Test 2: Client registration" << std::endl;\n    {\n        SecureClient client;\n        if (client.connect_to_server("127.0.0.1", 8080)) {\n            std::string response = client.send_request("REGISTER", "testuser1", "SecurePass123!");\n            std::cout << "Registration response: " << response;\n        }\n    }\n    \n    // Test case 3: Client login with valid credentials\n    std::cout << "\
Test 3: Client login (valid credentials)" << std::endl;\n    {\n        SecureClient client;\n        if (client.connect_to_server("127.0.0.1", 8080)) {\n            std::string response = client.send_request("LOGIN", "testuser1", "SecurePass123!");\n            std::cout << "Login response: " << response;\n        }\n    }\n    \n    // Test case 4: Client login with invalid credentials\n    std::cout << "\
Test 4: Client login (invalid credentials)" << std::endl;\n    {\n        SecureClient client;\n        if (client.connect_to_server("127.0.0.1", 8080)) {\n            std::string response = client.send_request("LOGIN", "testuser1", "WrongPassword");\n            std::cout << "Login response: " << response;\n        }\n    }\n    \n    // Test case 5: Input validation test\n    std::cout << "\
Test 5: Input validation (invalid username)" << std::endl;\n    {\n        SecureClient client;\n        if (client.connect_to_server("127.0.0.1", 8080)) {\n            std::string response = client.send_request("LOGIN", "../../../etc/passwd", "test");\n            std::cout << "Validation response: " << response;
        }
    }
    
    return 0;
}
