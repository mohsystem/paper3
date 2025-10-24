
import java.io.*;
import java.net.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;

class Task7 {
    // Server class
    static class ChatServer {
        private static final int PORT = 8888;
        private static final String USER_FILE = "users.txt";
        private ServerSocket serverSocket;
        
        public void start() throws IOException {
            serverSocket = new ServerSocket(PORT);
            System.out.println("Server started on port " + PORT);
            
            while (true) {
                Socket clientSocket = serverSocket.accept();
                new ClientHandler(clientSocket).start();
            }
        }
        
        class ClientHandler extends Thread {
            private Socket socket;
            private BufferedReader in;
            private PrintWriter out;
            
            public ClientHandler(Socket socket) {
                this.socket = socket;
            }
            
            public void run() {
                try {
                    in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                    out = new PrintWriter(socket.getOutputStream(), true);
                    
                    String request = in.readLine();
                    if (request != null && request.startsWith("LOGIN:")) {
                        String[] parts = request.substring(6).split(":", 2);
                        if (parts.length == 2) {
                            String username = sanitizeInput(parts[0]);
                            String password = parts[1];
                            
                            if (authenticateUser(username, password)) {
                                out.println("SUCCESS:Login successful");
                            } else {
                                out.println("FAILURE:Invalid credentials");
                            }
                        } else {
                            out.println("FAILURE:Invalid request format");
                        }
                    } else {
                        out.println("FAILURE:Unknown request");
                    }
                    
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    try {
                        socket.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        
        private String sanitizeInput(String input) {
            if (input == null) return "";
            return input.replaceAll("[^a-zA-Z0-9_]", "").substring(0, Math.min(input.length(), 50));
        }
        
        private boolean authenticateUser(String username, String password) {
            try {
                File file = new File(USER_FILE);
                if (!file.exists()) {
                    initializeUserFile();
                }
                
                BufferedReader reader = new BufferedReader(new FileReader(file));
                String line;
                
                while ((line = reader.readLine()) != null) {
                    String[] parts = line.split(":", 3);
                    if (parts.length == 3) {
                        String storedUser = parts[0];
                        String storedSalt = parts[1];
                        String storedHash = parts[2];
                        
                        if (storedUser.equals(username)) {
                            String hashedPassword = hashPassword(password, storedSalt);
                            reader.close();
                            return hashedPassword.equals(storedHash);
                        }
                    }
                }
                reader.close();
                
            } catch (Exception e) {
                e.printStackTrace();
            }
            return false;
        }
        
        private void initializeUserFile() throws IOException, NoSuchAlgorithmException {
            PrintWriter writer = new PrintWriter(new FileWriter(USER_FILE));
            String[] testUsers = {"alice", "bob", "charlie", "david", "eve"};
            String[] testPasswords = {"password123", "secure456", "test789", "demo000", "chat111"};
            
            for (int i = 0; i < testUsers.length; i++) {
                String salt = generateSalt();
                String hashedPassword = hashPassword(testPasswords[i], salt);
                writer.println(testUsers[i] + ":" + salt + ":" + hashedPassword);
            }
            writer.close();
        }
        
        private String generateSalt() throws NoSuchAlgorithmException {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);
            return Base64.getEncoder().encodeToString(salt);
        }
        
        private String hashPassword(String password, String salt) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                md.update(salt.getBytes(StandardCharsets.UTF_8));
                byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));
                return Base64.getEncoder().encodeToString(hashedPassword);
            } catch (NoSuchAlgorithmException e) {
                throw new RuntimeException(e);
            }
        }
    }
    
    // Client class
    static class ChatClient {
        private String serverAddress;
        private int serverPort;
        
        public ChatClient(String serverAddress, int serverPort) {
            this.serverAddress = serverAddress;
            this.serverPort = serverPort;
        }
        
        public String login(String username, String password) {
            try {
                Socket socket = new Socket(serverAddress, serverPort);
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                
                String request = buildLoginRequest(username, password);
                out.println(request);
                
                String response = in.readLine();
                
                socket.close();
                return response;
                
            } catch (IOException e) {
                return "FAILURE:Connection error - " + e.getMessage();
            }
        }
        
        private String buildLoginRequest(String username, String password) {
            return "LOGIN:" + username + ":" + password;
        }
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        // Start server in a separate thread
        Thread serverThread = new Thread(() -> {
            try {
                ChatServer server = new ChatServer();
                server.start();
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
        serverThread.setDaemon(true);
        serverThread.start();
        
        // Wait for server to start
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        
        // Test cases
        ChatClient client = new ChatClient("localhost", 8888);
        
        System.out.println("Test Case 1 - Valid login (alice):");
        String response1 = client.login("alice", "password123");
        System.out.println(response1);
        
        System.out.println("\\nTest Case 2 - Valid login (bob):");
        String response2 = client.login("bob", "secure456");
        System.out.println(response2);
        
        System.out.println("\\nTest Case 3 - Invalid password:");
        String response3 = client.login("alice", "wrongpassword");
        System.out.println(response3);
        
        System.out.println("\\nTest Case 4 - Non-existent user:");
        String response4 = client.login("hacker", "password");
        System.out.println(response4);
        
        System.out.println("\\nTest Case 5 - Valid login (charlie):");
        String response5 = client.login("charlie", "test789");
        System.out.println(response5);
    }
}
