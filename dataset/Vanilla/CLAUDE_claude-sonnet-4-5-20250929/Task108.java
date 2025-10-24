
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

class Task108 {
    private static Map<String, Object> objectStore = new ConcurrentHashMap<>();
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("=== Test Case 1: Store and Retrieve ===");
        storeObject("key1", "Hello World");
        System.out.println("Retrieved: " + retrieveObject("key1"));
        
        System.out.println("\\n=== Test Case 2: Store Multiple Objects ===");
        storeObject("key2", 12345);
        storeObject("key3", new double[]{1.1, 2.2, 3.3});
        System.out.println("Retrieved key2: " + retrieveObject("key2"));
        System.out.println("Retrieved key3: " + Arrays.toString((double[])retrieveObject("key3")));
        
        System.out.println("\\n=== Test Case 3: Update Object ===");
        storeObject("key1", "Updated Value");
        System.out.println("Updated key1: " + retrieveObject("key1"));
        
        System.out.println("\\n=== Test Case 4: Delete Object ===");
        boolean deleted = deleteObject("key2");
        System.out.println("Deleted key2: " + deleted);
        System.out.println("Retrieve deleted key2: " + retrieveObject("key2"));
        
        System.out.println("\\n=== Test Case 5: List All Keys ===");
        System.out.println("All keys: " + listKeys());
        
        // Start server in a separate thread for demonstration
        Thread serverThread = new Thread(() -> {
            try {
                startServer(8080);
            } catch (IOException e) {
                e.printStackTrace();
            }
        });
        serverThread.setDaemon(true);
        serverThread.start();
        
        System.out.println("\\nServer started on port 8080. Press Ctrl+C to stop.");
    }
    
    public static void storeObject(String key, Object value) {
        objectStore.put(key, value);
    }
    
    public static Object retrieveObject(String key) {
        return objectStore.get(key);
    }
    
    public static boolean deleteObject(String key) {
        return objectStore.remove(key) != null;
    }
    
    public static Set<String> listKeys() {
        return new HashSet<>(objectStore.keySet());
    }
    
    public static void startServer(int port) throws IOException {
        ServerSocket serverSocket = new ServerSocket(port);
        System.out.println("Server listening on port " + port);
        
        while (true) {
            Socket clientSocket = serverSocket.accept();
            new Thread(new ClientHandler(clientSocket)).start();
        }
    }
    
    static class ClientHandler implements Runnable {
        private Socket clientSocket;
        
        public ClientHandler(Socket socket) {
            this.clientSocket = socket;
        }
        
        @Override
        public void run() {
            try (BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                 PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true)) {
                
                String request = in.readLine();
                if (request == null) return;
                
                String[] parts = request.split("\\\\|");
                String command = parts[0];
                String response = "";
                
                switch (command) {
                    case "STORE":
                        if (parts.length >= 3) {
                            storeObject(parts[1], parts[2]);
                            response = "OK|Object stored";
                        } else {
                            response = "ERROR|Invalid parameters";
                        }
                        break;
                    case "RETRIEVE":
                        if (parts.length >= 2) {
                            Object value = retrieveObject(parts[1]);
                            response = value != null ? "OK|" + value : "ERROR|Key not found";
                        } else {
                            response = "ERROR|Invalid parameters";
                        }
                        break;
                    case "DELETE":
                        if (parts.length >= 2) {
                            boolean deleted = deleteObject(parts[1]);
                            response = deleted ? "OK|Object deleted" : "ERROR|Key not found";
                        } else {
                            response = "ERROR|Invalid parameters";
                        }
                        break;
                    case "LIST":
                        response = "OK|" + listKeys();
                        break;
                    default:
                        response = "ERROR|Unknown command";
                }
                
                out.println(response);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
