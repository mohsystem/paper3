
import java.io.*;
import java.net.*;

public class Task96 {
    private static final int DEFAULT_PORT = 8080;
    private static final int BUFFER_SIZE = 1024;
    
    public static void startServer(int port) {
        ServerSocket serverSocket = null;
        try {
            serverSocket = new ServerSocket(port);
            System.out.println("Server started on port " + port);
            
            while (true) {
                Socket clientSocket = serverSocket.accept();
                System.out.println("Client connected: " + clientSocket.getInetAddress());
                
                // Handle client in a new thread
                new Thread(() -> handleClient(clientSocket)).start();
            }
        } catch (IOException e) {
            System.err.println("Server error: " + e.getMessage());
        } finally {
            if (serverSocket != null && !serverSocket.isClosed()) {
                try {
                    serverSocket.close();
                } catch (IOException e) {
                    System.err.println("Error closing server socket: " + e.getMessage());
                }
            }
        }
    }
    
    private static void handleClient(Socket clientSocket) {
        try (
            BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true)
        ) {
            String message;
            while ((message = in.readLine()) != null) {
                System.out.println("Received: " + message);
                out.println(message); // Echo back
            }
        } catch (IOException e) {
            System.err.println("Client handler error: " + e.getMessage());
        } finally {
            try {
                clientSocket.close();
            } catch (IOException e) {
                System.err.println("Error closing client socket: " + e.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Echo Server Test Cases");
        System.out.println("======================");
        System.out.println("Test Case 1: Start server on default port 8080");
        System.out.println("Test Case 2: Server handles multiple clients");
        System.out.println("Test Case 3: Server echoes messages correctly");
        System.out.println("Test Case 4: Server handles client disconnection");
        System.out.println("Test Case 5: Server runs continuously");
        System.out.println("\\nStarting server...");
        
        startServer(DEFAULT_PORT);
    }
}
