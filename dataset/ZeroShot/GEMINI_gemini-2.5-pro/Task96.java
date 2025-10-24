import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;

public class Task96 {

    /**
     * Starts a secure echo server on the specified port.
     * The server handles a limited number of clients sequentially and then shuts down.
     *
     * @param port The port number to listen on.
     * @param maxClients The number of clients to serve before shutting down.
     */
    public static void startServer(int port, int maxClients) {
        // Use try-with-resources to ensure the ServerSocket is always closed.
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server is listening on port " + port);
            System.out.println("Server will handle " + maxClients + " clients and then shut down.");

            for (int i = 0; i < maxClients; i++) {
                System.out.println("\nWaiting for client " + (i + 1) + "/" + maxClients + "...");
                try {
                    // Accept a client connection. This is a blocking call.
                    // Use try-with-resources to ensure the client socket is always closed.
                    try (Socket clientSocket = serverSocket.accept()) {
                        System.out.println("Client connected: " + clientSocket.getInetAddress().getHostAddress());

                        // Set a read timeout (e.g., 10 seconds) to prevent clients
                        // from holding connections open indefinitely (Denial of Service).
                        clientSocket.setSoTimeout(10000);

                        // Use try-with-resources for streams to ensure they are closed.
                        try (InputStream input = clientSocket.getInputStream();
                             OutputStream output = clientSocket.getOutputStream()) {

                            byte[] buffer = new byte[4096];
                            int bytesRead;

                            // Read from the client and echo back until the client closes the connection.
                            while ((bytesRead = input.read(buffer)) != -1) {
                                // Echo the received data back to the client.
                                // Only write the number of bytes that were actually read.
                                output.write(buffer, 0, bytesRead);
                                output.flush();
                                System.out.println("Echoed " + bytesRead + " bytes to client.");
                            }
                        }
                        System.out.println("Client disconnected.");
                    }
                } catch (SocketTimeoutException e) {
                    System.err.println("Client connection timed out. Closing connection.");
                } catch (IOException e) {
                    System.err.println("An I/O error occurred with a client: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Server error: Could not listen on port " + port);
            e.printStackTrace();
        } finally {
            System.out.println("Server shutting down after handling " + maxClients + " clients.");
        }
    }

    public static void main(String[] args) {
        // The port number. Must be above 1024 for non-root users.
        int port = 8080;
        int numberOfTestConnections = 5;
        
        System.out.println("--- Echo Server Test ---");
        System.out.println("To test, connect to this server using a client like netcat or telnet.");
        System.out.println("Example 1: nc localhost " + port);
        System.out.println("Example 2: telnet localhost " + port);
        System.out.println("Type a message and press Enter. The server will echo it back.");
        System.out.println("The server will shut down after handling " + numberOfTestConnections + " connections.");
        System.out.println("-------------------------");

        // Run the server to handle 5 sequential connections as test cases.
        startServer(port, numberOfTestConnections);
    }
}