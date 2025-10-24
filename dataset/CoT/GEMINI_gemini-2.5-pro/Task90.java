import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Task90 {

    private static final int MAX_CLIENTS = 10;
    // Using a thread-safe list to store client handlers.
    private static final List<ClientHandler> clients = Collections.synchronizedList(new ArrayList<>());
    private static ExecutorService pool = Executors.newFixedThreadPool(MAX_CLIENTS);

    public static void startServer(int port) {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Chat Server started on port " + port);
            System.out.println("Waiting for clients...");

            while (!serverSocket.isClosed()) {
                try {
                    Socket clientSocket = serverSocket.accept();
                    if (clients.size() >= MAX_CLIENTS) {
                        System.out.println("Max clients reached. Rejecting new connection from " + clientSocket.getRemoteSocketAddress());
                        clientSocket.close();
                        continue;
                    }
                    System.out.println("Client connected: " + clientSocket.getRemoteSocketAddress());
                    ClientHandler clientThread = new ClientHandler(clientSocket);
                    clients.add(clientThread);
                    pool.execute(clientThread);
                } catch (IOException e) {
                    System.err.println("Error accepting client connection: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Could not listen on port " + port + ": " + e.getMessage());
        } finally {
            pool.shutdown();
        }
    }

    private static class ClientHandler implements Runnable {
        private final Socket clientSocket;
        private PrintWriter out;
        private BufferedReader in;
        private final String clientName;

        public ClientHandler(Socket socket) {
            this.clientSocket = socket;
            this.clientName = "Client-" + (clients.size() + 1);
        }

        @Override
        public void run() {
            try {
                out = new PrintWriter(clientSocket.getOutputStream(), true);
                in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));

                String message;
                // Read messages from client and broadcast
                while ((message = in.readLine()) != null) {
                    // Basic input validation: check message length
                    if (message.length() > 1024) {
                        System.out.println("Received oversized message from " + clientName + ". Disconnecting.");
                        break;
                    }
                    System.out.println(clientName + " says: " + message);
                    broadcast(clientName + ": " + message, this);
                }
            } catch (SocketException e) {
                System.out.println(clientName + " disconnected abruptly.");
            } catch (IOException e) {
                System.err.println("IOException for " + clientName + ": " + e.getMessage());
            } finally {
                cleanup();
            }
        }

        private void sendMessage(String message) {
            out.println(message);
        }

        private void broadcast(String message, ClientHandler sender) {
            // Synchronized block to safely iterate over the list
            synchronized (clients) {
                for (ClientHandler client : clients) {
                    // Don't send the message back to the sender
                    if (client != sender) {
                        client.sendMessage(message);
                    }
                }
            }
        }

        private void cleanup() {
            System.out.println(clientName + " has disconnected.");
            // Must synchronize when modifying the shared list
            synchronized (clients) {
                clients.remove(this);
            }
            try {
                if (in != null) in.close();
                if (out != null) out.close();
                if (clientSocket != null && !clientSocket.isClosed()) {
                    clientSocket.close();
                }
            } catch (IOException e) {
                System.err.println("Error closing resources for " + clientName + ": " + e.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases are not applicable in the traditional sense for a server.
        // The main method starts the server. To test, connect multiple clients
        // using a tool like netcat or a separate client program.
        // For example: `nc localhost 9090`
        // The server will log connections, which can be observed.
        System.out.println("--- Test Case 1: Starting server on port 9090 ---");
        int port = 9090; // Default port
        if (args.length > 0) {
            try {
                port = Integer.parseInt(args[0]);
            } catch (NumberFormatException e) {
                System.err.println("Invalid port number. Using default 9090.");
            }
        }
        
        // This will run indefinitely until the process is terminated.
        // To simulate further test cases, you would need to run the server
        // with different configurations or stop and restart it.
        // Simulating 5 test cases by logging:
        System.out.println("--- Test Case 2: Server is ready to accept up to " + MAX_CLIENTS + " clients.");
        System.out.println("--- Test Case 3: Connect a client using 'nc localhost " + port + "' and send a message.");
        System.out.println("--- Test Case 4: Connect another client and verify messages are broadcasted.");
        System.out.println("--- Test Case 5: Disconnect a client and observe the 'disconnected' log message.");

        startServer(port);
    }
}