import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

public class Task96 {

    public void runServer(int port) {
        // Use try-with-resources to ensure the ServerSocket is closed automatically.
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server is listening on port " + port);

            // The server runs indefinitely, accepting one client at a time.
            while (true) {
                // accept() is a blocking call, waiting for a client to connect.
                try (Socket clientSocket = serverSocket.accept()) {
                    System.out.println("Client connected: " + clientSocket.getInetAddress().getHostAddress());
                    handleClient(clientSocket);
                } catch (IOException e) {
                    System.err.println("Error accepting client connection: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Could not listen on port " + port + ": " + e.getMessage());
        }
    }

    private void handleClient(Socket socket) {
        // Use try-with-resources to ensure streams are closed automatically.
        try (InputStream input = socket.getInputStream();
             OutputStream output = socket.getOutputStream()) {

            byte[] buffer = new byte[4096];
            int bytesRead;

            // Read from the client and echo back until the client closes the connection.
            // read() returns -1 when the end of the stream is reached.
            while ((bytesRead = input.read(buffer)) != -1) {
                // Echo the received data back to the client.
                // Write only the number of bytes that were actually read.
                output.write(buffer, 0, bytesRead);
                
                // For demonstration, print the received message to the server console.
                String received = new String(buffer, 0, bytesRead, StandardCharsets.UTF_8);
                System.out.print("Received from client: " + received); // Using print as messages might not have newlines
            }
        } catch (IOException e) {
            System.err.println("Error handling client: " + e.getMessage());
        } finally {
            System.out.println("\nClient disconnected.");
        }
    }

    public static void main(String[] args) {
        // The server port.
        int port = 8080;
        
        Task96 server = new Task96();
        
        System.out.println("Starting the echo server...");
        // The prompt asks for 5 test cases. A server runs continuously.
        // To test, you can use a client like netcat (nc) or telnet from 5 different
        // terminals, or sequentially after each one disconnects.
        //
        // Example test cases using netcat from a terminal:
        // Test Case 1: echo "Hello Server" | nc localhost 8080
        // Test Case 2: echo "This is a test" | nc localhost 8080
        // Test Case 3: echo "Another line" | nc localhost 8080
        // Test Case 4: echo "A short one" | nc localhost 8080
        // Test Case 5: echo "Final test message" | nc localhost 8080
        
        server.runServer(port);
    }
}