
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
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

using namespace std;

class Task7 {
public:
    // User class
    class User {
    public:
        string username;
        string password;
        
        User(string u, string p) : username(u), password(p) {}
    };
    
    // Client class
    class ChatClient {
    private:
        SOCKET sock;
        
    public:
        ChatClient(const string& host, int port) {
            #ifdef _WIN32
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            #endif
            
            sock = socket(AF_INET, SOCK_STREAM, 0);
            
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = inet_addr(host.c_str());
            
            connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
        }
        
        string login(const string& username, const string& password) {
            string request = "LOGIN:" + username + ":" + password;
            send(sock, request.c_str(), request.length(), 0);
            
            char buffer[1024] = {0};
            recv(sock, buffer, 1024, 0);
            return string(buffer);
        }
        
        void close_connection() {
            closesocket(sock);
            #ifdef _WIN32
            WSACleanup();
            #endif
        }
    };
    
    // Server class
    class ChatServer {
    private:
        SOCKET server_socket;
        map<string, string> users;
        string user_file_path;
        
        void load_users() {
            ifstream file(user_file_path);
            if (!file.is_open()) {
                ofstream outfile(user_file_path);
                outfile << "alice:password123\\n";
                outfile << "bob:securepass\\n";
                outfile << "charlie:mypass456\\n";
                outfile << "david:test1234\\n";
                outfile << "eve:qwerty789\\n";
                outfile.close();
                file.open(user_file_path);
            }
            
            string line;
            while (getline(file, line)) {
                size_t pos = line.find(':');
                if (pos != string::npos) {
                    string username = line.substr(0, pos);
                    string password = line.substr(pos + 1);
                    users[username] = password;
                }
            }
            file.close();
        }
        
        string process_request(const string& request) {
            if (request.substr(0, 6) != "LOGIN:") {
                return "ERROR:Invalid request format";
            }
            
            size_t first_colon = request.find(':');
            size_t second_colon = request.find(':', first_colon + 1);
            
            if (second_colon == string::npos) {
                return "ERROR:Invalid login format";
            }
            
            string username = request.substr(first_colon + 1, second_colon - first_colon - 1);
            string password = request.substr(second_colon + 1);
            
            if (users.find(username) != users.end() && users[username] == password) {
                return "SUCCESS:Login successful";
            } else {
                return "ERROR:Invalid username or password";
            }
        }
        
        void handle_client(SOCKET client_socket) {
            char buffer[1024] = {0};
            recv(client_socket, buffer, 1024, 0);
            
            string request(buffer);
            string response = process_request(request);
            
            send(client_socket, response.c_str(), response.length(), 0);
            closesocket(client_socket);
        }
        
    public:
        ChatServer(int port) : user_file_path("users.txt") {
            #ifdef _WIN32
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            #endif
            
            server_socket = socket(AF_INET, SOCK_STREAM, 0);
            
            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = INADDR_ANY;
            server_addr.sin_port = htons(port);
            
            bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
            listen(server_socket, 5);
            
            load_users();
        }
        
        void start() {
            cout << "Server started on port 8080" << endl;
            while (true) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
                
                if (client_socket != INVALID_SOCKET) {
                    thread client_thread(&ChatServer::handle_client, this, client_socket);
                    client_thread.detach();
                }
            }
        }
        
        void stop() {
            closesocket(server_socket);
            #ifdef _WIN32
            WSACleanup();
            #endif
        }
    };
};

int main() {
    cout << "=== Chat System Test Cases ===" << endl << endl;
    
    // Start server in a separate thread
    thread server_thread([]() {
        Task7::ChatServer server(8080);
        server.start();
    });
    server_thread.detach();
    
    // Wait for server to start
    this_thread::sleep_for(chrono::seconds(1));
    
    // Test cases
    string test_cases[][2] = {
        {"alice", "password123"},
        {"bob", "securepass"},
        {"charlie", "wrongpass"},
        {"david", "test1234"},
        {"unknown", "password"}
    };
    
    for (int i = 0; i < 5; i++) {
        try {
            cout << "Test Case " << (i + 1) << ":" << endl;
            cout << "Username: " << test_cases[i][0] << endl;
            cout << "Password: " << test_cases[i][1] << endl;
            
            Task7::ChatClient client("127.0.0.1", 8080);
            string response = client.login(test_cases[i][0], test_cases[i][1]);
            cout << "Response: " << response << endl;
            client.close_connection();
            cout << endl;
            
            this_thread::sleep_for(chrono::milliseconds(500));
        } catch (exception& e) {
            cerr << "Client error: " << e.what() << endl;
        }
    }
    
    return 0;
}
