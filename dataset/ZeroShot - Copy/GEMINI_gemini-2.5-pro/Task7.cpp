#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <random>

// Platform-specific socket includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

// --- SHA256 Implementation (vendored for single-file requirement) ---
// Based on a public domain implementation
class SHA256 {
protected:
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    const static uint32 sha256_k[];
    static const unsigned int SHA224_256_BLOCK_SIZE = (512 / 8);
public:
    void init();
    void update(const unsigned char *message, unsigned int len);
    void final(unsigned char *digest);
    static const unsigned int DIGEST_SIZE = (256 / 8);

protected:
    void transform(const unsigned char *message, unsigned int block_nb);
    unsigned int m_tot_len;
    unsigned int m_len;
    unsigned char m_block[2 * SHA224_256_BLOCK_SIZE];
    uint32 m_h[8];
};

#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str) {str_tmp = (str); x = ((uint32)str_tmp[0] << 24) | ((uint32)str_tmp[1] << 16) | ((uint32)str_tmp[2] << 8) | ((uint32)str_tmp[3]);}
#define SHA2_PACK32(str, x) {uint32 x_tmp = (x); (str)[0] = (uint8)(x_tmp >> 24); (str)[1] = (uint8)(x_tmp >> 16); (str)[2] = (uint8)(x_tmp >> 8); (str)[3] = (uint8)x_tmp;}

const unsigned int SHA256::sha256_k[64] =
    {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
     0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
     0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
     0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
     0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
     0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
     0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
     0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
     0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
     0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
     0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
     0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
     0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
     0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
     0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
     0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

void SHA256::transform(const unsigned char *message, unsigned int block_nb) {
    uint32 w[64];
    uint32 wv[8];
    uint32 t1, t2;
    const unsigned char *sub_block;
    int i;
    int j;
    const uint8* str_tmp;
    for (i = 0; i < (int) block_nb; i++) {
        sub_block = message + (i << 6);
        for (j = 0; j < 16; j++) {
            SHA2_UNPACK32(w[j], &sub_block[j << 2]);
        }
        for (j = 16; j < 64; j++) {
            w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
        }
        for (j = 0; j < 8; j++) {
            wv[j] = m_h[j];
        }
        for (j = 0; j < 64; j++) {
            t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6]) + sha256_k[j] + w[j];
            t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = t1 + t2;
        }
        for (j = 0; j < 8; j++) {
            m_h[j] += wv[j];
        }
    }
}

void SHA256::init() {
    m_h[0] = 0x6a09e667; m_h[1] = 0xbb67ae85; m_h[2] = 0x3c6ef372; m_h[3] = 0xa54ff53a;
    m_h[4] = 0x510e527f; m_h[5] = 0x9b05688c; m_h[6] = 0x1f83d9ab; m_h[7] = 0x5be0cd19;
    m_len = 0;
    m_tot_len = 0;
}

void SHA256::update(const unsigned char *message, unsigned int len) {
    unsigned int block_nb;
    unsigned int new_len, rem_len, tmp_len;
    const unsigned char *shifted_message;
    tmp_len = SHA224_256_BLOCK_SIZE - m_len;
    rem_len = len < tmp_len ? len : tmp_len;
    memcpy(&m_block[m_len], message, rem_len);
    if (m_len + len < SHA224_256_BLOCK_SIZE) {
        m_len += len;
        return;
    }
    new_len = len - rem_len;
    block_nb = new_len / SHA224_256_BLOCK_SIZE;
    shifted_message = message + rem_len;
    transform(m_block, 1);
    transform(shifted_message, block_nb);
    rem_len = new_len % SHA224_256_BLOCK_SIZE;
    memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
    m_len = rem_len;
    m_tot_len += (block_nb + 1) << 6;
}

