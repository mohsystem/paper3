import java.io.*;
import java.net.*;
import java.util.List;
import java.util.concurrent.*;

public class Task48 {

    private static final int PORT = 12345;
    private static final String HOST = "127.0.0.1";
    private static final int MAX_CLIENTS = 10;

    // Server implementation
    private static class Server {
        // Use a thread-safe list to store client writers.
        private static final List<PrintWriter> clientWriters = new CopyOnWriteArrayList<>();

        public void start() {
            // Use a fixed-size thread pool to prevent resource exhaustion from too many clients.
            ExecutorService pool = Executors.newFixedThreadPool(MAX_CLIENTS);
            System.out.println("Java Server started on port " + PORT);
            try (ServerSocket serverSocket = new ServerSocket(PORT)) {
                while (!Thread.currentThread().isInterrupted()) {
                    try {
                        Socket clientSocket = serverSocket.accept();
                        pool.execute(new ClientHandler(clientSocket));
                    } catch (IOException e) {
                        if (serverSocket.isClosed()) {
                            System.out.println("Server socket closed, shutting down.");
                            break;
                        }
                        System.err.println("Error accepting client connection: " + e.getMessage());
                    }
                }
            } catch (IOException e) {
                System.err.println("Could not listen on port " + PORT + ": " + e.getMessage());
            } finally {
                pool.shutdown();
            }
        }

        private static class ClientHandler implements Runnable {
            private final Socket clientSocket;
            private PrintWriter out;
            private String clientName;

            public ClientHandler(Socket socket) {
                this.clientSocket = socket;
            }

            @Override
            public void run() {
                try {
                    BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                    out = new PrintWriter(clientSocket.getOutputStream(), true);

                    // Basic identification: first message is the name
                    clientName = sanitizeInput(in.readLine());
                    if (clientName == null || clientName.trim().isEmpty()) {
                        clientName = "Anonymous@" + clientSocket.getRemoteSocketAddress();
                    }
                    System.out.println(clientName + " has connected.");
                    broadcastMessage(clientName + " has joined the chat.");
                    clientWriters.add(out);

                    String inputLine;
                    while ((inputLine = in.readLine()) != null) {
                        String message = sanitizeInput(inputLine);
                        // Basic input validation: prevent overly long messages
                        if (message.length() > 512) {
                            message = message.substring(0, 512);
                        }
                        broadcastMessage(clientName + ": " + message);
                    }
                } catch (IOException e) {
                    // Handle client disconnection gracefully.
                } finally {
                    try {
                        if (out != null) {
                            clientWriters.remove(out);
                        }
                        if (clientName != null) {
                            broadcastMessage(clientName + " has left the chat.");
                            System.out.println(clientName + " has disconnected.");
                        }
                        clientSocket.close();
                    } catch (IOException e) {
                        System.err.println("Error closing client socket: " + e.getMessage());
                    }
                }
            }

            private void broadcastMessage(String message) {
                // Iterating over CopyOnWriteArrayList is thread-safe.
                for (PrintWriter writer : clientWriters) {
                    writer.println(message);
                }
            }

            // Simple input sanitization to remove potentially harmful characters.
            private String sanitizeInput(String input) {
                if (input == null) return "";
                return input.replaceAll("[\\p{Cntrl}&&[^\r\n\t]]", "").trim();
            }
        }
    }

    // Client implementation
    private static class Client {
        public void start() {
            try (
                Socket socket = new Socket(HOST, PORT);
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in))
            ) {
                System.out.print("Enter your name: ");
                String name = stdIn.readLine();
                out.println(name); // Send name to server first

                // Thread to listen for messages from the server
                Thread listenerThread = new Thread(() -> {
                    try {
                        String fromServer;
                        while ((fromServer = in.readLine()) != null) {
                            System.out.println(fromServer);
                        }
                    } catch (IOException e) {
                        System.out.println("Connection to server lost.");
                    }
                });
                listenerThread.start();

                // Main thread to send messages to the server
                String fromUser;
                while (listenerThread.isAlive() && (fromUser = stdIn.readLine()) != null) {
                    out.println(fromUser);
                }
            } catch (UnknownHostException e) {
                System.err.println("Don't know about host " + HOST);
            } catch (IOException e) {
                System.err.println("Couldn't get I/O for the connection to " + HOST);
            }
        }
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            if (args[0].equalsIgnoreCase("server")) {
                new Server().start();
            } else if (args[0].equalsIgnoreCase("client")) {
                new Client().start();
            } else {
                System.out.println("Usage: java Task48 [server|client]");
            }
        } else {
            System.out.println("--- Running Automated Test Cases ---");
            runTestCases();
            System.out.println("\n--- Test Cases Finished ---");
            System.out.println("To run manually: java Task48 [server|client]");
        }
    }

    private static void runTestCases() {
        // Test Case 1: Start Server
        ServerSocket serverSocketForTest = null;
        Thread serverThread = new Thread(() -> new Server().start());
        serverThread.start();
        try {
            Thread.sleep(500); // Wait for server to start
        } catch (InterruptedException e) { Thread.currentThread().interrupt(); }

        // Test Case 2 & 3: Two clients connect and send messages
        Thread client1 = new Thread(() -> runTestClient("ClientA", "Hello World!"));
        Thread client2 = new Thread(() -> runTestClient("ClientB", "I am here too."));

        client1.start();
        client2.start();

        try {
             Thread.sleep(1000); // Wait for messages to be sent
        } catch (InterruptedException e) { Thread.currentThread().interrupt(); }
        
        // Test Case 4: A third client connects
        Thread client3 = new Thread(() -> runTestClient("ClientC", "Hi everyone!"));
        client3.start();

        try {
            // Test Case 5: Wait for all clients to finish
            client1.join();
            client2.join();
            client3.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        // Shutdown server
        serverThread.interrupt();
        try {
            // Unblock the server's accept() call
            new Socket(HOST, PORT).close();
        } catch (IOException e) { /* expected */ }
    }

    private static void runTestClient(String name, String message) {
        try (Socket socket = new Socket(HOST, PORT);
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))) {
            
            out.println(name); // Send name

            // Listen for 2 messages then disconnect
            Thread listener = new Thread(() -> {
                try {
                    for(int i = 0; i < 3; i++) {
                       if(in.readLine() == null) break;
                    }
                } catch (IOException e) {}
            });
            listener.start();

            Thread.sleep(200); // Stagger messages
            out.println(message);
            listener.join(1000); // Wait for messages or timeout

        } catch (IOException | InterruptedException e) {
            System.err.println("Test client " + name + " error: " + e.getMessage());
        }
    }
}