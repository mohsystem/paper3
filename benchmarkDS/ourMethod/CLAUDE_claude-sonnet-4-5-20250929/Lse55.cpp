
#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>

// User data structure with hashed password and unique salt
struct UserData {
    std::string username;
    unsigned char passwordHash[32];  // SHA-256 output
    unsigned char salt[16];
    bool isAdmin;
    
    UserData() : username(""), isAdmin(false) {
        memset(passwordHash, 0, sizeof(passwordHash));
        memset(salt, 0, sizeof(salt));
    }
};

// Session structure with expiration
struct UserSession {
    std::string username;
    bool isAdmin;
    time_t expiryTime;
    
    UserSession() : username(""), isAdmin(false), expiryTime(0) {}
    
    UserSession(const std::string& user, bool admin) 
        : username(user), isAdmin(admin) {
        // Session expires in 30 minutes
        expiryTime = time(NULL) + (30 * 60);
    }
    
    bool isValid() const {
        return time(NULL) < expiryTime;
    }
};

// Global storage
static std::map<std::string, UserData> users;
static std::map<std::string, UserSession> sessions;

// Generate cryptographically secure random bytes using OpenSSL
bool generateSecureRandom(unsigned char* buffer, size_t length) {
    // Use OpenSSL's CSPRNG per security requirements\n    return RAND_bytes(buffer, static_cast<int>(length)) == 1;\n}\n\n// Hash password using PBKDF2-HMAC-SHA-256 with 210000 iterations\nbool hashPassword(const std::string& password, const unsigned char* salt, \n                  unsigned char* output) {\n    // Use PBKDF2-HMAC-SHA-256 with 210000 iterations per security policy\n    const int iterations = 210000;\n    const int keyLength = 32;  // 256 bits\n    \n    int result = PKCS5_PBKDF2_HMAC(\n        password.c_str(), static_cast<int>(password.length()),\n        salt, 16,\n        iterations,\n        EVP_sha256(),\n        keyLength,\n        output\n    );\n    \n    return result == 1;\n}\n\n// Constant-time comparison to prevent timing attacks\nbool constantTimeCompare(const unsigned char* a, const unsigned char* b, size_t length) {\n    if (a == NULL || b == NULL) {\n        return false;\n    }\n    unsigned char result = 0;\n    for (size_t i = 0; i < length; i++) {\n        result |= a[i] ^ b[i];\n    }\n    return result == 0;\n}\n\n// Generate secure session token\nstd::string generateSessionToken() {\n    unsigned char randomBytes[32];\n    if (!generateSecureRandom(randomBytes, sizeof(randomBytes))) {\n        return "";\n    }\n    \n    // Convert to hex string\n    std::stringstream ss;\n    for (size_t i = 0; i < sizeof(randomBytes); i++) {\n        ss << std::hex << std::setw(2) << std::setfill('0') \n           << static_cast<int>(randomBytes[i]);\n    }\n    return ss.str();\n}\n\n// Create user with unique salt per CWE-759 mitigation\nbool createUser(const std::string& username, const std::string& password, bool isAdmin) {\n    UserData userData;\n    userData.username = username;\n    userData.isAdmin = isAdmin;\n    \n    // Generate unique 16-byte salt using CSPRNG per CWE-759\n    if (!generateSecureRandom(userData.salt, sizeof(userData.salt))) {\n        return false;\n    }\n    \n    // Hash password with salt\n    if (!hashPassword(password, userData.salt, userData.passwordHash)) {\n        return false;\n    }\n    \n    users[username] = userData;\n    return true;\n}\n\n// Initialize test users\nvoid initializeUsers() {\n    createUser("user1", "Password123!", false);\n    createUser("admin1", "AdminPass456!", true);\n    createUser("user2", "SecurePass789!", false);\n    createUser("admin2", "AdminSecure000!", true);\n    createUser("testuser", "TestPass111!", false);\n}\n\n// GET /login - Display login form\nstd::string login(const std::map<std::string, std::string>& params) {\n    return "<html><body>"\n           "<h2>Login</h2>"\n           "<form method='POST' action='/do_login'>"\n           "Username: <input type='text' name='username' required><br>"\n           "Password: <input type='password' name='password' required><br>"\n           "<input type='submit' value='Login'>"\n           "</form></body></html>";\n}\n\n// POST /do_login - Process login credentials\nstd::map<std::string, std::string> doLogin(const std::map<std::string, std::string>& params) {\n    std::map<std::string, std::string> result;\n    \n    // Input validation: check required parameters\n    auto usernameIt = params.find("username");\n    auto passwordIt = params.find("password");\n    \n    if (usernameIt == params.end() || passwordIt == params.end() ||\n        usernameIt->second.empty() || passwordIt->second.empty()) {\n        result["success"] = "false";\n        result["message"] = "Invalid credentials";\n        return result;\n    }\n    \n    std::string username = usernameIt->second;\n    std::string password = passwordIt->second;\n    \n    // Trim username\n    size_t start = username.find_first_not_of(" \\t\\r\\n");\n    size_t end = username.find_last_not_of(" \\t\\r\\n");\n    if (start != std::string::npos && end != std::string::npos) {\n        username = username.substr(start, end - start + 1);\n    }\n    \n    // Validate username: length and format\n    if (username.length() > 50 || username.empty()) {\n        result["success"] = "false";\n        result["message"] = "Invalid credentials";\n        return result;\n    }\n    \n    for (char c : username) {\n        if (!isalnum(c) && c != '_') {\n            result["success"] = "false";\n            result["message"] = "Invalid credentials";\n            return result;\n        }\n    }\n    \n    // Validate password length\n    if (password.length() > 128) {\n        result["success"] = "false";\n        result["message"] = "Invalid credentials";\n        return result;\n    }\n    \n    // Lookup user\n    auto userIt = users.find(username);\n    if (userIt == users.end()) {\n        // Generic error to prevent user enumeration\n        result["success"] = "false";\n        result["message"] = "Invalid credentials";\n        return result;\n    }\n    \n    const UserData& userData = userIt->second;\n    \n    // Hash provided password with user's salt
    unsigned char providedHash[32];
    if (!hashPassword(password, userData.salt, providedHash)) {
        result["success"] = "false";
        result["message"] = "Authentication error";
        return result;
    }
    
    // Verify password using constant-time comparison
    if (!constantTimeCompare(providedHash, userData.passwordHash, 32)) {
        result["success"] = "false";
        result["message"] = "Invalid credentials";
        return result;
    }
    
    // Generate secure session token
    std::string sessionToken = generateSessionToken();
    if (sessionToken.empty()) {
        result["success"] = "false";
        result["message"] = "Authentication error";
        return result;
    }
    
    // Create session
    sessions[sessionToken] = UserSession(userData.username, userData.isAdmin);
    
    result["success"] = "true";
    result["sessionToken"] = sessionToken;
    result["message"] = "Login successful";
    
    return result;
}

