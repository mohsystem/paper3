
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.*;

class Task7 {
    // Server implementation
    static class ChatServer {
        private static final int PORT = 8080;
        private static final String USERS_FILE = "users.dat";
        private static final int MAX_LOGIN_ATTEMPTS = 3;
        private static final long LOCKOUT_TIME = 300000; // 5 minutes
        private static final ConcurrentHashMap<String, Integer> loginAttempts = new ConcurrentHashMap<>();
        private static final ConcurrentHashMap<String, Long> lockedAccounts = new ConcurrentHashMap<>();
        
        public static void startServer() {
            try (ServerSocket serverSocket = new ServerSocket(PORT)) {
                System.out.println("Server started on port " + PORT);
                
                while (true) {
                    Socket clientSocket = serverSocket.accept();
                    new Thread(() -> handleClient(clientSocket)).start();
                }
            } catch (IOException e) {
                System.err.println("Server error: " + e.getMessage());
            }
        }
        
        private static void handleClient(Socket socket) {
            try (
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8));
                PrintWriter out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8), true)
            ) {
                String request = in.readLine();
                if (request == null || request.trim().isEmpty()) {
                    out.println("ERROR:Invalid request");
                    return;
                }
                
                String response = processRequest(request);
                out.println(response);
                
            } catch (IOException e) {
                System.err.println("Client handling error: " + e.getMessage());
            } finally {
                try {
                    socket.close();
                } catch (IOException e) {
                    System.err.println("Socket close error: " + e.getMessage());
                }
            }
        }
        
        private static String processRequest(String request) {
            // Input validation
            if (request == null || request.length() > 1000) {
                return "ERROR:Invalid request format";
            }
            
            String[] parts = request.split(":", 3);
            if (parts.length != 3 || !parts[0].equals("LOGIN")) {
                return "ERROR:Invalid request format";
            }
            
            String username = sanitizeInput(parts[1]);
            String password = parts[2];
            
            // Validate username format
            if (!isValidUsername(username)) {
                return "ERROR:Invalid username format";
            }
            
            // Check if account is locked
            if (isAccountLocked(username)) {
                return "ERROR:Account temporarily locked";
            }
            
            // Authenticate user
            if (authenticateUser(username, password)) {
                loginAttempts.remove(username);
                return "SUCCESS:Login successful";
            } else {
                incrementLoginAttempts(username);
                return "ERROR:Invalid credentials";
            }
        }
        
        private static boolean isAccountLocked(String username) {
            Long lockoutTime = lockedAccounts.get(username);
            if (lockoutTime != null) {
                if (System.currentTimeMillis() < lockoutTime) {
                    return true;
                } else {
                    lockedAccounts.remove(username);
                    loginAttempts.remove(username);
                }
            }
            return false;
        }
        
        private static void incrementLoginAttempts(String username) {
            int attempts = loginAttempts.getOrDefault(username, 0) + 1;
            loginAttempts.put(username, attempts);
            
            if (attempts >= MAX_LOGIN_ATTEMPTS) {
                lockedAccounts.put(username, System.currentTimeMillis() + LOCKOUT_TIME);
            }
        }
        
        private static boolean isValidUsername(String username) {
            if (username == null || username.isEmpty() || username.length() > 50) {
                return false;
            }
            return username.matches("^[a-zA-Z0-9_]+$");
        }
        
        private static String sanitizeInput(String input) {
            if (input == null) return "";
            return input.replaceAll("[^a-zA-Z0-9_]", "");
        }
        
        private static boolean authenticateUser(String username, String password) {
            try {
                File usersFile = new File(USERS_FILE);
                if (!usersFile.exists()) {
                    initializeUsersFile();
                }
                
                try (BufferedReader reader = new BufferedReader(new FileReader(usersFile, StandardCharsets.UTF_8))) {
                    String line;
                    while ((line = reader.readLine()) != null) {
                        String[] parts = line.split(":");
                        if (parts.length == 3) {
                            String storedUsername = parts[0];
                            String storedSalt = parts[1];
                            String storedHash = parts[2];
                            
                            if (storedUsername.equals(username)) {
                                String computedHash = hashPassword(password, storedSalt);
                                return MessageDigest.isEqual(
                                    computedHash.getBytes(StandardCharsets.UTF_8),
                                    storedHash.getBytes(StandardCharsets.UTF_8)
                                );
                            }
                        }
                    }
                }
            } catch (IOException e) {
                System.err.println("Authentication error: " + e.getMessage());
            }
            return false;
        }
        
        private static void initializeUsersFile() {
            try (PrintWriter writer = new PrintWriter(new FileWriter(USERS_FILE, StandardCharsets.UTF_8))) {
                // Create test users with hashed passwords
                String[][] users = {
                    {"user1", "password1"},
                    {"user2", "password2"},
                    {"admin", "admin123"},
                    {"testuser", "test123"},
                    {"alice", "alice456"}
                };
                
                for (String[] user : users) {
                    String salt = generateSalt();
                    String hash = hashPassword(user[1], salt);
                    writer.println(user[0] + ":" + salt + ":" + hash);
                }
            } catch (IOException e) {
                System.err.println("Error initializing users file: " + e.getMessage());
            }
        }
        
        private static String generateSalt() {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);
            return Base64.getEncoder().encodeToString(salt);
        }
        
        private static String hashPassword(String password, String salt) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                md.update(salt.getBytes(StandardCharsets.UTF_8));
                byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));
                return Base64.getEncoder().encodeToString(hashedPassword);
            } catch (NoSuchAlgorithmException e) {
                throw new RuntimeException("SHA-256 algorithm not found", e);
            }
        }
    }
    
    // Client implementation
    static class ChatClient {
        private static final String SERVER_HOST = "localhost";
        private static final int SERVER_PORT = 8080;
        private static final int CONNECTION_TIMEOUT = 5000;
        
        public static String login(String username, String password) {
            // Input validation
            if (!isValidInput(username) || !isValidInput(password)) {
                return "ERROR:Invalid input format";
            }
            
            if (username.length() > 50 || password.length() > 100) {
                return "ERROR:Input too long";
            }
            
            String request = buildLoginRequest(username, password);
            return sendRequest(request);
        }
        
        private static boolean isValidInput(String input) {
            return input != null && !input.isEmpty() && !input.contains("\\n") && !input.contains("\\r");
        }
        
        private static String buildLoginRequest(String username, String password) {
            // Sanitize username but preserve password as-is for authentication
            String sanitizedUsername = username.replaceAll("[^a-zA-Z0-9_]", "");
            return "LOGIN:" + sanitizedUsername + ":" + password;
        }
        
        private static String sendRequest(String request) {
            try (Socket socket = new Socket()) {
                socket.connect(new InetSocketAddress(SERVER_HOST, SERVER_PORT), CONNECTION_TIMEOUT);
                socket.setSoTimeout(CONNECTION_TIMEOUT);
                
                try (
                    PrintWriter out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8), true);
                    BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8))
                ) {
                    out.println(request);
                    String response = in.readLine();
                    return response != null ? response : "ERROR:No response from server";
                }
            } catch (IOException e) {
                return "ERROR:Connection failed - " + e.getMessage();
            }
        }
        
        public static void interactiveLogin() {
            try (Scanner scanner = new Scanner(System.in)) {
                System.out.print("Enter username: ");
                String username = scanner.nextLine().trim();
                
                System.out.print("Enter password: ");
                String password = scanner.nextLine();
                
                String response = login(username, password);
                System.out.println("Server response: " + response);
            }
        }
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        // Start server in a separate thread
        Thread serverThread = new Thread(() -> ChatServer.startServer());
        serverThread.setDaemon(true);
        serverThread.start();
        
        // Wait for server to start
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        
        // Test cases
        System.out.println("=== Running Test Cases ===\\n");
        
        // Test case 1: Valid login
        System.out.println("Test 1 - Valid user login:");
        String response1 = ChatClient.login("user1", "password1");
        System.out.println("Response: " + response1);
        System.out.println();
        
        // Test case 2: Invalid password
        System.out.println("Test 2 - Invalid password:");
        String response2 = ChatClient.login("user1", "wrongpassword");
        System.out.println("Response: " + response2);
        System.out.println();
        
        // Test case 3: Non-existent user
        System.out.println("Test 3 - Non-existent user:");
        String response3 = ChatClient.login("nonexistent", "password");
        System.out.println("Response: " + response3);
        System.out.println();
        
        // Test case 4: Valid admin login
        System.out.println("Test 4 - Admin login:");
        String response4 = ChatClient.login("admin", "admin123");
        System.out.println("Response: " + response4);
        System.out.println();
        
        // Test case 5: Empty credentials
        System.out.println("Test 5 - Empty credentials:");
        String response5 = ChatClient.login("", "");
        System.out.println("Response: " + response5);
        System.out.println();
    }
}
