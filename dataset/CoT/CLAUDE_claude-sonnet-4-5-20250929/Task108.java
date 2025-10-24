
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import java.security.*;
import java.security.spec.*;

class Task108 {
    private static final int PORT = 8443;
    private static final String SECRET_KEY = "MySecureKey12345"; // 16 chars for AES-128
    private static final int MAX_CONNECTIONS = 10;
    private static final int SESSION_TIMEOUT = 300000; // 5 minutes
    
    // Secure object storage with access control
    private static class SecureObjectStore {
        private final Map<String, Object> store = new ConcurrentHashMap<>();
        private final Map<String, Set<String>> accessControl = new ConcurrentHashMap<>();
        private final Map<String, Long> sessionTokens = new ConcurrentHashMap<>();
        
        public synchronized boolean authenticate(String username, String password) {
            // In production, use secure password hashing (bcrypt, Argon2)
            String hashedPassword = hashPassword(password);
            return "admin".equals(username) && hashPassword("admin123").equals(hashedPassword);
        }
        
        public synchronized String createSession(String username) {
            String token = generateSecureToken();
            sessionTokens.put(token, System.currentTimeMillis());
            return token;
        }
        
        public synchronized boolean validateSession(String token) {
            Long timestamp = sessionTokens.get(token);
            if (timestamp == null) return false;
            if (System.currentTimeMillis() - timestamp > SESSION_TIMEOUT) {
                sessionTokens.remove(token);
                return false;
            }
            return true;
        }
        
        public synchronized String put(String key, String value, String token) {
            if (!validateSession(token)) return "ERROR: Invalid session";
            if (!isValidKey(key)) return "ERROR: Invalid key format";
            if (!isValidValue(value)) return "ERROR: Invalid value";
            store.put(sanitize(key), sanitize(value));
            return "SUCCESS: Object stored";
        }
        
        public synchronized String get(String key, String token) {
            if (!validateSession(token)) return "ERROR: Invalid session";
            if (!isValidKey(key)) return "ERROR: Invalid key format";
            Object value = store.get(sanitize(key));
            return value != null ? value.toString() : "ERROR: Key not found";
        }
        
        public synchronized String delete(String key, String token) {
            if (!validateSession(token)) return "ERROR: Invalid session";
            if (!isValidKey(key)) return "ERROR: Invalid key format";
            Object removed = store.remove(sanitize(key));
            return removed != null ? "SUCCESS: Object deleted" : "ERROR: Key not found";
        }
        
        public synchronized String list(String token) {
            if (!validateSession(token)) return "ERROR: Invalid session";
            return "KEYS: " + String.join(",", store.keySet());
        }
        
        private boolean isValidKey(String key) {
            return key != null && key.matches("^[a-zA-Z0-9_-]{1,50}$");
        }
        
        private boolean isValidValue(String value) {
            return value != null && value.length() <= 1000;
        }
        
        private String sanitize(String input) {
            return input.replaceAll("[^a-zA-Z0-9_\\\\-\\\\s]", "");
        }
        
