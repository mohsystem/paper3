
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import java.security.*;

class Task108 {
    private static final int PORT = 8080;
    private static final String SECRET_KEY = "MySecretKey12345"; // 16 chars for AES-128
    private static Map<String, String> serverObjects = new ConcurrentHashMap<>();
    private static Set<String> authenticatedClients = ConcurrentHashMap.newKeySet();
    
    // Server methods
    public static void startServer() {
        serverObjects.put("object1", "value1");
        serverObjects.put("object2", "value2");
        
        new Thread(() -> {
            try (ServerSocket serverSocket = new ServerSocket(PORT)) {
                System.out.println("Server started on port " + PORT);
                while (true) {
                    Socket clientSocket = serverSocket.accept();
                    new Thread(() -> handleClient(clientSocket)).start();
                }
            } catch (IOException e) {
                System.err.println("Server error: " + e.getMessage());
            }
        }).start();
    }
    
    private static void handleClient(Socket socket) {
        try (BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true)) {
            
            String clientId = socket.getInetAddress().toString();
            String request = in.readLine();
            
            if (request == null) return;
            
            String[] parts = request.split("\\\\|");
            String command = parts[0];
            
            // Authentication
            if (command.equals("AUTH") && parts.length == 2) {
                if (authenticate(parts[1])) {
                    authenticatedClients.add(clientId);
                    out.println("AUTH_SUCCESS");
                } else {
                    out.println("AUTH_FAILED");
                }
                return;
            }
            
            // Check authentication for other commands
            if (!authenticatedClients.contains(clientId)) {
                out.println("ERROR|Not authenticated");
                return;
            }
            
            String response = processCommand(parts);
            out.println(response);
            
        } catch (Exception e) {
            System.err.println("Client handling error: " + e.getMessage());
        }
    }
    
    private static boolean authenticate(String token) {
        // Simple authentication - in production use proper authentication
        return token.equals(SECRET_KEY);
    }
    
    private static String processCommand(String[] parts) {
        String command = parts[0];
        
        try {
            switch (command) {
                case "GET":
                    if (parts.length == 2) {
                        String key = sanitizeInput(parts[1]);
                        String value = serverObjects.get(key);
                        return value != null ? "SUCCESS|" + value : "ERROR|Object not found";
                    }
                    return "ERROR|Invalid GET command";
                    
                case "SET":
                    if (parts.length == 3) {
                        String key = sanitizeInput(parts[1]);
                        String value = sanitizeInput(parts[2]);
                        serverObjects.put(key, value);
                        return "SUCCESS|Object set";
                    }
                    return "ERROR|Invalid SET command";
                    
                case "DELETE":
                    if (parts.length == 2) {
                        String key = sanitizeInput(parts[1]);
                        serverObjects.remove(key);
                        return "SUCCESS|Object deleted";
                    }
                    return "ERROR|Invalid DELETE command";
                    
                case "LIST":
                    return "SUCCESS|" + String.join(",", serverObjects.keySet());
                    
                default:
                    return "ERROR|Unknown command";
            }
        } catch (Exception e) {
            return "ERROR|" + e.getMessage();
        }
    }
    
    private static String sanitizeInput(String input) {
        // Remove potentially dangerous characters
        return input.replaceAll("[^a-zA-Z0-9_-]", "");
    }
    
    // Client methods
    public static String sendRequest(String host, int port, String request) {
        try (Socket socket = new Socket(host, port);
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))) {
            
            out.println(request);
            return in.readLine();
        } catch (IOException e) {
            return "ERROR|Connection failed: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Remote Object Access System ===\\n");
        
        // Start server
        startServer();
        
        // Wait for server to start
        try { Thread.sleep(1000); } catch (InterruptedException e) {}
        
        // Test Case 1: Authentication
        System.out.println("Test 1 - Authentication:");
        String response = sendRequest("localhost", PORT, "AUTH|" + SECRET_KEY);
        System.out.println("Response: " + response);
        System.out.println();
        
        // Test Case 2: Get object
        System.out.println("Test 2 - Get object:");
        response = sendRequest("localhost", PORT, "GET|object1");
        System.out.println("Response: " + response);
        System.out.println();
        
        // Test Case 3: Set object
        System.out.println("Test 3 - Set object:");
        response = sendRequest("localhost", PORT, "SET|object3|value3");
        System.out.println("Response: " + response);
        System.out.println();
        
        // Test Case 4: List objects
        System.out.println("Test 4 - List objects:");
        response = sendRequest("localhost", PORT, "LIST");
        System.out.println("Response: " + response);
        System.out.println();
        
        // Test Case 5: Delete object
        System.out.println("Test 5 - Delete object:");
        response = sendRequest("localhost", PORT, "DELETE|object2");
        System.out.println("Response: " + response);
        System.out.println();
        
        System.exit(0);
    }
}
