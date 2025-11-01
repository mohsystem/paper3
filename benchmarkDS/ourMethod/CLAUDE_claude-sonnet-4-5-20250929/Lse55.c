
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define MAX_USERS 10
#define MAX_SESSIONS 100
#define USERNAME_MAX 51
#define PASSWORD_MAX 129
#define SALT_SIZE 16
#define HASH_SIZE 32
#define TOKEN_SIZE 64

/* User data structure with hashed password and unique salt */
typedef struct {
    char username[USERNAME_MAX];
    unsigned char passwordHash[HASH_SIZE];
    unsigned char salt[SALT_SIZE];
    int isAdmin;
    int active;
} UserData;

/* Session structure with expiration */
typedef struct {
    char sessionToken[TOKEN_SIZE + 1];
    char username[USERNAME_MAX];
    int isAdmin;
    time_t expiryTime;
    int active;
} UserSession;

/* Global storage */
static UserData users[MAX_USERS];
static int userCount = 0;
static UserSession sessions[MAX_SESSIONS];
static int sessionCount = 0;

/* Generate cryptographically secure random bytes using OpenSSL */
int generateSecureRandom(unsigned char* buffer, size_t length) {
    /* Use OpenSSL's CSPRNG per security requirements */\n    if (buffer == NULL) {\n        return 0;\n    }\n    return RAND_bytes(buffer, (int)length) == 1;\n}\n\n/* Hash password using PBKDF2-HMAC-SHA-256 with 210000 iterations */\nint hashPassword(const char* password, const unsigned char* salt, unsigned char* output) {\n    /* Input validation */\n    if (password == NULL || salt == NULL || output == NULL) {\n        return 0;\n    }\n    \n    /* Use PBKDF2-HMAC-SHA-256 with 210000 iterations per security policy */\n    const int iterations = 210000;\n    const int keyLength = 32;\n    \n    int result = PKCS5_PBKDF2_HMAC(\n        password, (int)strlen(password),\n        salt, SALT_SIZE,\n        iterations,\n        EVP_sha256(),\n        keyLength,\n        output\n    );\n    \n    return result == 1;\n}\n\n/* Constant-time comparison to prevent timing attacks */\nint constantTimeCompare(const unsigned char* a, const unsigned char* b, size_t length) {\n    if (a == NULL || b == NULL) {\n        return 0;\n    }\n    unsigned char result = 0;\n    size_t i = 0;\n    for (i = 0; i < length; i++) {\n        result |= a[i] ^ b[i];\n    }\n    return result == 0;\n}\n\n/* Generate secure session token */\nint generateSessionToken(char* token, size_t tokenSize) {\n    unsigned char randomBytes[32];\n    size_t i = 0;\n    int written = 0;\n    \n    if (token == NULL || tokenSize < TOKEN_SIZE + 1) {\n        return 0;\n    }\n    \n    if (!generateSecureRandom(randomBytes, sizeof(randomBytes))) {\n        return 0;\n    }\n    \n    /* Convert to hex string with bounds checking */\n    for (i = 0; i < sizeof(randomBytes) && written < (int)tokenSize - 1; i++) {\n        int chars = snprintf(token + written, tokenSize - written, "%02x", randomBytes[i]);\n        if (chars < 0 || chars >= (int)(tokenSize - written)) {\n            return 0;\n        }\n        written += chars;\n    }\n    token[written] = '\\0';\n    \n    return 1;\n}\n\n/* Create user with unique salt per CWE-759 mitigation */\nint createUser(const char* username, const char* password, int isAdmin) {\n    UserData* user = NULL;\n    \n    /* Input validation */\n    if (username == NULL || password == NULL || userCount >= MAX_USERS) {\n        return 0;\n    }\n    \n    /* Validate username length */\n    if (strlen(username) >= USERNAME_MAX) {\n        return 0;\n    }\n    \n    user = &users[userCount];\n    \n    /* Initialize memory */\n    memset(user, 0, sizeof(UserData));\n    \n    /* Copy username with bounds check */\n    strncpy(user->username, username, USERNAME_MAX - 1);\n    user->username[USERNAME_MAX - 1] = '\\0';\n    \n    /* Generate unique 16-byte salt using CSPRNG per CWE-759 */\n    if (!generateSecureRandom(user->salt, SALT_SIZE)) {\n        return 0;\n    }\n    \n    /* Hash password with salt */\n    if (!hashPassword(password, user->salt, user->passwordHash)) {\n        return 0;\n    }\n    \n    user->isAdmin = isAdmin;\n    user->active = 1;\n    userCount++;\n    \n    return 1;\n}\n\n/* Initialize test users */\nvoid initializeUsers(void) {\n    createUser("user1", "Password123!", 0);\n    createUser("admin1", "AdminPass456!", 1);\n    createUser("user2", "SecurePass789!", 0);\n    createUser("admin2", "AdminSecure000!", 1);\n    createUser("testuser", "TestPass111!", 0);\n}\n\n/* Find user by username */\nUserData* findUser(const char* username) {\n    int i = 0;\n    if (username == NULL) {\n        return NULL;\n    }\n    \n    for (i = 0; i < userCount; i++) {\n        if (users[i].active && strcmp(users[i].username, username) == 0) {\n            return &users[i];\n        }\n    }\n    return NULL;\n}\n\n/* Find session by token */\nUserSession* findSession(const char* token) {\n    int i = 0;\n    if (token == NULL) {\n        return NULL;\n    }\n    \n    for (i = 0; i < sessionCount; i++) {\n        if (sessions[i].active && strcmp(sessions[i].sessionToken, token) == 0) {\n            return &sessions[i];\n        }\n    }\n    return NULL;\n}\n\n/* Check if session is valid */\nint isSessionValid(UserSession* session) {\n    if (session == NULL || !session->active) {\n        return 0;\n    }\n    return time(NULL) < session->expiryTime;\n}\n\n/* GET /login - Display login form */\nchar* login(void) {\n    static char response[512];\n    snprintf(response, sizeof(response),\n        "<html><body>"\n        "<h2>Login</h2>"\n        "<form method='POST' action='/do_login'>"\n        "Username: <input type='text' name='username' required><br>"\n        "Password: <input type='password' name='password' required><br>"\n        "<input type='submit' value='Login'>"\n        "</form></body></html>");\n    return response;\n}\n\n/* POST /do_login - Process login credentials */\ntypedef struct {\n    int success;\n    char sessionToken[TOKEN_SIZE + 1];\n    char message[128];\n} LoginResult;\n\nLoginResult doLogin(const char* username, const char* password) {\n    LoginResult result;\n    UserData* userData = NULL;\n    unsigned char providedHash[HASH_SIZE];\n    UserSession* session = NULL;\n    size_t i = 0;\n    int isAlnum = 1;\n    \n    /* Initialize result */\n    memset(&result, 0, sizeof(LoginResult));\n    result.success = 0;\n    strncpy(result.message, "Invalid credentials", sizeof(result.message) - 1);\n    \n    /* Input validation: check required parameters */\n    if (username == NULL || password == NULL || \n        strlen(username) == 0 || strlen(password) == 0) {\n        return result;\n    }\n    \n    /* Validate username: length and format */\n    if (strlen(username) > 50) {\n        return result;\n    }\n    \n    /* Check alphanumeric + underscore */\n    for (i = 0; username[i] != '\\0'; i++) {\n        char c = username[i];\n        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || \n              (c >= '0' && c <= '9') || c == '_')) {\n            isAlnum = 0;\n            break;\n        }\n    }\n    \n    if (!isAlnum) {\n        return result;\n    }\n    \n    /* Validate password length */\n    if (strlen(password) > 128) {\n        return result;\n    }\n    \n    /* Lookup user */\n    userData = findUser(username);\n    if (userData == NULL) {\n        /* Generic error to prevent user enumeration */\n        return result;\n    }\n    \n    /* Hash provided password with user's salt */
    if (!hashPassword(password, userData->salt, providedHash)) {
        strncpy(result.message, "Authentication error", sizeof(result.message) - 1);
        return result;
    }
    
    /* Verify password using constant-time comparison */
    if (!constantTimeCompare(providedHash, userData->passwordHash, HASH_SIZE)) {
        return result;
    }
    
    /* Generate secure session token */
    if (!generateSessionToken(result.sessionToken, sizeof(result.sessionToken))) {
        strncpy(result.message, "Authentication error", sizeof(result.message) - 1);
        return result;
    }
    
    /* Create session */
    if (sessionCount >= MAX_SESSIONS) {
        strncpy(result.message, "Authentication error", sizeof(result.message) - 1);
        return result;
    }
    
    session = &sessions[sessionCount];
    memset(session, 0, sizeof(UserSession));
    strncpy(session->sessionToken, result.sessionToken, TOKEN_SIZE);
    session->sessionToken[TOKEN_SIZE] = '\\0';
    strncpy(session->username, userData->username, USERNAME_MAX - 1);
    session->username[USERNAME_MAX - 1] = '\\0';
    session->isAdmin = userData->isAdmin;
    session->expiryTime = time(NULL) + (30 * 60);  /* 30 minutes */
    session->active = 1;
    sessionCount++;
    
    result.success = 1;
    strncpy(result.message, "Login successful", sizeof(result.message) - 1);
    
    return result;
}

