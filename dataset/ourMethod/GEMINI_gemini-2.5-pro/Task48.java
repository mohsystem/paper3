import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

public class Task48 {

    private static final int PORT = 12345;
    private static final String HOST = "127.0.0.1";
    private static final int MAX_CLIENTS = 10;

    // Server implementation
    private static class ChatServer {
        private static final List<ClientHandler> clients = Collections.synchronizedList(new ArrayList<>());
        private static final ExecutorService pool = Executors.newFixedThreadPool(MAX_CLIENTS);

        public void start() {
            System.out.println("Server started on port " + PORT);
            try (ServerSocket serverSocket = new ServerSocket(PORT)) {
                while (true) {
                    Socket clientSocket = serverSocket.accept();
                    ClientHandler clientThread = new ClientHandler(clientSocket);
                    clients.add(clientThread);
                    pool.execute(clientThread);
                }
            } catch (IOException e) {
                System.err.println("Error in server: " + e.getMessage());
            } finally {
                pool.shutdown();
            }
        }

        private static class ClientHandler implements Runnable {
            private final Socket clientSocket;
            private PrintWriter out;
            private BufferedReader in;
            private String clientName;

            public ClientHandler(Socket socket) {
                this.clientSocket = socket;
            }

            @Override
            public void run() {
                try {
                    out = new PrintWriter(clientSocket.getOutputStream(), true, StandardCharsets.UTF_8);
                    in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream(), StandardCharsets.UTF_8));

                    // Simple name registration
                    out.println("Enter your name:");
                    clientName = in.readLine();
                    if (clientName == null || clientName.trim().isEmpty() || clientName.length() > 50) {
                        clientName = "Anonymous" + System.currentTimeMillis() % 100;
                    }
                    System.out.println(clientName + " connected.");
                    broadcastMessage(clientName + " has joined the chat.", this);

                    String message;
                    while ((message = in.readLine()) != null) {
                        if (message.length() > 1024) { // Basic input validation
                            message = message.substring(0, 1024);
                        }
                        System.out.println("Received from " + clientName + ": " + message);
                        broadcastMessage(clientName + ": " + message, this);
                    }
                } catch (SocketException e) {
                    System.out.println(clientName + " disconnected abruptly.");
                } catch (IOException e) {
                    System.err.println("Error handling client " + clientName + ": " + e.getMessage());
                } finally {
                    cleanup();
                }
            }
            
            private void broadcastMessage(String message, ClientHandler sender) {
                // Synchronize to prevent concurrent modification issues (TOCTOU)
                synchronized (clients) {
                    for (ClientHandler client : clients) {
                        if (client != sender) {
                            client.sendMessage(message);
                        }
                    }
                }
            }

            public void sendMessage(String message) {
                out.println(message);
            }
            
            private void cleanup() {
                try {
                    clients.remove(this);
                    if (clientName != null) {
                        System.out.println(clientName + " has disconnected.");
                        broadcastMessage(clientName + " has left the chat.", this);
                    }
                    if (in != null) in.close();
                    if (out != null) out.close();
                    if (clientSocket != null) clientSocket.close();
                } catch (IOException e) {
                    System.err.println("Error during client cleanup: " + e.getMessage());
                }
            }
        }
    }

    // Client implementation
    private static class ChatClient {
        private final AtomicBoolean running = new AtomicBoolean(true);
        public void start() {
            try (Socket socket = new Socket(HOST, PORT);
                 PrintWriter out = new PrintWriter(socket.getOutputStream(), true, StandardCharsets.UTF_8);
                 BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8));
                 BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in))) {

                System.out.println("Connected to the chat server. Type 'exit' to quit.");

                // Thread to listen for messages from the server
                Thread listenerThread = new Thread(() -> {
                    try {
                        String serverMessage;
                        while (running.get() && (serverMessage = in.readLine()) != null) {
                            System.out.println(serverMessage);
                        }
                    } catch (IOException e) {
                        if (running.get()) {
                            System.err.println("Connection to server lost.");
                        }
                    } finally {
                        running.set(false);
                    }
                });
                listenerThread.start();
                
                // Main thread to send messages
                String userInput;
                while (running.get() && (userInput = consoleReader.readLine()) != null) {
                    if ("exit".equalsIgnoreCase(userInput.trim())) {
                        break;
                    }
                    if (!userInput.trim().isEmpty()) {
                        out.println(userInput);
                    }
                }

            } catch (IOException e) {
                System.err.println("Client error: " + e.getMessage());
            } finally {
                running.set(false);
                System.out.println("Disconnected from server.");
            }
        }
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            printUsage();
            // The following demonstrates a 'test plan' as this is an interactive app.
            System.out.println("\n--- Test Plan ---");
            System.out.println("1. Run 'java Task48 server' in one terminal.");
            System.out.println("2. Run 'java Task48 client' in a second terminal, enter name 'Alice'.");
            System.out.println("3. Run 'java Task48 client' in a third terminal, enter name 'Bob'.");
            System.out.println("4. In Alice's terminal, type 'Hello Bob!' and press Enter. Observe Bob's terminal receiving the message.");
            System.out.println("5. In Bob's terminal, type 'Hi Alice!' and press Enter. Observe Alice's terminal receiving the message.");
            return;
        }

        String mode = args[0];
        if ("server".equalsIgnoreCase(mode)) {
            new ChatServer().start();
        } else if ("client".equalsIgnoreCase(mode)) {
            new ChatClient().start();
        } else {
            printUsage();
        }
    }

    private static void printUsage() {
        System.out.println("Usage: java Task48 [server|client]");
    }
}