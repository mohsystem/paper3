
#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <random>
#include <iomanip>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Session data structure
struct SessionData {
    std::string username;
    time_t expiry;
    
    bool isExpired() const {
        return time(nullptr) > expiry;
    }
};

// User credentials structure
struct UserCredentials {
    unsigned char passwordHash[32];
    unsigned char salt[16];
};

// Login attempt tracking
struct LoginAttempt {
    int attempts;
    time_t lastAttempt;
};

// Global storage
std::map<std::string, SessionData> sessions;
std::map<std::string, UserCredentials> users;
std::map<std::string, LoginAttempt> loginAttempts;

const int SESSION_TIMEOUT = 1800; // 30 minutes
const int MAX_LOGIN_ATTEMPTS = 5;
const int RATE_LIMIT_WINDOW = 300; // 5 minutes

// Generate random bytes using OpenSSL CSPRNG
void generateRandomBytes(unsigned char* buffer, size_t length) {
    if (RAND_bytes(buffer, static_cast<int>(length)) != 1) {
        throw std::runtime_error("Failed to generate random bytes");
    }
}

// Hash password with salt using PBKDF2
void hashPassword(const std::string& password, const unsigned char* salt, 
                  unsigned char* hash) {
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()),
                          salt, 16, 100000, EVP_sha256(), 32, hash) != 1) {
        throw std::runtime_error("Password hashing failed");
    }
}

// Create user with secure password hash
void createUser(const std::string& username, const std::string& password) {
    UserCredentials creds;
    generateRandomBytes(creds.salt, 16);
    hashPassword(password, creds.salt, creds.passwordHash);
    users[username] = creds;
}

// Verify password with constant-time comparison
bool verifyPassword(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }
    
    unsigned char hash[32];
    hashPassword(password, it->second.salt, hash);
    
    // Constant-time comparison using OpenSSL
    return CRYPTO_memcmp(hash, it->second.passwordHash, 32) == 0;
}

// Generate secure session token
std::string generateSessionToken() {
    unsigned char token[32];
    generateRandomBytes(token, 32);
    
    std::ostringstream oss;
    for (size_t i = 0; i < 32; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') 
            << static_cast<int>(token[i]);
    }
    return oss.str();
}

// Validate session token
std::string validateSession(const std::string& token) {
    if (token.empty()) {
        return "";
    }
    
    auto it = sessions.find(token);
    if (it == sessions.end() || it->second.isExpired()) {
        if (it != sessions.end()) {
            sessions.erase(it);
        }
        return "";
    }
    
    return it->second.username;
}

// Check rate limiting
bool isRateLimited(const std::string& username) {
    auto it = loginAttempts.find(username);
    if (it == loginAttempts.end()) {
        return false;
    }
    
    time_t timeSince = time(nullptr) - it->second.lastAttempt;
    if (timeSince > RATE_LIMIT_WINDOW) {
        loginAttempts.erase(it);
        return false;
    }
    
    return it->second.attempts >= MAX_LOGIN_ATTEMPTS;
}

// Record login attempt
void recordLoginAttempt(const std::string& username, bool success) {
    if (success) {
        loginAttempts.erase(username);
    } else {
        auto it = loginAttempts.find(username);
        if (it != loginAttempts.end()) {
            it->second.attempts++;
            it->second.lastAttempt = time(nullptr);
        } else {
            LoginAttempt attempt;
            attempt.attempts = 1;
            attempt.lastAttempt = time(nullptr);
            loginAttempts[username] = attempt;
        }
    }
}

// HTML escape to prevent XSS
std::string escapeHtml(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 2);
    
    for (char c : input) {
        switch (c) {
            case '&': output += "&amp;"; break;
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '"': output += "&quot;"; break;
            case '\\'': output += "&#x27;"; break;
            default: output += c; break;
        }
    }
    
    return output;
}

