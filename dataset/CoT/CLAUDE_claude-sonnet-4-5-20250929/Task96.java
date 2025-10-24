
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;

public class Task96 {
    private static final int BUFFER_SIZE = 8192;
    private static final int TIMEOUT_MS = 30000;
    private static final int MIN_PORT = 1024;
    private static final int MAX_PORT = 65535;

    public static void startServer(int port) {
        if (port < MIN_PORT || port > MAX_PORT) {
            System.err.println("Port must be between " + MIN_PORT + " and " + MAX_PORT);
            return;
        }

        ServerSocket serverSocket = null;
        try {
            serverSocket = new ServerSocket(port);
            serverSocket.setSoTimeout(TIMEOUT_MS);
            System.out.println("Server listening on port " + port);

            while (true) {
                Socket clientSocket = null;
                try {
                    clientSocket = serverSocket.accept();
                    clientSocket.setSoTimeout(TIMEOUT_MS);
                    System.out.println("Client connected: " + clientSocket.getInetAddress());

                    handleClient(clientSocket);
                } catch (SocketTimeoutException e) {
                    System.out.println("Accept timeout, continuing...");
                } catch (IOException e) {
                    System.err.println("Error handling client: " + e.getMessage());
                } finally {
                    if (clientSocket != null && !clientSocket.isClosed()) {
                        try {
                            clientSocket.close();
                        } catch (IOException e) {
                            System.err.println("Error closing client socket: " + e.getMessage());
                        }
                    }
                }
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
        BufferedReader in = null;
        PrintWriter out = null;
        try {
            in = new BufferedReader(new InputStreamReader(
                clientSocket.getInputStream(), StandardCharsets.UTF_8));
            out = new PrintWriter(new OutputStreamWriter(
                clientSocket.getOutputStream(), StandardCharsets.UTF_8), true);

            String message;
            while ((message = in.readLine()) != null) {
                if (message.length() > BUFFER_SIZE) {
                    message = message.substring(0, BUFFER_SIZE);
                }
                System.out.println("Received: " + message);
                out.println(message);
            }
        } catch (IOException e) {
            System.err.println("Client communication error: " + e.getMessage());
        } finally {
            if (out != null) {
                out.close();
            }
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    System.err.println("Error closing input stream: " + e.getMessage());
                }
            }
        }
    }

    public static void main(String[] args) {
        // Test case: Start server on port 8080
        System.out.println("Test 1: Starting echo server on port 8080");
        new Thread(() -> startServer(8080)).start();

        // Give server time to start
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        // Test cases with client connections
        String[] testMessages = {
            "Hello, Server!",
            "Echo test message",
            "Test 123",
            "Special chars: @#$%",
            "Final test message"
        };

        for (int i = 0; i < testMessages.length; i++) {
            final int testNum = i + 2;
            final String msg = testMessages[i];
            System.out.println("\\nTest " + testNum + ": Sending message: " + msg);
            
            try (Socket socket = new Socket("localhost", 8080);
                 PrintWriter out = new PrintWriter(new OutputStreamWriter(
                     socket.getOutputStream(), StandardCharsets.UTF_8), true);
                 BufferedReader in = new BufferedReader(new InputStreamReader(
                     socket.getInputStream(), StandardCharsets.UTF_8))) {
                
                out.println(msg);
                String response = in.readLine();
                System.out.println("Response: " + response);
            } catch (IOException e) {
                System.err.println("Client test error: " + e.getMessage());
            }

            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
}
