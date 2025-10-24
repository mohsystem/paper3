import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.atomic.AtomicBoolean;

public class Task96 {

    // A flag to signal the server to stop
    private static final AtomicBoolean serverRunning = new AtomicBoolean(true);

    /**
     * Starts a simple echo server on the specified port.
     * The server handles one client connection and then shuts down for this example.
     * @param port The port number to listen on.
     */
    public static void startEchoServer(int port) {
        // Use try-with-resources to ensure the ServerSocket is closed automatically
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server is listening on port " + port);
            
            // Set a timeout to unblock accept() and check the running flag
            serverSocket.setSoTimeout(1000); 

            while (serverRunning.get()) {
                try {
                    Socket clientSocket = serverSocket.accept();
                    System.out.println("Client connected: " + clientSocket.getInetAddress().getHostAddress());

                    // Use try-with-resources for client resources
                    try (
                        PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true);
                        BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()))
                    ) {
                        String inputLine;
                        while ((inputLine = in.readLine()) != null) {
                            System.out.println("Received from client: " + inputLine);
                            out.println(inputLine); // Echo the message back
                            System.out.println("Echoed to client: " + inputLine);
                        }
                    } catch (IOException e) {
                        System.err.println("Exception handling client " + clientSocket.getInetAddress() + ": " + e.getMessage());
                    } finally {
                        System.out.println("Client disconnected.");
                        // For this test, stop the server after handling one client.
                        // In a real-world scenario, you would remove this line to handle multiple clients.
                        serverRunning.set(false);
                    }
                } catch (java.net.SocketTimeoutException e) {
                    // Timeout occurred, loop again to check the serverRunning flag
                }
            }
        } catch (IOException e) {
            System.err.println("Could not listen on port " + port + ": " + e.getMessage());
        } finally {
            System.out.println("Server has shut down.");
        }
    }

    /**
     * Runs 5 test cases by connecting to the server and sending messages.
     * @param host The hostname or IP of the server.
     * @param port The port number of the server.
     */
    public static void runClientTests(String host, int port) {
        String[] testMessages = {
            "Hello, Server!",
            "This is test case 2.",
            "A message with numbers 12345.",
            "Another test.",
            "Goodbye!"
        };

        // Use try-with-resources to ensure the client socket and streams are closed.
        try (
            Socket socket = new Socket(host, port);
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))
        ) {
            System.out.println("\nClient connected to server. Running tests...");
            for (String message : testMessages) {
                System.out.println("Client sending: " + message);
                out.println(message); // Send message to server
                String response = in.readLine(); // Read echo from server
                System.out.println("Client received echo: " + response);
                if (!message.equals(response)) {
                    System.err.println("Test Failed: Mismatch in echoed message!");
                }
            }
        } catch (IOException e) {
            System.err.println("Client error: " + e.getMessage());
        } finally {
             System.out.println("Client tests finished.");
        }
    }

    public static void main(String[] args) {
        final int PORT = 12345;
        final String HOST = "127.0.0.1";

        // Create and start the server in a new thread
        Thread serverThread = new Thread(() -> startEchoServer(PORT));
        serverThread.start();

        // Give the server a moment to start up
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.err.println("Main thread interrupted.");
        }

        // Run client tests from the main thread
        runClientTests(HOST, PORT);

        // Wait for the server thread to finish
        try {
            serverThread.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.err.println("Main thread interrupted while waiting for server to shut down.");
        }
        
        System.out.println("\nProgram finished.");
    }
}