// URL decode
std::string urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            int value = 0;
            std::istringstream iss(str.substr(i + 1, 2));
            iss >> std::hex >> value;
            result += static_cast<char>(value);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    
    return result;
}

// Parse form data
std::map<std::string, std::string> parseFormData(const std::string& data) {
    std::map<std::string, std::string> params;
    std::istringstream stream(data);
    std::string pair;
    
    while (std::getline(stream, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, pos));
            std::string value = urlDecode(pair.substr(pos + 1));
            params[key] = value;
        }
    }
    
    return params;
}

// Extract cookie value
std::string getCookie(const std::string& cookies, const std::string& name) {
    size_t pos = cookies.find(name + "=");
    if (pos == std::string::npos) {
        return "";
    }
    
    size_t start = pos + name.length() + 1;
    size_t end = cookies.find(';', start);
    if (end == std::string::npos) {
        end = cookies.length();
    }
    
    return cookies.substr(start, end - start);
}

// Send HTTP response
void sendResponse(SOCKET clientSocket, int statusCode, 
                  const std::string& statusText, const std::string& body,
                  const std::string& extraHeaders = "") {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\\r\\n";
    response << "Content-Type: text/html; charset=utf-8\\r\\n";
    response << "Content-Length: " << body.length() << "\\r\\n";
    response << "X-Content-Type-Options: nosniff\\r\\n";
    response << "X-Frame-Options: DENY\\r\\n";
    response << "Connection: close\\r\\n";
    if (!extraHeaders.empty()) {
        response << extraHeaders;
    }
    response << "\\r\\n";
    response << body;
    
    std::string responseStr = response.str();
    send(clientSocket, responseStr.c_str(), static_cast<int>(responseStr.length()), 0);
}

