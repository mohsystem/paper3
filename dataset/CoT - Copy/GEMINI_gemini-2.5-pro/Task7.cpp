#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Socket-related headers for POSIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// --- !! SECURITY WARNING !! ---
// This code sends passwords in PLAINTEXT over the network.
// It is NOT secure and should ONLY be used for educational purposes.
// A production system MUST use TLS/SSL to encrypt all communication.
// -----------------------------

#define PORT 8082
#define BUFFER_SIZE 1024
const char* USER_FILE = "users.dat";

// --- SHA256 Implementation (Compact, self-contained) ---
// Based on public domain work.
class SHA256 {
public:
    SHA256();
    void update(const uint8_t *data, size_t len);
    void final(uint8_t *hash);
private:
    uint32_t m_state[8];
    uint64_t m_bitcount;
    uint8_t m_buffer[64];
    static const uint32_t K[64];
    void transform();
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define BSIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define BSIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SSIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SSIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

const uint32_t SHA256::K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

SHA256::SHA256() : m_bitcount(0) {
    m_state[0] = 0x6a09e667; m_state[1] = 0xbb67ae85; m_state[2] = 0x3c6ef372; m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f; m_state[5] = 0x9b05688c; m_state[6] = 0x1f83d9ab; m_state[7] = 0x5be0cd19;
}
void SHA256::transform() {
    uint32_t a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3], e = m_state[4], f = m_state[5], g = m_state[6], h = m_state[7];
    uint32_t w[64];
    for (int i = 0; i < 16; ++i)
        w[i] = (m_buffer[4*i] << 24) | (m_buffer[4*i+1] << 16) | (m_buffer[4*i+2] << 8) | m_buffer[4*i+3];
    for (int i = 16; i < 64; ++i)
        w[i] = SSIG1(w[i - 2]) + w[i - 7] + SSIG0(w[i - 15]) + w[i - 16];
    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + BSIG1(e) + CH(e, f, g) + K[i] + w[i];
        uint32_t t2 = BSIG0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    m_state[0] += a; m_state[1] += b; m_state[2] += c; m_state[3] += d;
    m_state[4] += e; m_state[5] += f; m_state[6] += g; m_state[7] += h;
}
void SHA256::update(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        m_buffer[m_bitcount % 64] = data[i];
        if (++m_bitcount % 64 == 0) transform();
    }
}
void SHA256::final(uint8_t *hash) {
    uint8_t pad[64];
    uint64_t i = m_bitcount % 64;
    pad[0] = 0x80;
    memset(pad + 1, 0, 64 - (i + 1));
    if (i >= 56) {
        update(pad, 64 - i);
        memset(pad, 0, 56);
    } else {
        update(pad, 56 - i);
    }
    uint64_t bits = m_bitcount * 8;
    for (i = 0; i < 8; ++i) pad[56+i] = (bits >> (56-8*i)) & 0xFF;
    update(pad, 8);
    for (i = 0; i < 8; i++) {
        hash[i*4] = (m_state[i] >> 24) & 0xFF;
        hash[i*4+1] = (m_state[i] >> 16) & 0xFF;
        hash[i*4+2] = (m_state[i] >> 8) & 0xFF;
        hash[i*4+3] = m_state[i] & 0xFF;
    }
}

// --- Security and Helper Utilities ---
std::string to_hex(const std::vector<uint8_t>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::vector<uint8_t> from_hex(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string hash_password(const std::string& password, const std::string& salt_hex) {
    SHA256 sha;
    std::string to_hash = salt_hex + password;
    sha.update(reinterpret_cast<const uint8_t*>(to_hash.c_str()), to_hash.length());
    uint8_t hash[32];
    sha.final(hash);
    return to_hex(std::vector<uint8_t>(hash, hash + 32));
}

bool verify_password(const std::string& password, const std::string& stored_hash, const std::string& salt_hex) {
    return hash_password(password, salt_hex) == stored_hash;
}

std::string generate_salt(size_t len) {
    std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
    std::vector<uint8_t> salt(len);
    if (urandom) {
        urandom.read(reinterpret_cast<char*>(salt.data()), len);
        urandom.close();
    }
    return to_hex(salt);
}

void add_user(const std::string& username, const std::string& password) {
    std::ifstream infile(USER_FILE);
    std::string line;
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string u;
        std::getline(ss, u, ':');
        if (u == username) {
            std::cout << "Error: User '" << username << "' already exists." << std::endl;
            return;
        }
    }
    infile.close();

    std::ofstream outfile(USER_FILE, std::ios::app);
    if (!outfile) {
        std::cerr << "Error: Could not open user file for writing." << std::endl;
        return;
    }
    std::string salt = generate_salt(16);
    std::string hashed_pwd = hash_password(password, salt);
    outfile << username << ":" << hashed_pwd << ":" << salt << std::endl;
    std::cout << "User '" << username << "' added successfully." << std::endl;
}

bool authenticate(const std::string& username, const std::string& password) {
    std::ifstream infile(USER_FILE);
    if (!infile) return false;
    std::string line;
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string u, h, s;
        std::getline(ss, u, ':');
        std::getline(ss, h, ':');
        std::getline(ss, s);
        if (u == username) {
            return verify_password(password, h, s);
        }
    }
    return false;
}

