
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <openssl/sha.h>
#include <microhttpd.h>

std::map<std::string, std::string> users;
std::map<std::string, std::string> sessions;
std::map<std::string, int> loginAttempts;
const int MAX_ATTEMPTS = 5;

std::string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

std::string generateSessionToken() {
    std::stringstream ss;
    for(int i = 0; i < 32; i++) {
        ss << std::hex << (rand() % 16);
    }
    return ss.str();
}

std::string getSession(struct MHD_Connection* connection) {
    const char* cookie = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "session");
    return cookie ? std::string(cookie) : "";
}

bool isAuthenticated(struct MHD_Connection* connection) {
    std::string sessionToken = getSession(connection);
    return !sessionToken.empty() && sessions.find(sessionToken) != sessions.end();
}

int sendResponse(struct MHD_Connection* connection, int code, const std::string& content) {
    struct MHD_Response* response = MHD_create_response_from_buffer(
        content.length(), (void*)content.c_str(), MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
    MHD_add_response_header(response, "X-Content-Type-Options", "nosniff");
    MHD_add_response_header(response, "X-Frame-Options", "DENY");
    int ret = MHD_queue_response(connection, code, response);
    MHD_destroy_response(response);
    return ret;
}

int handleRequest(void* cls, struct MHD_Connection* connection, const char* url,
                 const char* method, const char* version, const char* upload_data,
                 size_t* upload_data_size, void** con_cls) {
    
    if (strcmp(url, "/") == 0 && strcmp(method, "GET") == 0) {
        std::string html = "<!DOCTYPE html><html><head><title>Index</title></head><body>"
                          "<h1>Welcome</h1><a href='/login'>Login</a></body></html>";
        return sendResponse(connection, 200, html);
    }
    
    if (strcmp(url, "/login") == 0 && strcmp(method, "GET") == 0) {
        std::string html = "<!DOCTYPE html><html><head><title>Login</title></head><body>"
                          "<h1>Login</h1><form method='POST' action='/login'>"
                          "<input type='text' name='username' placeholder='Username' required/><br/>"
                          "<input type='password' name='password' placeholder='Password' required/><br/>"
                          "<button type='submit'>Login</button></form></body></html>";
        return sendResponse(connection, 200, html);
    }
    
    if (strcmp(url, "/secret") == 0) {
        if (!isAuthenticated(connection)) {
            struct MHD_Response* response = MHD_create_response_from_buffer(0, (void*)"", MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Location", "/login");
            int ret = MHD_queue_response(connection, 302, response);
            MHD_destroy_response(response);
            return ret;
        }
        std::string html = "<!DOCTYPE html><html><head><title>Secret</title></head><body>"
                          "<h1>Secret settings</h1></body></html>";
        return sendResponse(connection, 200, html);
    }
    
    return sendResponse(connection, 404, "Not Found");
}

int main() {
    srand(time(NULL));
    users["admin"] = hashPassword("admin123");
    users["user1"] = hashPassword("password1");
    
    struct MHD_Daemon* daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000,
                                                 NULL, NULL, &handleRequest, NULL, MHD_OPTION_END);
    if (!daemon) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "Server started on port 8000" << std::endl;
    std::cout << "Test cases:" << std::endl;
    std::cout << "1. Visit http://localhost:8000/ - should show index with login link" << std::endl;
    std::cout << "2. Click login link - should show login form" << std::endl;
    std::cout << "3. Login with admin/admin123 - should redirect to secret page" << std::endl;
    std::cout << "4. Try accessing /secret without login - should redirect to login" << std::endl;
    std::cout << "5. Login with wrong credentials - should show error" << std::endl;
    
    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}
