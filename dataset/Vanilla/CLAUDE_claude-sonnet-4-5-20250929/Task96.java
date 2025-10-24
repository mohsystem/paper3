
import java.io.*;
import java.net.*;

public class Task96 {
    private static final int PORT = 8888;
    
    public static void startServer(int port) {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server started on port " + port);
            
            while (true) {
                try (Socket clientSocket = serverSocket.accept();
                     BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                     PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true)) {
                    
                    System.out.println("Client connected: " + clientSocket.getInetAddress());
                    
                    String message;
                    while ((message = in.readLine()) != null) {
                        System.out.println("Received: " + message);
                        out.println(message);
                        
                        if ("QUIT".equalsIgnoreCase(message)) {
                            break;
                        }
                    }
                    
                    System.out.println("Client disconnected");
                } catch (IOException e) {
                    System.err.println("Error handling client: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Could not start server: " + e.getMessage());
        }
    }
    
    public static String sendMessage(int port, String message) {
        try (Socket socket = new Socket("localhost", port);
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))) {
            
            out.println(message);
            return in.readLine();
        } catch (IOException e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Start server in a separate thread
        Thread serverThread = new Thread(() -> startServer(PORT));
        serverThread.setDaemon(true);
        serverThread.start();
        
        // Wait for server to start
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        
        // Test cases
        System.out.println("\\n=== Running Test Cases ===\\n");
        
        // Test 1: Simple message
        System.out.println("Test 1: Simple message");
        String response1 = sendMessage(PORT, "Hello, Server!");
        System.out.println("Sent: Hello, Server!");
        System.out.println("Echo: " + response1);
        System.out.println();
        
        // Test 2: Numeric message
        System.out.println("Test 2: Numeric message");
        String response2 = sendMessage(PORT, "12345");
        System.out.println("Sent: 12345");
        System.out.println("Echo: " + response2);
        System.out.println();
        
        // Test 3: Special characters
        System.out.println("Test 3: Special characters");
        String response3 = sendMessage(PORT, "Test@#$%^&*()");
        System.out.println("Sent: Test@#$%^&*()");
        System.out.println("Echo: " + response3);
        System.out.println();
        
        // Test 4: Empty-like message
        System.out.println("Test 4: Space message");
        String response4 = sendMessage(PORT, " ");
        System.out.println("Sent: ' '");
        System.out.println("Echo: '" + response4 + "'");
        System.out.println();
        
        // Test 5: Long message
        System.out.println("Test 5: Long message");
        String longMsg = "This is a longer message to test the echo functionality!";
        String response5 = sendMessage(PORT, longMsg);
        System.out.println("Sent: " + longMsg);
        System.out.println("Echo: " + response5);
        System.out.println();
        
        System.out.println("=== All Tests Completed ===");
    }
}