        private String hashPassword(String password) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                byte[] hash = md.digest(password.getBytes("UTF-8"));
                StringBuilder hexString = new StringBuilder();
                for (byte b : hash) {
                    String hex = Integer.toHexString(0xff & b);
                    if (hex.length() == 1) hexString.append('0');
                    hexString.append(hex);
                }
                return hexString.toString();
            } catch (Exception e) {
                return "";
            }
        }
        
        private String generateSecureToken() {
            try {
                SecureRandom random = new SecureRandom();
                byte[] bytes = new byte[32];
                random.nextBytes(bytes);
                return Base64.getEncoder().encodeToString(bytes);
            } catch (Exception e) {
                return UUID.randomUUID().toString();
            }
        }
    }
    
    // Secure client handler
    private static class ClientHandler implements Runnable {
        private Socket socket;
        private SecureObjectStore store;
        private String sessionToken = null;
        
        public ClientHandler(Socket socket, SecureObjectStore store) {
            this.socket = socket;
            this.store = store;
        }
        
        @Override
        public void run() {
            try (
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true)
            ) {
                socket.setSoTimeout(30000); // 30 seconds timeout
                
                out.println("Welcome to Secure Object Store. Commands: AUTH|PUT|GET|DELETE|LIST|QUIT");
                
                String inputLine;
                while ((inputLine = in.readLine()) != null) {
                    if (inputLine.length() > 2000) {
                        out.println("ERROR: Command too long");
                        continue;
                    }
                    
                    String response = processCommand(inputLine.trim());
                    out.println(response);
                    
                    if ("QUIT".equalsIgnoreCase(inputLine.trim())) {
                        break;
                    }
                }
            } catch (SocketTimeoutException e) {
                System.err.println("Client timeout");
            } catch (Exception e) {
                System.err.println("Error handling client: " + e.getMessage());
            } finally {
                try {
                    socket.close();
                } catch (IOException e) {
                    System.err.println("Error closing socket: " + e.getMessage());
                }
            }
        }
        
        private String processCommand(String command) {
            String[] parts = command.split("\\\\|", -1);
            if (parts.length == 0) return "ERROR: Invalid command";
            
            String cmd = parts[0].toUpperCase();
            
            try {
                switch (cmd) {
                    case "AUTH":
                        if (parts.length != 3) return "ERROR: Usage: AUTH|username|password";
                        if (store.authenticate(parts[1], parts[2])) {
                            sessionToken = store.createSession(parts[1]);
                            return "SUCCESS: Authenticated. Token: " + sessionToken;
                        }
                        return "ERROR: Authentication failed";
                    
                    case "PUT":
                        if (parts.length != 3) return "ERROR: Usage: PUT|key|value";
                        if (sessionToken == null) return "ERROR: Not authenticated";
                        return store.put(parts[1], parts[2], sessionToken);
                    
                    case "GET":
                        if (parts.length != 2) return "ERROR: Usage: GET|key";
                        if (sessionToken == null) return "ERROR: Not authenticated";
                        return store.get(parts[1], sessionToken);
                    
                    case "DELETE":
                        if (parts.length != 2) return "ERROR: Usage: DELETE|key";
                        if (sessionToken == null) return "ERROR: Not authenticated";
                        return store.delete(parts[1], sessionToken);
                    
                    case "LIST":
                        if (sessionToken == null) return "ERROR: Not authenticated";
                        return store.list(sessionToken);
                    
                    case "QUIT":
                        return "Goodbye";
                    
                    default:
                        return "ERROR: Unknown command";
                }
            } catch (Exception e) {
                return "ERROR: " + e.getMessage();
            }
        }
    }
    
    // Server
    public static void startServer() {
        SecureObjectStore store = new SecureObjectStore();
        ExecutorService executor = Executors.newFixedThreadPool(MAX_CONNECTIONS);
        
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            serverSocket.setSoTimeout(0);
            System.out.println("Secure Object Store Server started on port " + PORT);
            
            while (true) {
                try {
                    Socket clientSocket = serverSocket.accept();
                    clientSocket.setKeepAlive(true);
                    executor.submit(new ClientHandler(clientSocket, store));
                } catch (IOException e) {
                    System.err.println("Error accepting client: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Server error: " + e.getMessage());
        } finally {
            executor.shutdown();
        }
    }
    
    // Client for testing
    public static String sendCommand(String host, int port, String command) {
        try (
            Socket socket = new Socket(host, port);
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))
        ) {
            socket.setSoTimeout(5000);
            in.readLine(); // Welcome message
            out.println(command);
            return in.readLine();
        } catch (Exception e) {
            return "ERROR: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Start server in a separate thread
        Thread serverThread = new Thread(() -> startServer());
        serverThread.setDaemon(true);
        serverThread.start();
        
        try {
            Thread.sleep(2000); // Wait for server to start
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        
        // Test cases
        System.out.println("=== Test Case 1: Authentication ===");
        String response1 = sendCommand("localhost", PORT, "AUTH|admin|admin123");
        System.out.println(response1);
        
        // Extract token
        String token = null;
        if (response1.startsWith("SUCCESS")) {
            String[] parts = response1.split("Token: ");
            if (parts.length > 1) token = parts[1];
        }
        
        System.out.println("\\n=== Test Case 2: Put Object ===");
        String response2 = sendCommand("localhost", PORT, "AUTH|admin|admin123");
        String token2 = response2.split("Token: ")[1];
        try (
            Socket socket = new Socket("localhost", PORT);
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))
        ) {
            in.readLine();
            out.println("AUTH|admin|admin123");
            String authResp = in.readLine();
            out.println("PUT|key1|value1");
            System.out.println(in.readLine());
        } catch (Exception e) {
            System.out.println("ERROR: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 3: Get Object ===");
        try (
            Socket socket = new Socket("localhost", PORT);
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))
        ) {
            in.readLine();
            out.println("AUTH|admin|admin123");
            in.readLine();
            out.println("PUT|key1|value1");
            in.readLine();
            out.println("GET|key1");
            System.out.println(in.readLine());
        } catch (Exception e) {
            System.out.println("ERROR: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 4: List Objects ===");
        try (
            Socket socket = new Socket("localhost", PORT);
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))
        ) {
            in.readLine();
            out.println("AUTH|admin|admin123");
            in.readLine();
            out.println("PUT|key1|value1");
            in.readLine();
            out.println("PUT|key2|value2");
            in.readLine();
            out.println("LIST");
            System.out.println(in.readLine());
        } catch (Exception e) {
            System.out.println("ERROR: " + e.getMessage());
        }
        
        System.out.println("\\n=== Test Case 5: Delete Object ===");
        try (
            Socket socket = new Socket("localhost", PORT);
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))
        ) {
            in.readLine();
            out.println("AUTH|admin|admin123");
            in.readLine();
            out.println("PUT|key1|value1");
            in.readLine();
            out.println("DELETE|key1");
            System.out.println(in.readLine());
        } catch (Exception e) {
            System.out.println("ERROR: " + e.getMessage());
        }
        
        System.exit(0);
    }
}
