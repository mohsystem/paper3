
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

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
#include <openssl/crypto.h>

#define MAX_USERS 100
#define MAX_SESSIONS 100
#define MAX_LOGIN_ATTEMPTS_LIMIT 5
#define SESSION_TIMEOUT 1800
#define RATE_LIMIT_WINDOW 300
#define BUFFER_SIZE 4096

typedef struct {
    char username[51];
    unsigned char passwordHash[32];
    unsigned char salt[16];
    int active;
} UserData;

typedef struct {
    char token[65];
    char username[51];
    time_t expiry;
    int active;
} SessionData;

typedef struct {
    char username[51];
    int attempts;
    time_t lastAttempt;
    int active;
} LoginAttemptData;

UserData users[MAX_USERS];
SessionData sessions[MAX_SESSIONS];
LoginAttemptData loginAttempts[MAX_USERS];

void generateRandomBytes(unsigned char* buffer, size_t length) {
    if (RAND_bytes(buffer, (int)length) != 1) {
        fprintf(stderr, "Failed to generate random bytes\\n");
        exit(1);
    }
}

void hashPassword(const char* password, const unsigned char* salt, unsigned char* hash) {
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, 16, 
                          100000, EVP_sha256(), 32, hash) != 1) {
        fprintf(stderr, "Password hashing failed\\n");
        exit(1);
    }
}

void createUser(const char* username, const char* password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (!users[i].active) {
            strncpy(users[i].username, username, 50);
            users[i].username[50] = '\\0';
            generateRandomBytes(users[i].salt, 16);
            hashPassword(password, users[i].salt, users[i].passwordHash);
            users[i].active = 1;
            return;
        }
    }
}

int verifyPassword(const char* username, const char* password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].active && strcmp(users[i].username, username) == 0) {
            unsigned char hash[32];
            hashPassword(password, users[i].salt, hash);
            return CRYPTO_memcmp(hash, users[i].passwordHash, 32) == 0;
        }
    }
    return 0;
}

void generateSessionToken(char* token) {
    unsigned char randomBytes[32];
    generateRandomBytes(randomBytes, 32);
    
    for (int i = 0; i < 32; i++) {
        sprintf(&token[i * 2], "%02x", randomBytes[i]);
    }
    token[64] = '\\0';
}

const char* validateSession(const char* token) {
    if (token == NULL || token[0] == '\\0') {
        return NULL;
    }
    
    time_t now = time(NULL);
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && strcmp(sessions[i].token, token) == 0) {
            if (now > sessions[i].expiry) {
                sessions[i].active = 0;
                return NULL;
            }
            return sessions[i].username;
        }
    }
    return NULL;
}

int isRateLimited(const char* username) {
    time_t now = time(NULL);
    for (int i = 0; i < MAX_USERS; i++) {
        if (loginAttempts[i].active && 
            strcmp(loginAttempts[i].username, username) == 0) {
            time_t timeSince = now - loginAttempts[i].lastAttempt;
            if (timeSince > RATE_LIMIT_WINDOW) {
                loginAttempts[i].active = 0;
                return 0;
            }
            return loginAttempts[i].attempts >= MAX_LOGIN_ATTEMPTS_LIMIT;
        }
    }
    return 0;
}

void recordLoginAttempt(const char* username, int success) {
    if (success) {
        for (int i = 0; i < MAX_USERS; i++) {
            if (loginAttempts[i].active && 
                strcmp(loginAttempts[i].username, username) == 0) {
                loginAttempts[i].active = 0;
                return;
            }
        }
    } else {
        for (int i = 0; i < MAX_USERS; i++) {
            if (loginAttempts[i].active && 
                strcmp(loginAttempts[i].username, username) == 0) {
                loginAttempts[i].attempts++;
                loginAttempts[i].lastAttempt = time(NULL);
                return;
            }
        }
        for (int i = 0; i < MAX_USERS; i++) {
            if (!loginAttempts[i].active) {
                strncpy(loginAttempts[i].username, username, 50);
                loginAttempts[i].username[50] = '\\0';
                loginAttempts[i].attempts = 1;
                loginAttempts[i].lastAttempt = time(NULL);
                loginAttempts[i].active = 1;
                return;
            }
        }
    }
}