/* GET /index - Public home page */
char* indexPage(void) {
    static char response[256];
    snprintf(response, sizeof(response),
        "<html><body>"
        "<h1>Welcome to the Application</h1>"
        "<p>This is a public page.</p>"
        "<a href='/login'>Login</a>"
        "</body></html>");
    return response;
}

/* GET /user_page - Authenticated user page */
typedef struct {
    int authorized;
    char username[USERNAME_MAX];
    char content[256];
    char message[128];
} PageResult;

PageResult userPage(const char* sessionToken) {
    PageResult result;
    UserSession* session = NULL;
    
    /* Initialize result */
    memset(&result, 0, sizeof(PageResult));
    result.authorized = 0;
    strncpy(result.message, "Authentication required", sizeof(result.message) - 1);
    
    /* Validate session token */
    if (sessionToken == NULL || strlen(sessionToken) == 0) {
        return result;
    }
    
    /* Check session validity */
    session = findSession(sessionToken);
    if (session == NULL || !isSessionValid(session)) {
        /* Clean up expired session */
        if (session != NULL) {
            session->active = 0;
        }
        strncpy(result.message, "Session expired or invalid", sizeof(result.message) - 1);
        return result;
    }
    
    result.authorized = 1;
    strncpy(result.username, session->username, USERNAME_MAX - 1);
    result.username[USERNAME_MAX - 1] = '\\0';
    snprintf(result.content, sizeof(result.content),
        "Welcome to your user page, %s!", session->username);
    
    return result;
}

