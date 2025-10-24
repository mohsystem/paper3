#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>

#define BUFFER_SIZE 4096
#define MAX_CLIENTS 100

// Globals for server
std::vector<int> g_client_sockets;
std::mutex g_clients_mutex;

void broadcast_message(const std::string& message, int sender_socket);
void handle_client(int client_socket);
void server_main(int port);
void send_messages(int server_socket);
void receive_messages(int server_socket);
void client_main(const char* ip, int port);

// Broadcasts a message to all clients except the sender
void broadcast_message(const std::string& message, int sender_socket) {
    std::lock_guard<std::mutex> lock(g_clients_mutex);
    std::string msg_with_newline = message + "\n";
    for (int socket : g_client_sockets) {
        if (socket != sender_socket) {
            // MSG_NOSIGNAL prevents send from generating SIGPIPE if the other end is closed
            if (send(socket, msg_with_newline.c_str(), msg_with_newline.length(), MSG_NOSIGNAL) < 0) {
                perror("send failed");
            }
        }
    }
}

// Handles a single client connection
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::string client_id_str = "Client " + std::to_string(client_socket);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << client_id_str << " disconnected." << std::endl;
                broadcast_message(client_id_str + " has left the chat.", -1);
            } else {
                perror("recv failed");
            }
            break; 
        }

        buffer[bytes_received] = '\0';
        std::string message(buffer);

        std::string broadcast_msg = client_id_str + ": " + message;
        std::cout << "Broadcasting: " << broadcast_msg << std::endl;
        broadcast_message(broadcast_msg, client_socket);
    }

    close(client_socket);
    std::lock_guard<std::mutex> lock(g_clients_mutex);
    g_client_sockets.erase(std::remove(g_client_sockets.begin(), g_client_sockets.end(), client_socket), g_client_sockets.end());
}


// Server entry point
void server_main(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_addrlen = sizeof(client_address);
        int new_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_addrlen);

        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        std::lock_guard<std::mutex> lock(g_clients_mutex);
        if (g_client_sockets.size() >= MAX_CLIENTS) {
            std::cerr << "Max clients reached. Connection rejected." << std::endl;
            const char* msg = "Server is full.\n";
            send(new_socket, msg, strlen(msg), 0);
            close(new_socket);
            continue;
        }

        g_client_sockets.push_back(new_socket);
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "New connection from " << client_ip << ":" << ntohs(client_address.sin_port) << " on socket " << new_socket << std::endl;
        broadcast_message("Client " + std::to_string(new_socket) + " has joined the chat.", -1);
        
        std::thread client_thread(handle_client, new_socket);
        client_thread.detach();
    }

    close(server_fd);
}


// Client thread for sending messages
void send_messages(int server_socket) {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty()) {
            if (send(server_socket, line.c_str(), line.length(), 0) < 0) {
                perror("send failed");
                break;
            }
        }
    }
    shutdown(server_socket, SHUT_WR);
}


// Client thread for receiving messages
void receive_messages(int server_socket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(server_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "\nServer closed the connection." << std::endl;
            } else {
                perror("recv failed");
            }
            exit(0);
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer; // Newline is sent by server
    }
}


// Client entry point
void client_main(const char* ip, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Connected to the server. You can start typing." << std::endl;

    std::thread sender_thread(send_messages, sock);
    std::thread receiver_thread(receive_messages, sock);

    sender_thread.join();
    receiver_thread.join();

    close(sock);
}


int main(int argc, char const *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " server <port>\n"
                  << "  " << argv[0] << " client <ip> <port>\n\n"
                  << "Test Cases (run in separate terminals):\n"
                  << "1. Run as server: " << argv[0] << " server 8080\n"
                  << "2. Run as client: " << argv[0] << " client 127.0.0.1 8080\n"
                  << "3. Run another client: " << argv[0] << " client 127.0.0.1 8080\n"
                  << "4. Test client disconnect: Close a client terminal with Ctrl+C.\n"
                  << "5. Test invalid arguments: " << argv[0] << " client 127.0.0.1 99999\n";
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "server" && argc == 3) {
        try {
            int port = std::stoi(argv[2]);
            if (port <= 0 || port > 65535) {
                std::cerr << "Invalid port number." << std::endl;
                return 1;
            }
            server_main(port);
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number: " << e.what() << std::endl;
            return 1;
        }
    } else if (mode == "client" && argc == 4) {
        try {
            int port = std::stoi(argv[3]);
            if (port <= 0 || port > 65535) {
                std::cerr << "Invalid port number." << std::endl;
                return 1;
            }
            client_main(argv[2], port);
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number: " << e.what() << std::endl;
            return 1;
        }
    } else {
        fprintf(stderr, "Invalid arguments. See usage printed below.\n");
        main(1, (char**)argv);
        return 1;
    }

    return 0;
}