void escapeHtml(const char* input, char* output, size_t outputSize) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < outputSize - 7; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&output[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&output[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&output[j], "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(&output[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&output[j], "&#x27;");
                j += 6;
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\\0';
}

void urlDecode(const char* src, char* dest, size_t destSize) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\\0' && j < destSize - 1; i++) {
        if (src[i] == '%' && isxdigit(src[i+1]) && isxdigit(src[i+2])) {
            char hex[3] = {src[i+1], src[i+2], '\\0'};
            dest[j++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else if (src[i] == '+') {
            dest[j++] = ' ';
        } else {
            dest[j++] = src[i];
        }
    }
    dest[j] = '\\0';
}

void parseFormData(const char* data, char params[][2][256], int* paramCount) {
    *paramCount = 0;
    char dataCopy[BUFFER_SIZE];
    strncpy(dataCopy, data, BUFFER_SIZE - 1);
    dataCopy[BUFFER_SIZE - 1] = '\\0';
    
    char* pair = strtok(dataCopy, "&");
    while (pair != NULL && *paramCount < 10) {
        char* equals = strchr(pair, '=');
        if (equals != NULL) {
            *equals = '\\0';
            urlDecode(pair, params[*paramCount][0], 256);
            urlDecode(equals + 1, params[*paramCount][1], 256);
            (*paramCount)++;
        }
        pair = strtok(NULL, "&");
    }
}

void getCookie(const char* cookies, const char* name, char* value, size_t valueSize) {
    value[0] = '\\0';
    if (cookies == NULL) return;
    
    char* pos = strstr(cookies, name);
    if (pos == NULL) return;
    
    pos += strlen(name) + 1;
    size_t i = 0;
    while (pos[i] != '\\0' && pos[i] != ';' && i < valueSize - 1) {
        value[i] = pos[i];
        i++;
    }
    value[i] = '\\0';
}

void sendResponse(SOCKET clientSocket, int statusCode, const char* statusText,
                  const char* body, const char* extraHeaders) {
    char response[8192];
    int len = snprintf(response, sizeof(response),
                      "HTTP/1.1 %d %s\\r\\n"
                      "Content-Type: text/html; charset=utf-8\\r\\n"
                      "Content-Length: %zu\\r\\n"
                      "X-Content-Type-Options: nosniff\\r\\n"
                      "X-Frame-Options: DENY\\r\\n"
                      "Connection: close\\r\\n"
                      "%s"
                      "\\r\\n"
                      "%s",
                      statusCode, statusText, strlen(body),
                      extraHeaders ? extraHeaders : "", body);
    
    if (len > 0 && len < (int)sizeof(response)) {
        send(clientSocket, response, len, 0);
    }
}

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        closesocket(clientSocket);
        return;
    }
    
    char method[16], path[256], version[16];
    sscanf(buffer, "%15s %255s %15s", method, path, version);
    
    char* cookieHeader = strstr(buffer, "Cookie:");
    char cookies[512] = "";
    if (cookieHeader != NULL) {
        sscanf(cookieHeader + 7, "%511[^\\r\\n]", cookies);
    }
    
    int contentLength = 0;
    char* contentLengthHeader = strstr(buffer, "Content-Length:");
    if (contentLengthHeader != NULL) {
        sscanf(contentLengthHeader + 15, "%d", &contentLength);
    }
    
    if (strcmp(path, "/") == 0 && strcmp(method, "GET") == 0) {
        const char* body = "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
                          "<title>Index</title></head><body>"
                          "<h1>Welcome</h1><p><a href=\\"/login\\">Login</a></p>"
                          "</body></html>";
        sendResponse(clientSocket, 200, "OK", body, NULL);
    }
    else if (strcmp(path, "/login") == 0 && strcmp(method, "GET") == 0) {
        const char* body = "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
                          "<title>Login</title></head><body><h1>Login</h1>"
                          "<form method=\\"POST\\" action=\\"/login\\">"
                          "<label>Username: <input type=\\"text\\" name=\\"username\\" "
                          "required maxlength=\\"50\\" pattern=\\"[a-zA-Z0-9_]+\\"></label><br>"
                          "<label>Password: <input type=\\"password\\" name=\\"password\\" "
                          "required maxlength=\\"100\\"></label><br>"
                          "<button type=\\"submit\\">Login</button></form></body></html>";
        sendResponse(clientSocket, 200, "OK", body, NULL);
    }
    else if (strcmp(path, "/login") == 0 && strcmp(method, "POST") == 0) {
        if (contentLength > BUFFER_SIZE || contentLength <= 0) {
            sendResponse(clientSocket, 413, "Request Too Large",
                        "<h1>Request Too Large</h1>", NULL);
            closesocket(clientSocket);
            return;
        }
        
        char* postData = strstr(buffer, "\\r\\n\\r\\n");
        if (postData == NULL) {
            closesocket(clientSocket);
            return;
        }
        postData += 4;
        
        char params[10][2][256];
        int paramCount;
        parseFormData(postData, params, &paramCount);
        
        char username[256] = "";
        char password[256] = "";
        
        for (int i = 0; i < paramCount; i++) {
            if (strcmp(params[i][0], "username") == 0) {
                strncpy(username, params[i][1], 255);
                username[255] = '\\0';
            } else if (strcmp(params[i][0], "password") == 0) {
                strncpy(password, params[i][1], 255);
                password[255] = '\\0';
            }
        }
        
        if (username[0] == '\\0' || password[0] == '\\0' ||
            strlen(username) > 50 || strlen(password) > 100) {
            sendResponse(clientSocket, 401, "Unauthorized",
                        "<h1>Invalid input</h1><a href=\\"/login\\">Try again</a>", NULL);
            closesocket(clientSocket);
            return;
        }
        
        int validUsername = 1;
        for (size_t i = 0; username[i] != '\\0'; i++) {
            if (!isalnum(username[i]) && username[i] != '_') {
                validUsername = 0;
                break;
            }
        }
        
        if (!validUsername) {
            sendResponse(clientSocket, 401, "Unauthorized",
                        "<h1>Invalid username format</h1><a href=\\"/login\\">Try again</a>",
                        NULL);
            closesocket(clientSocket);
            return;
        }
        
        if (isRateLimited(username)) {
            sendResponse(clientSocket, 429, "Too Many Requests",
                        "<h1>Too many attempts. Try again later.</h1>", NULL);
            closesocket(clientSocket);
            return;
        }
        
        int valid = verifyPassword(username, password);
        recordLoginAttempt(username, valid);
        
        if (valid) {
            char token[65];
            generateSessionToken(token);
            
            for (int i = 0; i < MAX_SESSIONS; i++) {
                if (!sessions[i].active) {
                    strncpy(sessions[i].token, token, 64);
                    sessions[i].token[64] = '\\0';
                    strncpy(sessions[i].username, username, 50);
                    sessions[i].username[50] = '\\0';
                    sessions[i].expiry = time(NULL) + SESSION_TIMEOUT;
                    sessions[i].active = 1;
                    break;
                }
            }
            
            char headers[512];
            snprintf(headers, sizeof(headers),
                    "Set-Cookie: session=%s; HttpOnly; Secure; SameSite=Strict; "
                    "Max-Age=%d; Path=/\\r\\nLocation: /secret\\r\\n",
                    token, SESSION_TIMEOUT);
            sendResponse(clientSocket, 302, "Found", "", headers);
        } else {
            sendResponse(clientSocket, 401, "Unauthorized",
                        "<h1>Invalid credentials</h1><a href=\\"/login\\">Try again</a>",
                        NULL);
        }
    }
    else if (strcmp(path, "/secret") == 0 && strcmp(method, "GET") == 0) {
        char token[256];
        getCookie(cookies, "session", token, sizeof(token));
        const char* username = validateSession(token);
        
        if (username == NULL) {
            sendResponse(clientSocket, 302, "Found", "", "Location: /login\\r\\n");
        } else {
            char safeUsername[512];
            escapeHtml(username, safeUsername, sizeof(safeUsername));
            
            char body[1024];
            snprintf(body, sizeof(body),
                    "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
                    "<title>Secret</title></head><body>"
                    "<h1>Secret settings</h1>"
                    "<p>Welcome, %s!</p>"
                    "<p><a href=\\"/\\">Home</a></p></body></html>",
                    safeUsername);
            sendResponse(clientSocket, 200, "OK", body, "Cache-Control: no-store\\r\\n");
        }
    }
    else {
        sendResponse(', type='text')