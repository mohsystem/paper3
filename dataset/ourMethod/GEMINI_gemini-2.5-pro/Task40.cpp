#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <random>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>

#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Note: This is a simplified, single-threaded, sequential-handling web server
// for demonstration purposes only. It is not robust for production use.
// It lacks proper error handling, timeout management, and concurrency.
// This example is designed to run on a POSIX-compliant system (like Linux or macOS).

const int PORT = 8080;
const int BUFFER_SIZE = 4096;
const int CSRF_TOKEN_BYTES = 32;

struct Session {
    std::string csrf_token;
    std::string email = "user@example.com";
    std::chrono::steady_clock::time_point last_access;
};

// Simple in-memory session store. In a real application, use a proper session manager.
std::map<std::string, Session> session_store;
std::mutex session_mutex;

// Generates a cryptographically secure random token and hex-encodes it.
std::string generate_secure_token(size_t byte_length) {
    std::random_device rd;
    std::vector<unsigned char> buffer(byte_length);
    std::uniform_int_distribution<unsigned int> dist(0, 255);
    for (size_t i = 0; i < byte_length; ++i) {
        buffer[i] = static_cast<unsigned char>(dist(rd));
    }
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : buffer) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

// Compares two strings in a way that resists timing attacks.
bool constant_time_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }

    int result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Simple URL decoding
std::string url_decode(const std::string& str) {
    std::string decoded;
    decoded.reserve(str.length());
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            try {
                std::string hex = str.substr(i + 1, 2);
                decoded += static_cast<char>(std::stoi(hex, nullptr, 16));
                i += 2;
            } catch (const std::exception&) {
                decoded += '%'; // Invalid hex, treat as literal
            }
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

// Simple form data parser
std::map<std::string, std::string> parse_form_data(const std::string& body) {
    std::map<std::string, std::string> data;
    std::stringstream ss(body);
    std::string pair;
    while (std::getline(ss, pair, '&')) {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = url_decode(pair.substr(0, eq_pos));
            std::string value = url_decode(pair.substr(eq_pos + 1));
            data[key] = value;
        }
    }
    return data;
}

void handle_get_settings(int client_socket, Session& session) {
    // Generate a new CSRF token for the form
    session.csrf_token = generate_secure_token(CSRF_TOKEN_BYTES);
    session.last_access = std::chrono::steady_clock::now();

    std::stringstream html_content;
    html_content << "<html><head><title>User Settings</title></head>"
                 << "<body><h1>Update User Settings</h1>"
                 << "<p>Current Email: " << session.email << "</p>"
                 << "<form action=\"/settings\" method=\"POST\">"
                 << "<label for=\"email\">New Email:</label><br>"
                 << "<input type=\"email\" id=\"email\" name=\"email\"><br><br>"
                 << "<input type=\"hidden\" name=\"csrf_token\" value=\"" << session.csrf_token << "\">"
                 << "<input type=\"submit\" value=\"Update\">"
                 << "</form></body></html>";

    std::string content = html_content.str();
    std::stringstream http_response;
    http_response << "HTTP/1.1 200 OK\r\n"
                  << "Content-Type: text/html\r\n"
                  << "Content-Length: " << content.length() << "\r\n"
                  << "\r\n"
                  << content;
    
    send(client_socket, http_response.str().c_str(), http_response.str().length(), 0);
}

void handle_post_settings(int client_socket, Session& session, const std::string& request_body) {
    auto form_data = parse_form_data(request_body);
    
    std::string submitted_token = form_data.count("csrf_token") ? form_data["csrf_token"] : "";
    
    // Security: Verify the CSRF token
    if (session.csrf_token.empty() || !constant_time_compare(session.csrf_token, submitted_token)) {
        std::string response_body = "<html><body><h1>403 Forbidden</h1><p>Invalid CSRF token.</p></body></html>";
        std::stringstream http_response;
        http_response << "HTTP/1.1 403 Forbidden\r\n"
                      << "Content-Type: text/html\r\n"
                      << "Content-Length: " << response_body.length() << "\r\n"
                      << "\r\n"
                      << response_body;
        send(client_socket, http_response.str().c_str(), http_response.str().length(), 0);
        return;
    }
    
    // Invalidate the used token to prevent reuse. A new one will be generated on the next GET.
    session.csrf_token.clear();
    
    if (form_data.count("email")) {
        // In a real app, validate and sanitize this input thoroughly
        session.email = form_data["email"];
    }
    session.last_access = std::chrono::steady_clock::now();
    
    std::string response_body = "<html><body><h1>Settings Updated</h1><p>New email: " + session.email + "</p><a href=\"/settings\">Go back</a></body></html>";
    std::stringstream http_response;
    http_response << "HTTP/1.1 200 OK\r\n"
                  << "Content-Type: text/html\r\n"
                  << "Content-Length: " << response_body.length() << "\r\n"
                  << "\r\n"
                  << response_body;

    send(client_socket, http_response.str().c_str(), http_response.str().length(), 0);
}

void handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    if (recv(client_socket, buffer, BUFFER_SIZE - 1, 0) < 0) {
        perror("recv failed");
        close(client_socket);
        return;
    }

    std::string request(buffer);
    std::stringstream request_stream(request);
    std::string method, path, http_version;
    request_stream >> method >> path >> http_version;
    
    std::string session_id;
    size_t cookie_pos = request.find("Cookie: session_id=");
    if (cookie_pos != std::string::npos) {
        size_t start = cookie_pos + 19; // length of "Cookie: session_id="
        size_t end = request.find(';', start);
        if (end == std::string::npos) end = request.find("\r\n", start);
        if (end != std::string::npos) {
            session_id = request.substr(start, end - start);
        }
    }

    bool new_session = false;
    if (session_id.empty() || session_store.find(session_id) == session_store.end()) {
        session_id = generate_secure_token(16);
        new_session = true;
    }

    std::stringstream response;
    // Set cookie header for new sessions or to keep existing ones alive
    response << "Set-Cookie: session_id=" << session_id << "; HttpOnly; Path=/; SameSite=Lax\r\n";

    {
        std::lock_guard<std::mutex> lock(session_mutex);
        if (new_session) {
             session_store[session_id] = Session{};
        }
        
        Session& current_session = session_store[session_id];

        if (path == "/settings" && method == "GET") {
            handle_get_settings(client_socket, current_session);
        } else if (path == "/settings" && method == "POST") {
            size_t body_pos = request.find("\r\n\r\n");
            if (body_pos != std::string::npos) {
                std::string body = request.substr(body_pos + 4);
                handle_post_settings(client_socket, current_session, body);
            }
        } else {
            std::string response_body = "<html><body><h1>404 Not Found</h1><p>Try <a href=\"/settings\">/settings</a></p></body></html>";
            response << "HTTP/1.1 404 Not Found\r\n"
                     << "Content-Type: text/html\r\n"
                     << "Content-Length: " << response_body.length() << "\r\n"
                     << "\r\n"
                     << response_body;
            send(client_socket, response.str().c_str(), response.str().length(), 0);
        }
    }
    
    close(client_socket);
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        return 1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return 1;
    }

    std::cout << "Server listening on http://localhost:" << PORT << std::endl;
    std::cout << "Access /settings to see the form." << std::endl;
    std::cout << "--- Test Cases ---" << std::endl;
    std::cout << "1. Open a web browser and navigate to http://localhost:" << PORT << "/settings" << std::endl;
    std::cout << "2. Enter a new email and submit the form. You should see a success message." << std::endl;
    std::cout << "3. (Advanced) Use browser developer tools to inspect the form. Note the hidden 'csrf_token' value." << std::endl;
    std::cout << "4. Simulate a CSRF attack with a missing token using curl:" << std::endl;
    std::cout << "   curl -X POST -d 'email=attacker@example.com' --cookie \"session_id=...\" http://localhost:" << PORT << "/settings" << std::endl;
    std::cout << "   (replace 'session_id=...' with the actual cookie from your browser). You should get a 403 Forbidden error." << std::endl;
    std::cout << "5. Simulate a CSRF attack with an incorrect token using curl:" << std::endl;
    std::cout << "   curl -X POST -d 'email=attacker@example.com&csrf_token=invalid' --cookie \"session_id=...\" http://localhost:" << PORT << "/settings" << std::endl;
    std::cout << "   You should also get a 403 Forbidden error." << std::endl;


    while (true) {
        int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to the next attempt
        }
        handle_connection(client_socket);
    }

    close(server_fd);
    return 0;
}