void SHA256::final(unsigned char *digest) {
    unsigned int block_nb;
    unsigned int pm_len;
    unsigned int len_b;
    int i;
    block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9) < (m_len % SHA224_256_BLOCK_SIZE)));
    len_b = (m_tot_len + m_len) << 3;
    pm_len = block_nb << 6;
    memset(m_block + m_len, 0, pm_len - m_len);
    m_block[m_len] = 0x80;
    SHA2_PACK32(&m_block[pm_len - 4], len_b);
    transform(m_block, block_nb);
    for (i = 0; i < 8; i++) {
        SHA2_PACK32(digest + (i << 2), m_h[i]);
    }
}
// --- End of SHA256 Implementation ---

// --- Constants and Globals ---
const int PORT = 12345;
const char* HOST = "127.0.0.1";
const std::string USER_FILE = "users.dat";
std::map<std::string, std::string> user_credentials;
std::map<std::string, SOCKET> connected_clients;
std::mutex clients_mutex;

// --- Utility Functions ---
std::string sha256(const std::string& str) {
    unsigned char digest[SHA256::DIGEST_SIZE];
    SHA256 ctx;
    ctx.init();
    ctx.update((unsigned char*)str.c_str(), str.length());
    ctx.final(digest);
    
    std::stringstream ss;
    for(unsigned int i = 0; i < SHA256::DIGEST_SIZE; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return ss.str();
}

std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for(unsigned char b : bytes) {
        ss << std::setw(2) << static_cast<unsigned>(b);
    }
    return ss.str();
}

std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = (unsigned char) strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

// NOTE: This is a weak substitute for a proper KDF like PBKDF2 or bcrypt.
// It's used here to avoid external libraries.
std::string hash_password(const std::string& password, const std::string& salt_hex) {
    std::string salted_password = password + salt_hex;
    std::string current_hash = sha256(salted_password);
    for(int i=0; i < 10000; ++i) { // Key stretching
        current_hash = sha256(current_hash);
    }
    return current_hash;
}

std::string generate_salt(size_t len) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    std::vector<unsigned char> salt(len);
    for(size_t i = 0; i < len; ++i) {
        salt[i] = static_cast<unsigned char>(distrib(gen));
    }
    return bytes_to_hex(salt);
}

void print_usage() {
    std::cout << "--- Secure Chat System ---" << std::endl;
    std::cout << "1. To add a user (run this first):" << std::endl;
    std::cout << "   ./Task7 adduser <username> <password>" << std::endl;
    std::cout << "\n2. To start the server:" << std::endl;
    std::cout << "   ./Task7 server" << std::endl;
    std::cout << "\n3. To start the client (in a new terminal):" << std::endl;
    std::cout << "   ./Task7 client" << std::endl;
    std::cout << "\n--- Test Cases to run on Client ---" << std::endl;
    std::cout << "1. Login with correct credentials." << std::endl;
    std::cout << "2. Try to login with correct user, wrong password." << std::endl;
    std::cout << "3. Try to login with a non-existent user." << std::endl;
    std::cout << "4. Login correctly and send a message." << std::endl;
    std::cout << "5. Try to login with an empty username or password." << std::endl;
}

void add_user(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Username and password cannot be empty." << std::endl;
        return;
    }
    std::ofstream file(USER_FILE, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open user file for writing." << std::endl;
        return;
    }
    std::string salt = generate_salt(16);
    std::string hashed_pw = hash_password(password, salt);
    file << username << ":" << salt << ":" << hashed_pw << std::endl;
    std::cout << "User '" << username << "' added successfully." << std::endl;
}

void load_users() {
    std::ifstream file(USER_FILE);
    if (!file.is_open()) {
        std::cerr << "User file not found. Please add users first." << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string username;
        std::getline(ss, username, ':');
        user_credentials[username] = line;
    }
    std::cout << "Loaded " << user_credentials.size() << " users." << std::endl;
}

void broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::cout << "Broadcasting: " << message << std::endl;
    for (auto const& [user, sock] : connected_clients) {
        send(sock, message.c_str(), message.length(), 0);
    }
}