/* GET /admin_page - Admin-only page */
PageResult adminPage(const char* sessionToken) {
    PageResult result;
    UserSession* session = NULL;
    
    /* Initialize result */
    memset(&result, 0, sizeof(PageResult));
    result.authorized = 0;
    strncpy(result.message, "Authentication required", sizeof(result.message) - 1);
    
    /* Validate session token */
    if (sessionToken == NULL || strlen(sessionToken) == 0) {
        return result;
    }
    
    /* Check session validity */
    session = findSession(sessionToken);
    if (session == NULL || !isSessionValid(session)) {
        /* Clean up expired session */
        if (session != NULL) {
            session->active = 0;
        }
        strncpy(result.message, "Session expired or invalid", sizeof(result.message) - 1);
        return result;
    }
    
    /* Check admin privilege */
    if (!session->isAdmin) {
        strncpy(result.message, "Insufficient privileges", sizeof(result.message) - 1);
        return result;
    }
    
    result.authorized = 1;
    strncpy(result.username, session->username, USERNAME_MAX - 1);
    result.username[USERNAME_MAX - 1] = '\\0';
    snprintf(result.content, sizeof(result.content),
        "Welcome to the admin page, %s!", session->username);
    
    return result;
}

int main(void) {
    char* loginPageResult = NULL;
    LoginResult loginResult;
    LoginResult invalidResult;
    PageResult userResult;
    PageResult adminResult;
    LoginResult adminLoginResult;
    PageResult adminAccessResult;
    
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    
    /* Initialize test users */
    initializeUsers();
    
    printf("=== Test Case 1: Login page (GET) ===\\n");
    loginPageResult = login();
    printf("%.100s...\\n", loginPageResult);
    
    printf("\\n=== Test Case 2: Valid login (POST) ===\\n");
    loginResult = doLogin("user1", "Password123!");
    printf("Success: %d\\n", loginResult.success);
    printf("Session token: %s\\n", loginResult.success ? "Generated" : "None");
    
    printf("\\n=== Test Case 3: Invalid login ===\\n");
    invalidResult = doLogin("user1", "WrongPassword!");
    printf("Success: %d\\n", invalidResult.success);
    printf("Message: %s\\n", invalidResult.message);
    
    printf("\\n=== Test Case 4: User page access ===\\n");
    if (loginResult.success) {
        userResult = userPage(loginResult.sessionToken);
        printf("Authorized: %d\\n", userResult.authorized);
        printf("Content: %s\\n", userResult.content);
    }
    
    printf("\\n=== Test Case 5: Admin page access (user) ===\\n");
    if (loginResult.success) {
        adminResult = adminPage(loginResult.sessionToken);
        printf("Authorized: %d\\n", adminResult.authorized);
        printf("Message: %s\\n", adminResult.message);
    }
    
    printf("\\n=== Bonus: Admin login and access ===\\n");
    adminLoginResult = doLogin("admin1", "AdminPass456!");
    if (adminLoginResult.success) {
        adminAccessResult = adminPage(adminLoginResult.sessionToken);
        printf("Admin authorized: %d\\n", adminAccessResult.authorized);
        printf("Admin content: %s\\n", adminAccessResult.content);
    }
    
    /* Cleanup OpenSSL */
    EVP_cleanup();
    
    return 0;
}