// --- Server ---
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    if (recv(client_socket, buffer, BUFFER_SIZE - 1, 0) <= 0) {
        close(client_socket);
        return;
    }

    std::string request(buffer);
    std::stringstream ss(request);
    std::string command, username, password;
    ss >> command >> username >> password;
    
    std::cout << "Server received: " << command << " " << username << " *****" << std::endl;

    std::string response;
    if (command == "LOGIN" && authenticate(username, password)) {
        response = "LOGIN_SUCCESS";
        std::cout << "Authentication successful for " << username << std::endl;
    } else {
        response = "LOGIN_FAIL";
        std::cout << "Authentication failed for " << username << std::endl;
    }

    send(client_socket, response.c_str(), response.length(), 0);
    close(client_socket);
}

void run_server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        return;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        close(server_fd);
        return;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }
        std::thread(handle_client, client_socket).detach();
    }
    close(server_fd);
}

// --- Client ---
void run_client(const std::string& username, const std::string& password) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation error" << std::endl;
        return;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed. Is the server running?" << std::endl;
        close(sock);
        return;
    }

    std::string request = "LOGIN " + username + " " + password;
    send(sock, request.c_str(), request.length(), 0);
    std::cout << "Client sent login request for user: " << username << std::endl;

    char buffer[BUFFER_SIZE] = {0};
    recv(sock, buffer, BUFFER_SIZE - 1, 0);
    std::cout << "Server response: " << buffer << std::endl;
    
    close(sock);
}


void print_usage() {
    std::cout << "Usage: ./Task7 <mode> [options]" << std::endl;
    std::cout << "Modes:" << std::endl;
    std::cout << "  server                  - Start the chat server." << std::endl;
    std::cout << "  adduser <user> <pass>   - Add a new user to the user file." << std::endl;
    std::cout << "  client                  - Run predefined client test cases." << std::endl;
    std::cout << "Compile with: g++ -std=c++11 -o Task7 your_file_name.cpp -pthread" << std::endl;
}

void run_client_tests() {
    std::cout << "\n--- Running Client Test Cases ---" << std::endl;
    std::cout << "Instructions:" << std::endl;
    std::cout << "1. Compile the code: g++ -std=c++11 -o Task7 your_file_name.cpp -pthread" << std::endl;
    std::cout << "2. Start the server in a separate terminal: ./Task7 server" << std::endl;
    std::cout << "3. Add a test user in another terminal: ./Task7 adduser testuser testpass123" << std::endl;
    std::cout << "4. The test cases below will now run against the server.\n" << std::endl;

    std::cout << "--- Test Case 1: Successful Login ---" << std::endl;
    run_client("testuser", "testpass123");
    
    std::cout << "\n--- Test Case 2: Incorrect Password ---" << std::endl;
    run_client("testuser", "wrongpassword");

    std::cout << "\n--- Test Case 3: Non-existent User ---" << std::endl;
    run_client("nouser", "anypassword");

    std::cout << "\n--- Test Case 4: Another Successful Login ---" << std::endl;
    run_client("testuser", "testpass123");

    std::cout << "\n--- Test Case 5: Empty Password ---" << std::endl;
    run_client("testuser", "");
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "server") {
        run_server();
    } else if (mode == "adduser") {
        if (argc != 4) {
            std::cout << "Usage: ./Task7 adduser <username> <password>" << std::endl;
            return 1;
        }
        add_user(argv[2], argv[3]);
    } else if (mode == "client") {
        run_client_tests();
    } else {
        print_usage();
    }

    return 0;
}