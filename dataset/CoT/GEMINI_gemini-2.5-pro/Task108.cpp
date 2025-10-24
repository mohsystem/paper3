#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <atomic>
#include <cstring> // For memset, strerror
#include <unistd.h> // For close, read, write
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>

// Global configuration
const int PORT = 8080;
const char* HOST = "127.0.0.1";
// Atomic flag to control the server's main loop.
std::atomic<bool> server_running(true);

// 1. Server-side object logic
namespace Calculator {
    double process_command(const std::string& command) {
        std::stringstream ss(command);
        std::string operation;
        double a, b;
        ss >> operation >> a >> b;

        if (ss.fail()) {
            throw std::runtime_error("Invalid command format. Expected: OP A B");
        }

        if (operation == "ADD") return a + b;
        if (operation == "SUB") return a - b;
        if (operation == "MUL") return a * b;
        if (operation == "DIV") {
            if (b == 0) {
                throw std::runtime_error("Division by zero");
            }
            return a / b;
        }
        throw std::runtime_error("Unknown operation");
    }
}

// 2. Client handler function
void handle_client(int client_socket) {
    std::cout << "Server: New client connected." << std::endl;
    char buffer[1024] = {0};

    // Use a loop to handle multiple requests from the same client
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        // Read data from the client
        // A robust server would handle partial reads.
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);

        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                std::cout << "Server: Client disconnected." << std::endl;
            } else {
                std::cerr << "Server: Read error: " << strerror(errno) << std::endl;
            }
            break; // Exit loop on error or disconnection
        }

        std::string command(buffer, bytes_read);
        // Trim newline characters
        command.erase(command.find_last_not_of("\r\n") + 1);

        if (command == "SHUTDOWN") {
            std::cout << "Server: Shutdown command received." << std::endl;
            server_running = false;
            break;
        }
        
        std::string response;
        try {
            double result = Calculator::process_command(command);
            response = std::to_string(result) + "\n";
        } catch (const std::runtime_error& e) {
            response = "ERROR: " + std::string(e.what()) + "\n";
        }

        // Send the response back to the client
        write(client_socket, response.c_str(), response.length());
    }

    close(client_socket);
}

// 3. Server main function
void run_server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Server: Failed to create socket." << std::endl;
        return;
    }

    // Allow reusing the address to avoid "Address already in use" errors
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Server: Bind failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "Server: Listen failed." << std::endl;
        close(server_fd);
        return;
    }

    std::cout << "Server: Listening on port " << PORT << std::endl;

    while (server_running) {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket < 0) {
            if (server_running) { // Avoid error message on controlled shutdown
                 std::cerr << "Server: Accept failed." << std::endl;
            }
            continue;
        }
        // Create a new thread for each client. detach() allows the thread
        // to run independently. For a real server, you'd use a thread pool.
        std::thread(handle_client, client_socket).detach();
    }
    
    close(server_fd);
    std::cout << "Server: Shut down." << std::endl;
}

// 4. Client main function
void run_client() {
    std::cout << "\nClient: Starting client..." << std::endl;
    
    auto run_test = [&](const std::string& test_name, const std::string& command) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Client: Socket creation error." << std::endl;
            return;
        }

        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, HOST, &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Client: Connection Failed for test '" << test_name << "'." << std::endl;
            close(sock);
            return;
        }

        std::string request = command + "\n";
        write(sock, request.c_str(), request.length());
        
        char buffer[1024] = {0};
        read(sock, buffer, sizeof(buffer) - 1);
        std::string response(buffer);
        response.erase(response.find_last_not_of("\r\n") + 1);
        
        std::cout << "Client Test " << test_name << " (" << command << "): " << response << std::endl;
        
        close(sock);
    };

    // --- Test Cases ---
    run_test("1", "ADD 10 5");
    run_test("2", "SUB 10 5");
    run_test("3", "MUL 10 5");
    run_test("4", "DIV 10 5");
    run_test("5", "DIV 10 0");

    // Send shutdown signal
    int final_sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &serv_addr.sin_addr);
    if(connect(final_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == 0){
        std::string shutdown_cmd = "SHUTDOWN\n";
        write(final_sock, shutdown_cmd.c_str(), shutdown_cmd.length());
        close(final_sock);
    }
}


// 5. Main entry point
int main() {
    // Start the server in a separate thread
    std::thread server_thread(run_server);
    
    // Give the server a moment to start up
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Run client tests
    run_client();

    // Wait for the server thread to finish
    server_thread.join();
    
    std::cout << "Program finished." << std::endl;

    return 0;
}