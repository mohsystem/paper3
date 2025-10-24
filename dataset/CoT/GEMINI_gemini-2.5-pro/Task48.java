import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

public class Task48 {

    private static final int PORT = 12345;
    private static final String HOST = "127.0.0.1";
    // Using a thread-safe set to store client writers
    private static Set<PrintWriter> clientWriters = new CopyOnWriteArraySet<>();

    // --- SERVER IMPLEMENTATION ---
    public static void startServer() {
        System.out.println("Chat Server is starting on port " + PORT + "...");
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            while (true) {
                // Accept new client connections
                new ClientHandler(serverSocket.accept()).start();
            }
        } catch (IOException e) {
            System.err.println("Error in the server: " + e.getMessage());
        }
    }

    private static class ClientHandler extends Thread {
        private Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        private String clientName;

        public ClientHandler(Socket socket) {
            this.socket = socket;
        }

        public void run() {
            try {
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                out = new PrintWriter(socket.getOutputStream(), true);
                
                // Request a name from the client.
                out.println("SUBMIT_NAME");
                clientName = in.readLine();
                if (clientName == null || clientName.trim().isEmpty()) {
                    clientName = "Anonymous" + (int)(Math.random() * 1000);
                }
                
                System.out.println(clientName + " has connected.");
                broadcastMessage(clientName + " has joined the chat.", null);
                clientWriters.add(out);

                String message;
                while ((message = in.readLine()) != null) {
                    if ("exit".equalsIgnoreCase(message.trim())) {
                        break;
                    }
                    String broadcastMsg = clientName + ": " + message;
                    System.out.println("Broadcasting: " + broadcastMsg);
                    broadcastMessage(broadcastMsg, out);
                }
            } catch (IOException e) {
                // This exception is often thrown when a client disconnects.
            } finally {
                if (clientName != null) {
                    System.out.println(clientName + " has left.");
                    broadcastMessage(clientName + " has left the chat.", null);
                }
                if (out != null) {
                    clientWriters.remove(out);
                }
                try {
                    socket.close();
                } catch (IOException e) {
                    // Ignore
                }
            }
        }
    }
    
    private static void broadcastMessage(String message, PrintWriter sender) {
        for (PrintWriter writer : clientWriters) {
            // Optionally avoid sending the message back to the sender
            // if (writer != sender) {
                writer.println(message);
            // }
        }
    }


    // --- CLIENT IMPLEMENTATION ---
    public static void startClient() {
        try (Socket socket = new Socket(HOST, PORT)) {
            System.out.println("Connected to the chat server. Type 'exit' to quit.");
            
            // Thread to read messages from the server
            new Thread(() -> {
                try {
                    BufferedReader serverIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                    String serverMessage;
                    while ((serverMessage = serverIn.readLine()) != null) {
                        if ("SUBMIT_NAME".equals(serverMessage)) {
                            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                            System.out.print("Enter your name: ");
                            BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in));
                            String name = consoleReader.readLine();
                            out.println(name);
                        } else {
                            System.out.println(serverMessage);
                        }
                    }
                } catch (IOException e) {
                    System.out.println("Disconnected from server.");
                }
            }).start();

            // Main thread to send messages to the server
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in));
            String userInput;
            while ((userInput = consoleReader.readLine()) != null) {
                out.println(userInput);
                if ("exit".equalsIgnoreCase(userInput.trim())) {
                    break;
                }
            }
        } catch (UnknownHostException e) {
            System.err.println("Server not found: " + e.getMessage());
        } catch (IOException e) {
            System.err.println("Couldn't get I/O for the connection to " + HOST + ": " + e.getMessage());
        }
    }


    public static void main(String[] args) {
        if (args.length > 0) {
            if (args[0].equalsIgnoreCase("server")) {
                startServer();
            } else if (args[0].equalsIgnoreCase("client")) {
                startClient();
            } else {
                printUsage();
            }
        } else {
            printUsage();
        }
    }
    
    private static void printUsage() {
        System.out.println("Usage: java Task48 <mode>");
        System.out.println("  mode: 'server' or 'client'");
        System.out.println("\n--- How to Test (5 Test Cases) ---");
        System.out.println("1. Start the server: Open a terminal and run 'java Task48 server'");
        System.out.println("2. Start Client 1: Open a second terminal and run 'java Task48 client'. Enter a name.");
        System.out.println("3. Start Client 2: Open a third terminal and run 'java Task48 client'. Enter another name.");
        System.out.println("4. Send Messages: Type a message in Client 1's terminal and press Enter. The message should appear in Client 2's terminal, and vice-versa.");
        System.out.println("5. Client Disconnect: In Client 1's terminal, type 'exit' and press Enter. A 'has left' message should appear on the server and in Client 2's terminal.");
    }
}