// GET /index - Public home page
std::string index(const std::map<std::string, std::string>& params) {
    return "<html><body>"
           "<h1>Welcome to the Application</h1>"
           "<p>This is a public page.</p>"
           "<a href='/login'>Login</a>"
           "</body></html>";
}

// GET /user_page - Authenticated user page
std::map<std::string, std::string> userPage(const std::map<std::string, std::string>& params) {
    std::map<std::string, std::string> result;
    
    // Validate session token
    auto tokenIt = params.find("sessionToken");
    if (tokenIt == params.end() || tokenIt->second.empty()) {
        result["authorized"] = "false";
        result["message"] = "Authentication required";
        return result;
    }
    
    std::string sessionToken = tokenIt->second;
    
    // Check session validity
    auto sessionIt = sessions.find(sessionToken);
    if (sessionIt == sessions.end() || !sessionIt->second.isValid()) {
        // Clean up expired session
        if (sessionIt != sessions.end()) {
            sessions.erase(sessionIt);
        }
        result["authorized"] = "false";
        result["message"] = "Session expired or invalid";
        return result;
    }
    
    const UserSession& session = sessionIt->second;
    result["authorized"] = "true";
    result["username"] = session.username;
    result["content"] = "Welcome to your user page, " + session.username + "!";
    return result;
}