// Handle client request
void handleClient(SOCKET clientSocket) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        closesocket(clientSocket);
        return;
    }
    
    std::string request(buffer, bytesRead);
    std::istringstream requestStream(request);
    std::string method, path, version;
    requestStream >> method >> path >> version;
    
    // Parse headers
    std::string line;
    std::string cookies;
    int contentLength = 0;
    
    while (std::getline(requestStream, line) && line != "\\r") {
        if (line.find("Cookie:") == 0) {
            cookies = line.substr(8);
        } else if (line.find("Content-Length:") == 0) {
            contentLength = std::stoi(line.substr(16));
        }
    }
    
    if (path == "/" && method == "GET") {
        std::string body = 
            "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
            "<title>Index</title></head><body>"
            "<h1>Welcome</h1><p><a href=\\"/login\\">Login</a></p>"
            "</body></html>";
        sendResponse(clientSocket, 200, "OK", body);
    }
    else if (path == "/login" && method == "GET") {
        std::string body = 
            "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
            "<title>Login</title></head><body><h1>Login</h1>"
            "<form method=\\"POST\\" action=\\"/login\\">"
            "<label>Username: <input type=\\"text\\" name=\\"username\\" "
            "required maxlength=\\"50\\" pattern=\\"[a-zA-Z0-9_]+\\"></label><br>"
            "<label>Password: <input type=\\"password\\" name=\\"password\\" "
            "required maxlength=\\"100\\"></label><br>"
            "<button type=\\"submit\\">Login</button></form></body></html>";
        sendResponse(clientSocket, 200, "OK", body);
    }
    else if (path == "/login" && method == "POST") {
        if (contentLength > 4096 || contentLength <= 0) {
            sendResponse(clientSocket, 413, "Request Too Large", 
                        "<h1>Request Too Large</h1>");
            closesocket(clientSocket);
            return;
        }
        
        std::string postData;
        std::getline(requestStream, postData);
        
        auto params = parseFormData(postData);
        std::string username = params["username"];
        std::string password = params["password"];
        
        // Input validation
        if (username.empty() || password.empty() || 
            username.length() > 50 || password.length() > 100) {
            sendResponse(clientSocket, 401, "Unauthorized", 
                        "<h1>Invalid input</h1><a href=\\"/login\\">Try again</a>");
            closesocket(clientSocket);
            return;
        }
        
        // Validate username format
        bool validUsername = true;
        for (char c : username) {
            if (!std::isalnum(c) && c != '_') {
                validUsername = false;
                break;
            }
        }
        
        if (!validUsername) {
            sendResponse(clientSocket, 401, "Unauthorized", 
                        "<h1>Invalid username format</h1><a href=\\"/login\\">Try again</a>");
            closesocket(clientSocket);
            return;
        }
        
        // Rate limiting
        if (isRateLimited(username)) {
            sendResponse(clientSocket, 429, "Too Many Requests",
                        "<h1>Too many attempts. Try again later.</h1>");
            closesocket(clientSocket);
            return;
        }
        
        // Verify credentials
        bool valid = verifyPassword(username, password);
        recordLoginAttempt(username, valid);
        
        if (valid) {
            std::string token = generateSessionToken();
            SessionData session;
            session.username = username;
            session.expiry = time(nullptr) + SESSION_TIMEOUT;
            sessions[token] = session;
            
            std::string setCookie = "Set-Cookie: session=" + token + 
                                   "; HttpOnly; Secure; SameSite=Strict; " +
                                   "Max-Age=" + std::to_string(SESSION_TIMEOUT) + 
                                   "; Path=/\\r\\nLocation: /secret\\r\\n";
            sendResponse(clientSocket, 302, "Found", "", setCookie);
        } else {
            sendResponse(clientSocket, 401, "Unauthorized",
                        "<h1>Invalid credentials</h1><a href=\\"/login\\">Try again</a>");
        }
    }
    else if (path == "/secret" && method == "GET") {
        std::string token = getCookie(cookies, "session");
        std::string username = validateSession(token);
        
        if (username.empty()) {
            sendResponse(clientSocket, 302, "Found", "", 
                        "Location: /login\\r\\n");
        } else {
            std::string safeUsername = escapeHtml(username);
            std::string body = 
                "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
                "<title>Secret</title></head><body>"
                "<h1>Secret settings</h1>"
                "<p>Welcome, " + safeUsername + "!</p>"
                "<p><a href=\\"/\\">Home</a></p></body></html>";
            sendResponse(clientSocket, 200, "OK", body, 
                        "Cache-Control: no-store\\r\\n");
        }
    }
    else {
        sendResponse(clientSocket, 404, "Not Found", "<h1>404 Not Found</h1>");
    }
    
    closesocket(clientSocket);
}

int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    
    // Create test users
    createUser("admin", "SecureP@ss123!");
    createUser("user1", "TestUser#456");
    createUser("user2", "Demo$Pass789");
    createUser("testuser", "MyTest&Pass000");
    createUser("guest", "Guest!Pass321");
    
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\\n";
        return 1;
    }
#endif
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\\n";
        return 1;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, 
               reinterpret_cast<char*>(&opt), sizeof(opt));
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8000);
    
    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr),
             sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\\n";
        closesocket(serverSocket);
        return 1;
    }
    
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "Listen failed\\n";
        closesocket(serverSocket);
        return 1;
    }
    
    std::cout << "Server started on port 8000\\n";
    std::cout << "Test users created:\\n";
    std::cout << "  admin / SecureP@ss123!\\n";
    std::cout << "  user1 / TestUser#456\\n";
    std::cout << "  user2 / Demo$Pass789\\n";
    std::cout << "  testuser / MyTest&Pass000\\n";
    std::cout << "  guest / Guest!Pass321\\n";
    
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, 
                                     reinterpret_cast<struct sockaddr*>(&clientAddr),
                                     &clientLen);
        
        if (clientSocket != INVALID_SOCKET) {
            handleClient(clientSocket);
        }
    }
    
    closesocket(serverSocket);
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    EVP_cleanup();
    return 0;
}
