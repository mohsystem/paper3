#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <regex>
#include <memory>
#include <algorithm>

// POSIX/Linux headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/file.h>
#include <termios.h>

// OpenSSL headers
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// --- Constants ---
const int PORT = 8080;
const int BUFFER_SIZE = 1024;
const char* CREDENTIALS_FILE = "users.dat";
const int MIN_PASSWORD_LEN = 8;
const int MAX_USERNAME_LEN = 32;
const int MAX_PASSWORD_LEN = 64;

// --- Hashing Constants ---
const int SALT_SIZE = 16;
const int HASH_SIZE = 32; // SHA-256
const int PBKDF2_ITERATIONS = 210000;

// --- Utility Functions ---

// Securely clear a string
void secure_zero_string(std::string& s) {
    if (!s.empty()) {
        #if defined(_MSC_VER)
            SecureZeroMemory(&s[0], s.size());
        #else
            volatile char* p = &s[0];
            for (size_t i = 0; i < s.size(); ++i) {
                p[i] = 0;
            }
        #endif
    }
}

// RAII wrapper for file descriptors
class FdWrapper {
public:
    explicit FdWrapper(int fd) : fd_(fd) {}
    ~FdWrapper() {
        if (fd_ != -1) {
            close(fd_);
        }
    }
    int get() const { return fd_; }
    FdWrapper(const FdWrapper&) = delete;
    FdWrapper& operator=(const FdWrapper&) = delete;
private:
    int fd_;
};

// --- Cryptography and Authentication ---
namespace Auth {

std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    if (hex.length() % 2 != 0) return bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string hash_password(const std::string& password, std::vector<unsigned char>& salt) {
    salt.resize(SALT_SIZE);
    if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
        throw std::runtime_error("Failed to generate salt.");
    }

    std::vector<unsigned char> hash(HASH_SIZE);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt.data(), salt.size(),
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          hash.size(), hash.data()) != 1) {
        throw std::runtime_error("Failed to hash password.");
    }
    return bytes_to_hex(hash);
}

bool verify_password(const std::string& password, const std::string& stored_record) {
    std::stringstream ss(stored_record);
    std::string username_part, salt_hex, hash_hex;

    std::getline(ss, username_part, ':');
    std::getline(ss, salt_hex, ':');
    std::getline(ss, hash_hex, ':');
    
    if (salt_hex.empty() || hash_hex.empty()) return false;

    std::vector<unsigned char> salt = hex_to_bytes(salt_hex);
    std::vector<unsigned char> stored_hash = hex_to_bytes(hash_hex);
    
    if(salt.empty() || stored_hash.empty()) return false;

    std::vector<unsigned char> calculated_hash(HASH_SIZE);
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt.data(), salt.size(),
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          calculated_hash.size(), calculated_hash.data()) != 1) {
        return false;
    }

    return CRYPTO_memcmp(calculated_hash.data(), stored_hash.data(), HASH_SIZE) == 0;
}

} // namespace Auth

// --- User Management ---
namespace UserManager {

bool is_valid_username(const std::string& username) {
    if (username.empty() || username.length() > MAX_USERNAME_LEN) return false;
    // Alphanumeric and underscore, no path traversal chars
    static const std::regex r("^[a-zA-Z0-9_]+$");
    return std::regex_match(username, r);
}

bool user_exists(const std::string& username) {
    std::ifstream file(CREDENTIALS_FILE);
    if (!file.is_open()) return false;
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind(username + ":", 0) == 0) {
            return true;
        }
    }
    return false;
}

void add_user(const std::string& username, std::string& password) {
    if (!is_valid_username(username)) {
        throw std::runtime_error("Invalid username format or length.");
    }
    if (password.length() < MIN_PASSWORD_LEN || password.length() > MAX_PASSWORD_LEN) {
        throw std::runtime_error("Password does not meet length requirements.");
    }

    // TOCTOU mitigation: Use file locking
    int fd = open(CREDENTIALS_FILE, O_CREAT | O_WRONLY | O_APPEND, 0600);
    if (fd == -1) {
        throw std::runtime_error("Could not open credentials file for writing.");
    }
    FdWrapper fd_wrapper(fd);

    if (flock(fd, LOCK_EX) != 0) {
        throw std::runtime_error("Could not lock credentials file.");
    }

    // Check again after getting lock
    if (user_exists(username)) {
        flock(fd, LOCK_UN);
        throw std::runtime_error("User already exists.");
    }

    std::vector<unsigned char> salt;
    std::string hash_hex = Auth::hash_password(password, salt);
    secure_zero_string(password);

    std::string salt_hex = Auth::bytes_to_hex(salt);
    
    std::string record = username + ":" + salt_hex + ":" + hash_hex + "\n";
    if (write(fd, record.c_str(), record.length()) != (ssize_t)record.length()) {
        flock(fd, LOCK_UN);
        throw std::runtime_error("Failed to write to credentials file.");
    }
    
    flock(fd, LOCK_UN);
    std::cout << "User '" << username << "' added successfully." << std::endl;
}

bool authenticate(const std::string& username, std::string& password) {
    if (!is_valid_username(username)) {
        return false;
    }

    std::ifstream file(CREDENTIALS_FILE);
    if (!file.is_open()) {
        std::cerr << "Could not open credentials file for reading." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind(username + ":", 0) == 0) {
            bool result = Auth::verify_password(password, line);
            secure_zero_string(password);
            return result;
        }
    }
    secure_zero_string(password);
    return false;
}

} // namespace UserManager

