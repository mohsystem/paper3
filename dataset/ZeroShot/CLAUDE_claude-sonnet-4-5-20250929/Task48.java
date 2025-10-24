
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

class Task48 {
    private static final int PORT = 8888;
    private static Set<ClientHandler> clientHandlers = ConcurrentHashMap.newKeySet();
    
    static class ClientHandler implements Runnable {
        private Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        private String clientName;
        
        public ClientHandler(Socket socket) {
            this.socket = socket;
        }
        
        @Override
        public void run() {
            try {
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                out = new PrintWriter(socket.getOutputStream(), true);
                
                // Get client name
                out.println("Enter your name:");
                clientName = in.readLine();
                if (clientName == null || clientName.trim().isEmpty()) {
                    clientName = "Anonymous";
                }
                
                broadcast(clientName + " has joined the chat!", this);
                clientHandlers.add(this);
                
                String message;
                while ((message = in.readLine()) != null) {
                    if (message.equalsIgnoreCase("/quit")) {
                        break;
                    }
                    broadcast(clientName + ": " + message, this);
                }
            } catch (IOException e) {
                System.err.println("Error handling client: " + e.getMessage());
            } finally {
                cleanup();
            }
        }
        
        private void cleanup() {
            clientHandlers.remove(this);
            broadcast(clientName + " has left the chat!", this);
            try {
                if (in != null) in.close();
                if (out != null) out.close();
                if (socket != null) socket.close();
            } catch (IOException e) {
                System.err.println("Error closing resources: " + e.getMessage());
            }
        }
        
        public void sendMessage(String message) {
            if (out != null) {
                out.println(message);
            }
        }
    }
    
    private static void broadcast(String message, ClientHandler sender) {
        System.out.println(message);
        for (ClientHandler client : clientHandlers) {
            client.sendMessage(message);
        }
    }
    
    public static void startServer(int port) {
        System.out.println("Chat server started on port " + port);
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            ExecutorService executor = Executors.newCachedThreadPool();
            
            while (true) {
                Socket clientSocket = serverSocket.accept();
                System.out.println("New client connected: " + clientSocket.getInetAddress());
                executor.execute(new ClientHandler(clientSocket));
            }
        } catch (IOException e) {
            System.err.println("Server error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Chat Server Application ===");
        System.out.println("Test Case 1: Starting server on port " + PORT);
        
        // Start server in a separate thread for demonstration
        Thread serverThread = new Thread(() -> startServer(PORT));
        serverThread.start();
        
        // Note: To fully test this application, you need to run multiple client instances
        // that connect to localhost:8888
        // Test Cases:
        // 1. Start server - Server should listen on specified port
        // 2. Connect client 1 - Should join and receive welcome message
        // 3. Connect client 2 - Both clients should see join notification
        // 4. Client 1 sends message - All clients should receive broadcast
        // 5. Client disconnects - All clients should see leave notification
        
        System.out.println("\\nTo test this application:");
        System.out.println("1. Run this server");
        System.out.println("2. Use telnet or netcat to connect: telnet localhost 8888");
        System.out.println("3. Connect multiple clients and send messages");
        System.out.println("4. Type /quit to disconnect");
    }
}