// GET /admin_page - Admin-only page
std::map<std::string, std::string> adminPage(const std::map<std::string, std::string>& params) {
    std::map<std::string, std::string> result;
    
    // Validate session token
    auto tokenIt = params.find("sessionToken");
    if (tokenIt == params.end() || tokenIt->second.empty()) {
        result["authorized"] = "false";
        result["message"] = "Authentication required";
        return result;
    }
    
    std::string sessionToken = tokenIt->second;
    
    // Check session validity
    auto sessionIt = sessions.find(sessionToken);
    if (sessionIt == sessions.end() || !sessionIt->second.isValid()) {
        // Clean up expired session
        if (sessionIt != sessions.end()) {
            sessions.erase(sessionIt);
        }
        result["authorized"] = "false";
        result["message"] = "Session expired or invalid";
        return result;
    }
    
    const UserSession& session = sessionIt->second;
    
    // Check admin privilege
    if (!session.isAdmin) {
        result["authorized"] = "false";
        result["message"] = "Insufficient privileges";
        return result;
    }
    
    result["authorized"] = "true";
    result["username"] = session.username;
    result["content"] = "Welcome to the admin page, " + session.username + "!";
    return result;
}

int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    
    // Initialize test users
    initializeUsers();
    
    std::cout << "=== Test Case 1: Login page (GET) ===" << std::endl;
    std::map<std::string, std::string> emptyParams;
    std::string loginPage = login(emptyParams);
    std::cout << loginPage.substr(0, std::min(size_t(100), loginPage.length())) << "..." << std::endl;
    
    std::cout << "\\n=== Test Case 2: Valid login (POST) ===" << std::endl;
    std::map<std::string, std::string> loginParams;
    loginParams["username"] = "user1";
    loginParams["password"] = "Password123!";
    std::map<std::string, std::string> loginResult = doLogin(loginParams);
    std::cout << "Success: " << loginResult["success"] << std::endl;
    std::string sessionToken = loginResult["sessionToken"];
    std::cout << "Session token: " << (sessionToken.empty() ? "None" : "Generated") << std::endl;
    
    std::cout << "\\n=== Test Case 3: Invalid login ===" << std::endl;
    std::map<std::string, std::string> invalidParams;
    invalidParams["username"] = "user1";
    invalidParams["password"] = "WrongPassword!";
    std::map<std::string, std::string> invalidResult = doLogin(invalidParams);
    std::cout << "Success: " << invalidResult["success"] << std::endl;
    std::cout << "Message: " << invalidResult["message"] << std::endl;
    
    std::cout << "\\n=== Test Case 4: User page access ===" << std::endl;
    if (!sessionToken.empty()) {
        std::map<std::string, std::string> userParams;
        userParams["sessionToken"] = sessionToken;
        std::map<std::string, std::string> userResult = userPage(userParams);
        std::cout << "Authorized: " << userResult["authorized"] << std::endl;
        std::cout << "Content: " << userResult["content"] << std::endl;
    }
    
    std::cout << "\\n=== Test Case 5: Admin page access (user) ===" << std::endl;
    if (!sessionToken.empty()) {
        std::map<std::string, std::string> adminParams;
        adminParams["sessionToken"] = sessionToken;
        std::map<std::string, std::string> adminResult = adminPage(adminParams);
        std::cout << "Authorized: " << adminResult["authorized"] << std::endl;
        std::cout << "Message: " << adminResult["message"] << std::endl;
    }
    
    std::cout << "\\n=== Bonus: Admin login and access ===" << std::endl;
    std::map<std::string, std::string> adminLoginParams;
    adminLoginParams["username"] = "admin1";
    adminLoginParams["password"] = "AdminPass456!";
    std::map<std::string, std::string> adminLoginResult = doLogin(adminLoginParams);
    std::string adminToken = adminLoginResult["sessionToken"];
    if (!adminToken.empty()) {
        std::map<std::string, std::string> adminAccessParams;
        adminAccessParams["sessionToken"] = adminToken;
        std::map<std::string, std::string> adminAccessResult = adminPage(adminAccessParams);
        std::cout << "Admin authorized: " << adminAccessResult["authorized"] << std::endl;
        std::cout << "Admin content: " << adminAccessResult["content"] << std::endl;
    }
    
    // Cleanup OpenSSL
    EVP_cleanup();
    
    return 0;
}
