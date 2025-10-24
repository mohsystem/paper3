import java.io.*;
import java.net.*;
import java.util.Map;
import java.util.concurrent.*;

public class Task108 {

    // A simple, thread-safe remote object server.
    // NOTE: This implementation does not use TLS/SSL for encrypted communication
    // or any form of authentication. In a production environment, these are critical.
    static class RemoteObjectServer implements Runnable {
        private final int port;
        private ServerSocket serverSocket;
        private final Map<String, String> remoteObject;
        private final ExecutorService pool;
        private volatile boolean running = true;

        public RemoteObjectServer(int port) {
            this.port = port;
            // Use ConcurrentHashMap for thread-safe operations on the shared object.
            // This mitigates TOCTOU vulnerabilities by using atomic operations.
            this.remoteObject = new ConcurrentHashMap<>();
            this.pool = Executors.newCachedThreadPool();
        }

        public void stop() {
            running = false;
            pool.shutdown();
            try {
                if (serverSocket != null && !serverSocket.isClosed()) {
                    serverSocket.close();
                }
            } catch (IOException e) {
                // This error is expected during shutdown, so we can ignore it.
            }
        }

        @Override
        public void run() {
            try {
                serverSocket = new ServerSocket(port);
                System.out.println("Server listening on port " + port);
                while (running) {
                    try {
                        Socket clientSocket = serverSocket.accept();
                        pool.execute(new ClientHandler(clientSocket, remoteObject));
                    } catch (IOException e) {
                        if (running) {
                            System.err.println("Error accepting client connection: " + e.getMessage());
                        }
                    }
                }
            } catch (IOException e) {
                if (running) {
                    System.err.println("Could not listen on port " + port + ": " + e.getMessage());
                }
            } finally {
                stop();
                System.out.println("Server stopped.");
            }
        }
    }

    static class ClientHandler implements Runnable {
        private final Socket clientSocket;
        private final Map<String, String> remoteObject;
        private static final int MAX_KEY_LENGTH = 256;
        private static final int MAX_VALUE_LENGTH = 1024;

        public ClientHandler(Socket socket, Map<String, String> remoteObject) {
            this.clientSocket = socket;
            this.remoteObject = remoteObject;
        }

        @Override
        public void run() {
            // Use try-with-resources for automatic resource management.
            try (
                BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true)
            ) {
                String line;
                while ((line = in.readLine()) != null) {
                    // Rule #4: Validate and sanitize all input.
                    String[] parts = line.trim().split(" ", 3);
                    if (parts.length == 0 || parts[0].isEmpty()) continue;

                    String command = parts[0].toUpperCase();
                    String response = processCommand(command, parts);
                    out.println(response);

                    if ("EXIT".equals(command)) {
                        break;
                    }
                }
            } catch (IOException e) {
                // Connection might have been closed by the client.
            } finally {
                try {
                    clientSocket.close();
                } catch (IOException e) {
                    // Ignore, already closing.
                }
            }
        }

        private String processCommand(String command, String[] parts) {
            try {
                switch (command) {
                    case "PUT":
                        if (parts.length != 3) return "ERROR: PUT requires 2 arguments: key and value.";
                        String putKey = parts[1];
                        String putValue = parts[2];
                        if (putKey.length() > MAX_KEY_LENGTH || putValue.length() > MAX_VALUE_LENGTH) {
                            return "ERROR: Key or value exceeds maximum length.";
                        }
                        remoteObject.put(putKey, putValue);
                        return "OK";
                    case "GET":
                        if (parts.length != 2) return "ERROR: GET requires 1 argument: key.";
                        String getKey = parts[1];
                        if (getKey.length() > MAX_KEY_LENGTH) return "ERROR: Key exceeds maximum length.";
                        String value = remoteObject.get(getKey);
                        return value != null ? "VALUE " + value : "NOT_FOUND";
                    case "DELETE":
                        if (parts.length != 2) return "ERROR: DELETE requires 1 argument: key.";
                        String delKey = parts[1];
                        if (delKey.length() > MAX_KEY_LENGTH) return "ERROR: Key exceeds maximum length.";
                        if (remoteObject.remove(delKey) != null) {
                            return "OK";
                        } else {
                            return "NOT_FOUND";
                        }
                    case "EXIT":
                        return "GOODBYE";
                    default:
                        // Rule #4: Reject unexpected commands.
                        return "ERROR: Unknown command '" + command + "'.";
                }
            } catch (Exception e) {
                // Rule #14: Ensure exceptions are caught and handled.
                return "ERROR: An internal server error occurred.";
            }
        }
    }

    public static String sendRequest(String host, int port, String request) {
        // Use try-with-resources for automatic socket and stream closing.
        try (
            Socket socket = new Socket(host, port);
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))
        ) {
            out.println(request);
            return in.readLine();
        } catch (IOException e) {
            return "CLIENT_ERROR: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        int port = 12345;
        RemoteObjectServer server = new RemoteObjectServer(port);
        Thread serverThread = new Thread(server);
        serverThread.start();
        
        try {
            // Give the server a moment to start up
            Thread.sleep(500);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        System.out.println("--- Running Test Cases ---");

        // Test Case 1: PUT and GET a value
        System.out.println("Test Case 1: PUT and GET");
        String response1 = sendRequest("localhost", port, "PUT key1 value1");
        System.out.println("Client sent: PUT key1 value1 -> Server responded: " + response1);
        String response2 = sendRequest("localhost", port, "GET key1");
        System.out.println("Client sent: GET key1 -> Server responded: " + response2);
        System.out.println();

        // Test Case 2: GET a non-existent value
        System.out.println("Test Case 2: GET non-existent key");
        String response3 = sendRequest("localhost", port, "GET non_existent_key");
        System.out.println("Client sent: GET non_existent_key -> Server responded: " + response3);
        System.out.println();

        // Test Case 3: DELETE a value
        System.out.println("Test Case 3: DELETE a key");
        String response4 = sendRequest("localhost", port, "DELETE key1");
        System.out.println("Client sent: DELETE key1 -> Server responded: " + response4);
        String response5 = sendRequest("localhost", port, "GET key1");
        System.out.println("Client sent: GET key1 -> Server responded: " + response5);
        System.out.println();

        // Test Case 4: Invalid command
        System.out.println("Test Case 4: Invalid command");
        String response6 = sendRequest("localhost", port, "INVALID_COMMAND");
        System.out.println("Client sent: INVALID_COMMAND -> Server responded: " + response6);
        System.out.println();
        
        // Test Case 5: Malformed command (wrong number of arguments)
        System.out.println("Test Case 5: Malformed command");
        String response7 = sendRequest("localhost", port, "PUT key2");
        System.out.println("Client sent: PUT key2 -> Server responded: " + response7);
        System.out.println();

        // Signal server to shut down
        server.stop();
        // A dummy connection to unblock the server.accept() call if it's waiting
        try {
            new Socket("localhost", port).close();
        } catch (IOException e) {
            // Expected as server is closing
        }
        
        try {
            serverThread.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        System.out.println("--- Test Cases Finished ---");
    }
}