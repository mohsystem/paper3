#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <cstring>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <cstdint>

// POSIX/Linux specific includes for networking and directories
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

// Note: This C++ code is specific to POSIX-compliant systems (like Linux, macOS)
// It will not compile on Windows without modification (e.g., using Winsock).

const int PORT = 8082;
const char* UPLOAD_DIR = "uploads_cpp";
std::atomic<bool> server_running(true);

void handle_connection(int client_socket) {
    try {
        // 1. Read file name length (4 bytes)
        uint32_t name_len_net;
        if (read(client_socket, &name_len_net, sizeof(name_len_net)) != sizeof(name_len_net)) {
            throw std::runtime_error("Failed to read name length");
        }
        uint32_t name_len = ntohl(name_len_net);

        // 2. Read file name
        std::vector<char> name_buffer(name_len);
        if (read(client_socket, name_buffer.data(), name_len) != name_len) {
            throw std::runtime_error("Failed to read name");
        }
        std::string filename(name_buffer.begin(), name_buffer.end());

        // 3. Read file content length (8 bytes)
        uint64_t content_len_net;
        if (read(client_socket, &content_len_net, sizeof(content_len_net)) != sizeof(content_len_net)) {
            throw std::runtime_error("Failed to read content length");
        }
        uint64_t content_len = ntohll(content_len_net);

        // 4. Read file content and write to file
        std::string full_path = std::string(UPLOAD_DIR) + "/" + filename;
        std::ofstream outfile(full_path, std::ios::binary);
        if (!outfile.is_open()) {
            throw std::runtime_error("Failed to open file for writing");
        }

        std::vector<char> content_buffer(4096);
        uint64_t bytes_received = 0;
        while (bytes_received < content_len) {
            ssize_t to_read = std::min((uint64_t)content_buffer.size(), content_len - bytes_received);
            ssize_t bytes_read = read(client_socket, content_buffer.data(), to_read);
            if (bytes_read <= 0) {
                throw std::runtime_error("Connection closed while receiving file content");
            }
            outfile.write(content_buffer.data(), bytes_read);
            bytes_received += bytes_read;
        }
        outfile.close();
        std::cout << "Server received and saved file: " << full_path << std::endl;

        // 5. Send confirmation
        std::string response = "File uploaded successfully to " + full_path;
        write(client_socket, response.c_str(), response.length());

    } catch (const std::exception& e) {
        std::cerr << "Server Error: " << e.what() << std::endl;
        const char* error_msg = "Server-side error occurred.";
        write(client_socket, error_msg, strlen(error_msg));
    }
    close(client_socket);
}

void start_server() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }
    
    std::cout << "C++ server started on port " << PORT << std::endl;

    while (server_running) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) {
            // This might happen when we shut down, it's ok
            if (server_running) perror("accept");
            continue;
        }
        // Spawn a thread to handle each client to avoid blocking the accept loop
        std::thread(handle_connection, client_socket).detach();
    }

    close(server_fd);
    std::cout << "C++ server stopped." << std::endl;
}

std::string upload_file(const std::string& host, int port, const std::string& file_name, const std::string& file_content) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return "Client Error: Socket creation error";
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
        return "Client Error: Invalid address / Address not supported";
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return "Client Error: Connection Failed";
    }

    // 1. Send file name length
    uint32_t name_len = file_name.length();
    uint32_t name_len_net = htonl(name_len);
    send(sock, &name_len_net, sizeof(name_len_net), 0);

    // 2. Send file name
    send(sock, file_name.c_str(), name_len, 0);

    // 3. Send file content length
    uint64_t content_len = file_content.length();
    uint64_t content_len_net = htonll(content_len);
    send(sock, &content_len_net, sizeof(content_len_net), 0);

    // 4. Send file content
    send(sock, file_content.c_str(), content_len, 0);

    // 5. Receive response
    char buffer[1024] = {0};
    read(sock, buffer, 1024);
    close(sock);
    
    return std::string("Client Response: ") + buffer;
}


int main() {
    // Create upload directory
    mkdir(UPLOAD_DIR, 0755);

    // 1. Start server in a background thread
    std::thread server_thread(start_server);
    
    // Give server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 2. Run test cases
    std::cout << "\n--- Running C++ Test Cases ---" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::string file_name = "test" + std::to_string(i) + ".txt";
        std::string content = "This is C++ test file " + std::to_string(i) + ".";
        std::cout << "Test " << i << ": Uploading '" << file_name << "'..." << std::endl;
        std::string result = upload_file("127.0.0.1", PORT, file_name, content);
        std::cout << result << std::endl;
    }
    std::cout << "--- C++ Test Cases Finished ---\n" << std::endl;

    // 3. Stop the server
    server_running = false;
    // Make a dummy connection to unblock the accept() call
    int dummy_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(dummy_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    close(dummy_sock);
    
    server_thread.join();

    return 0;
}