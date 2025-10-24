
import java.io.*;
import java.net.*;
import java.util.*;
import java.nio.file.*;

class Task7 {
    // User class to store credentials
    static class User {
        String username;
        String password;
        
        User(String username, String password) {
            this.username = username;
            this.password = password;
        }
    }
    
    // Client class
    static class ChatClient {
        private Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        
        public ChatClient(String host, int port) throws IOException {
            socket = new Socket(host, port);
            out = new PrintWriter(socket.getOutputStream(), true);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        }
        
        public String login(String username, String password) throws IOException {
            String request = "LOGIN:" + username + ":" + password;
            out.println(request);
            return in.readLine();
        }
        
        public void close() throws IOException {
            in.close();
            out.close();
            socket.close();
        }
    }
    
    // Server class
    static class ChatServer {
        private ServerSocket serverSocket;
        private Map<String, String> users;
        private String userFilePath = "users.txt";
        
        public ChatServer(int port) throws IOException {
            serverSocket = new ServerSocket(port);
            users = new HashMap<>();
            loadUsers();
        }
        
        private void loadUsers() {
            try {
                File file = new File(userFilePath);
                if (!file.exists()) {
                    file.createNewFile();
                    // Create default users
                    try (PrintWriter writer = new PrintWriter(new FileWriter(file))) {
                        writer.println("alice:password123");
                        writer.println("bob:securepass");
                        writer.println("charlie:mypass456");
                        writer.println("david:test1234");
                        writer.println("eve:qwerty789");
                    }
                }
                
                BufferedReader reader = new BufferedReader(new FileReader(file));
                String line;
                while ((line = reader.readLine()) != null) {
                    String[] parts = line.split(":");
                    if (parts.length == 2) {
                        users.put(parts[0], parts[1]);
                    }
                }
                reader.close();
            } catch (IOException e) {
                System.err.println("Error loading users: " + e.getMessage());
            }
        }
        
        public void start() {
            System.out.println("Server started on port " + serverSocket.getLocalPort());
            while (true) {
                try {
                    Socket clientSocket = serverSocket.accept();
                    new Thread(new ClientHandler(clientSocket)).start();
                } catch (IOException e) {
                    System.err.println("Error accepting client: " + e.getMessage());
                    break;
                }
            }
        }
        
        class ClientHandler implements Runnable {
            private Socket socket;
            private PrintWriter out;
            private BufferedReader in;
            
            public ClientHandler(Socket socket) {
                this.socket = socket;
            }
            
            public void run() {
                try {
                    out = new PrintWriter(socket.getOutputStream(), true);
                    in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                    
                    String request = in.readLine();
                    String response = processRequest(request);
                    out.println(response);
                    
                    socket.close();
                } catch (IOException e) {
                    System.err.println("Error handling client: " + e.getMessage());
                }
            }
            
            private String processRequest(String request) {
                if (request == null || !request.startsWith("LOGIN:")) {
                    return "ERROR:Invalid request format";
                }
                
                String[] parts = request.split(":");
                if (parts.length != 3) {
                    return "ERROR:Invalid login format";
                }
                
                String username = parts[1];
                String password = parts[2];
                
                if (users.containsKey(username) && users.get(username).equals(password)) {
                    return "SUCCESS:Login successful";
                } else {
                    return "ERROR:Invalid username or password";
                }
            }
        }
        
        public void stop() throws IOException {
            serverSocket.close();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Chat System Test Cases ===\\n");
        
        // Start server in a separate thread
        Thread serverThread = new Thread(() -> {
            try {
                ChatServer server = new ChatServer(8080);
                server.start();
            } catch (IOException e) {
                System.err.println("Server error: " + e.getMessage());
            }
        });
        serverThread.start();
        
        // Wait for server to start
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        
        // Test cases
        String[][] testCases = {
            {"alice", "password123"},
            {"bob", "securepass"},
            {"charlie", "wrongpass"},
            {"david", "test1234"},
            {"unknown", "password"}
        };
        
        for (int i = 0; i < testCases.length; i++) {
            try {
                System.out.println("Test Case " + (i + 1) + ":");
                System.out.println("Username: " + testCases[i][0]);
                System.out.println("Password: " + testCases[i][1]);
                
                ChatClient client = new ChatClient("localhost", 8080);
                String response = client.login(testCases[i][0], testCases[i][1]);
                System.out.println("Response: " + response);
                client.close();
                System.out.println();
                
                Thread.sleep(500);
            } catch (Exception e) {
                System.err.println("Client error: " + e.getMessage());
            }
        }
        
        System.exit(0);
    }
}