// --- Server Implementation ---
void handle_client(SOCKET client_socket) {
    std::string username;
    char buffer[4096];
    
    // Authentication
    send(client_socket, "SUBMIT_credentials\n", 19, 0);
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        closesocket(client_socket);
        return;
    }
    buffer[bytes_received] = '\0';
    std::string login_request(buffer);
    login_request.erase(login_request.find_last_not_of("\r\n") + 1);

    std::stringstream ss(login_request);
    std::string command, provided_user, provided_pass;
    ss >> command >> provided_user >> provided_pass;

    bool authenticated = false;
    if (command == "LOGIN" && !provided_user.empty() && !provided_pass.empty()) {
        if (user_credentials.count(provided_user)) {
            std::string record = user_credentials[provided_user];
            std::stringstream record_ss(record);
            std::string u, salt, hash;
            std::getline(record_ss, u, ':');
            std::getline(record_ss, salt, ':');
            std::getline(record_ss, hash, ':');

            if (hash_password(provided_pass, salt) == hash) {
                authenticated = true;
                username = provided_user;
                send(client_socket, "LOGIN_SUCCESS\n", 14, 0);
            }
        }
    }
    
    if (!authenticated) {
        send(client_socket, "LOGIN_FAIL Invalid username or password.\n", 39, 0);
        closesocket(client_socket);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        connected_clients[username] = client_socket;
    }

    broadcast("SERVER: " + username + " has joined.\n");
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0';
        std::string message(buffer);
        message.erase(message.find_last_not_of("\r\n") + 1);
        
        if (message == "LOGOUT") {
            break;
        }
        broadcast(username + ": " + message + "\n");
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        connected_clients.erase(username);
    }
    broadcast("SERVER: " + username + " has left.\n");
    closesocket(client_socket);
}

void run_server() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif
    load_users();

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(server_socket);
        return;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(server_socket);
        return;
    }

    std::cout << "Server started on port " << PORT << std::endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }
        std::thread(handle_client, client_socket).detach();
    }

    closesocket(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

// --- Client Implementation ---
void receive_messages(SOCKET sock) {
    char buffer[4096];
    while (true) {
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "\nConnection to server lost." << std::endl;
            closesocket(sock);
            exit(0);
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer;
        std::cout.flush();
    }
}

void run_client() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(client_socket);
        return;
    }
    std::cout << "Connected to server." << std::endl;

    char buffer[1024];
    int bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if(bytes > 0) {
        buffer[bytes] = '\0';
        if (std::string(buffer).find("SUBMIT_credentials") != std::string::npos) {
            std::string username, password;
            std::cout << "Enter username: ";
            std::cin >> username;
            std::cout << "Enter password: ";
            std::cin >> password;
            
            std::string login_msg = "LOGIN " + username + " " + password + "\n";
            send(client_socket, login_msg.c_str(), login_msg.length(), 0);

            bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                std::cout << "Server: " << buffer;
                if (std::string(buffer).find("LOGIN_SUCCESS") != std::string::npos) {
                    std::cout << "Login successful! You can now chat. Type 'LOGOUT' to exit." << std::endl;
                    
                    std::thread receiver(receive_messages, client_socket);
                    receiver.detach();
                    
                    std::string line;
                    std::getline(std::cin, line); // Consume the leftover newline
                    while(std::getline(std::cin, line)) {
                        if (line == "LOGOUT") {
                            send(client_socket, "LOGOUT\n", 7, 0);
                            break;
                        }
                        line += "\n";
                        send(client_socket, line.c_str(), line.length(), 0);
                    }
                }
            }
        }
    }
    
    closesocket(client_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

// --- Main Dispatcher ---
int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    std::string command = argv[1];
    if (command == "server") {
        run_server();
    } else if (command == "client") {
        run_client();
    } else if (command == "adduser") {
        if (argc != 4) {
            std::cerr << "Usage: ./Task7 adduser <username> <password>" << std::endl;
            return 1;
        }
        add_user(argv[2], argv[3]);
    } else {
        print_usage();
        return 1;
    }
    return 0;
}