// --- Server Logic ---
void handle_client(int client_socket) {
    FdWrapper sock_wrapper(client_socket);
    char buffer[BUFFER_SIZE];
    bool authenticated = false;

    while (true) {
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            std::cout << "Client disconnected or error." << std::endl;
            break;
        }
        buffer[bytes_read] = '\0';
        std::string message(buffer);
        message.erase(message.find_last_not_of("\r\n") + 1);

        std::stringstream ss(message);
        std::string command;
        ss >> command;

        if (!authenticated) {
            if (command == "LOGIN") {
                std::string username, password;
                ss >> username >> password;
                if (!username.empty() && !password.empty() && UserManager::authenticate(username, password)) {
                    authenticated = true;
                    send(client_socket, "OK\n", 3, 0);
                } else {
                    send(client_socket, "FAIL\n", 5, 0);
                }
            } else {
                send(client_socket, "FAIL Please LOGIN first\n", 25, 0);
            }
        } else {
            if (command == "MSG") {
                std::string msg_text;
                std::getline(ss, msg_text); // Read rest of the line
                if (!msg_text.empty() && msg_text[0] == ' ') {
                    msg_text = msg_text.substr(1);
                }
                std::string response = "ECHO: " + msg_text + "\n";
                send(client_socket, response.c_str(), response.length(), 0);
            } else if (command == "LOGOUT") {
                break;
            } else {
                send(client_socket, "FAIL Unknown command\n", 21, 0);
            }
        }
    }
}

void run_server() {
    // In a real application, you should use TLS/SSL to encrypt communication.
    // This example uses plain TCP sockets for simplicity.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        return;
    }
    FdWrapper server_fd_wrapper(server_fd);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        std::cout << "New client connected" << std::endl;
        std::thread(handle_client, client_socket).detach();
    }
}

// --- Client Logic ---
void get_password_no_echo(std::string& password) {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << std::endl;
}

void run_client(const char* server_ip) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return;
    }
    FdWrapper sock_wrapper(sock);

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return;
    }

    std::string username, password;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Enter password: ";
    get_password_no_echo(password);
    
    if (username.length() > MAX_USERNAME_LEN || password.length() > MAX_PASSWORD_LEN) {
        std::cerr << "Username or password too long." << std::endl;
        return;
    }


    std::string login_req = "LOGIN " + username + " " + password + "\n";
    send(sock, login_req.c_str(), login_req.length(), 0);
    secure_zero_string(password);

    char buffer[BUFFER_SIZE] = {0};
    int bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        if (std::string(buffer) == "OK\n") {
            std::cout << "Login successful. Type 'MSG <your_message>' or 'LOGOUT'." << std::endl;
            while (true) {
                std::cout << "> ";
                std::string line;
                std::getline(std::cin, line);
                if (line.empty()) continue;
                if(line.size() + 1 > BUFFER_SIZE) {
                    std::cout << "Message too long." << std::endl;
                    continue;
                }
                
                line += "\n";
                send(sock, line.c_str(), line.length(), 0);

                if (line.rfind("LOGOUT", 0) == 0) break;

                bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    std::cout << "Server: " << buffer;
                } else {
                    std::cout << "Server disconnected." << std::endl;
                    break;
                }
            }
        } else {
            std::cerr << "Login failed. Server response: " << buffer;
        }
    } else {
        std::cerr << "No response from server or connection closed." << std::endl;
    }
}

// --- Main ---
void print_usage(const char* prog_name) {
    std::cerr << "Usage:\n"
              << "  " << prog_name << " server\n"
              << "  " << prog_name << " client <server_ip>\n"
              << "  " << prog_name << " adduser <username> <password>\n";
}

int main(int argc, char* argv[]) {
    // ---- Test Cases ----
    // To test the system, run the following commands in separate terminals.
    //
    // Test Case 1: Add a valid user.
    //   ./program adduser testuser StrongPwd123!
    //   Expected: User 'testuser' added successfully.
    //
    // Test Case 2: Start the server.
    //   ./program server
    //   Expected: Server listens on port 8080.
    //
    // Test Case 3: Client connects with correct credentials and chats.
    //   ./program client 127.0.0.1
    //   (At prompt, enter 'testuser' and 'StrongPwd123!')
    //   Expected: Login successful. Can send and receive messages.
    //
    // Test Case 4: Client connects with incorrect credentials.
    //   ./program client 127.0.0.1
    //   (At prompt, enter 'testuser' and 'wrongpassword')
    //   Expected: Login failed.
    //
    // Test Case 5: Attempt to add a user with an invalid username.
    //   ./program adduser ../../etc/passwd bad
    //   Expected: Error message about invalid username format.
    // --------------------

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    try {
        if (command == "server") {
            run_server();
        } else if (command == "client") {
            if (argc != 3) {
                print_usage(argv[0]);
                return 1;
            }
            run_client(argv[2]);
        } else if (command == "adduser") {
            if (argc != 4) {
                print_usage(argv[0]);
                return 1;
            }
            std::string username = argv[2];
            std::string password = argv[3];
            UserManager::add_user(username, password);
            secure_zero_string(password);
        } else {
            print_usage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}