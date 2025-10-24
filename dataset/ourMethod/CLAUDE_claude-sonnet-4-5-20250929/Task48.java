
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.regex.Pattern;

public class Task48 {
    /* Maximum message size to prevent excessive memory usage */
    private static final int MAX_MESSAGE_SIZE = 4096;
    /* Maximum username length */
    private static final int MAX_USERNAME = 32;
    /* Server port */
    private static final int SERVER_PORT = 8888;
    /* Maximum number of clients */
    private static final int MAX_CLIENTS = 100;
    
    /* Pattern to validate input: reject control characters except newline/tab */
    private static final Pattern VALID_INPUT = Pattern.compile("^[\\\\x20-\\\\x7E\\\\t\\\\n\\\\r]+$");
    
    /* Client handler class */
    private static class ClientHandler implements Runnable {
        private final Socket socket;
        private final ChatServer server;
        private BufferedReader reader;
        private PrintWriter writer;
        private String username;
        
        public ClientHandler(Socket socket, ChatServer server) {
            /* Input validation */
            if (socket == null || server == null) {
                throw new IllegalArgumentException("Socket and server cannot be null");
            }
            this.socket = socket;
            this.server = server;
            this.username = null;
        }
        
        /* Validate input string */
        private boolean validateInput(String input, int maxLength) {
            if (input == null || input.isEmpty()) {
                return false;
            }
            /* Check length to prevent buffer overflow */
            if (input.length() > maxLength) {
                return false;
            }
            /* Validate characters using pattern */
            return VALID_INPUT.matcher(input).matches();
        }
        
        @Override
        public void run() {
            try {
                /* Set socket timeout to prevent indefinite blocking */
                socket.setSoTimeout(300000); // 5 minutes
                
                /* Initialize streams with UTF-8 encoding */
                reader = new BufferedReader(
                    new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8));
                writer = new PrintWriter(
                    new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8), true);
                
                /* Receive and validate username */
                username = reader.readLine();
                if (!validateInput(username, MAX_USERNAME)) {
                    writer.println("Invalid username");
                    return;
                }
                
                /* Add client to server */
                if (!server.addClient(this)) {
                    writer.println("Server full");
                    return;
                }
                
                /* Broadcast join message */
                server.broadcast("[Server] " + username + " joined the chat", this);
                
                /* Main message loop */
                String message;
                while ((message = reader.readLine()) != null) {
                    /* Validate message before processing */
                    if (validateInput(message, MAX_MESSAGE_SIZE)) {
                        String formatted = "[" + username + "]: " + message;
                        server.broadcast(formatted, this);
                    }
                }
            } catch (IOException e) {
                /* Log error without exposing internal details */
                System.err.println("Client handler error: connection issue");
            } finally {
                /* Clean up resources */
                server.removeClient(this);
                if (username != null) {
                    server.broadcast("[Server] " + username + " left the chat", this);
                }
                closeResources();
            }
        }
        
        /* Send message to this client */
        public void sendMessage(String message) {
            if (writer != null && message != null) {
                writer.println(message);
            }
        }
        
        public String getUsername() {
            return username;
        }
        
        /* Close all resources */
        private void closeResources() {
            try {
                if (reader != null) reader.close();
            } catch (IOException e) {
                /* Ignore close errors */
            }
            try {
                if (writer != null) writer.close();
            } catch (Exception e) {
                /* Ignore close errors */
            }
            try {
                if (socket != null && !socket.isClosed()) socket.close();
            } catch (IOException e) {
                /* Ignore close errors */
            }
        }
    }
    
    /* Chat server class */
    private static class ChatServer {
        private final Set<ClientHandler> clients;
        private final ExecutorService executor;
        private final AtomicBoolean running;
        private ServerSocket serverSocket;
        
        public ChatServer() {
            /* Use thread-safe collection for clients */
            this.clients = Collections.synchronizedSet(new HashSet<>());
            /* Create thread pool with maximum size to prevent resource exhaustion */
            this.executor = Executors.newFixedThreadPool(MAX_CLIENTS);
            this.running = new AtomicBoolean(true);
        }
        
        /* Add client with size limit enforcement */
        public boolean addClient(ClientHandler client) {
            synchronized (clients) {
                if (clients.size() >= MAX_CLIENTS) {
                    return false;
                }
                return clients.add(client);
            }
        }
        
        /* Remove client */
        public void removeClient(ClientHandler client) {
            synchronized (clients) {
                clients.remove(client);
            }
        }
        
        /* Broadcast message to all clients except sender */
        public void broadcast(String message, ClientHandler sender) {
            /* Input validation */
            if (message == null || message.isEmpty()) {
                return;
            }
            
            synchronized (clients) {
                for (ClientHandler client : clients) {
                    if (client != sender) {
                        try {
                            client.sendMessage(message);
                        } catch (Exception e) {
                            /* Continue broadcasting to other clients on error */
                            System.err.println("Failed to send to client");
                        }
                    }
                }
            }
        }
        
        /* Start server */
        public void start() {
            try {
                /* Create server socket */
                serverSocket = new ServerSocket(SERVER_PORT);
                System.out.println("Server listening on port " + SERVER_PORT);
                
                /* Accept client connections */
                while (running.get()) {
                    try {
                        Socket clientSocket = serverSocket.accept();
                        /* Create and submit client handler */
                        ClientHandler handler = new ClientHandler(clientSocket, this);
                        executor.submit(handler);
                    } catch (SocketException e) {
                        if (!running.get()) {
                            break; // Server shutdown
                        }
                        System.err.println("Socket error");
                    } catch (IOException e) {
                        System.err.println("Failed to accept connection");
                    }
                }
            } catch (IOException e) {
                System.err.println("Failed to start server");
            } finally {
                shutdown();
            }
        }
        
        /* Shutdown server */
        public void shutdown() {
            running.set(false);
            
            /* Close all client connections */
            synchronized (clients) {
                for (ClientHandler client : clients) {
                    try {
                        client.closeResources();
                    } catch (Exception e) {
                        /* Ignore errors during shutdown */
                    }
                }
                clients.clear();
            }
            
            /* Shutdown executor */
            executor.shutdown();
            try {
                if (!executor.awaitTermination(5, TimeUnit.SECONDS)) {
                    executor.shutdownNow();
                }
            } catch (InterruptedException e) {
                executor.shutdownNow();
                Thread.currentThread().interrupt();
            }
            
            /* Close server socket */
            if (serverSocket != null && !serverSocket.isClosed()) {
                try {
                    serverSocket.close();
                } catch (IOException e) {
                    /* Ignore close errors */
                }
            }
        }
    }
    
    /* Chat client class */
    private static class ChatClient {
        private Socket socket;
        private BufferedReader reader;
        private PrintWriter writer;
        private BufferedReader consoleReader;
        private final String username;
        
        public ChatClient(String username) {
            /* Input validation */
            if (username == null || username.isEmpty() || username.length() > MAX_USERNAME) {
                throw new IllegalArgumentException("Invalid username");
            }
            if (!VALID_INPUT.matcher(username).matches()) {
                throw new IllegalArgumentException("Username contains invalid characters");
            }
            this.username = username;
        }
        
        /* Connect to server */
        public void connect() {
            try {
                /* Connect to localhost */
                socket = new Socket("127.0.0.1", SERVER_PORT);
                
                /* Initialize streams with UTF-8 encoding */
                reader = new BufferedReader(
                    new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8));
                writer = new PrintWriter(
                    new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8), true);
                consoleReader = new BufferedReader(
                    new InputStreamReader(System.in, StandardCharsets.UTF_8));
                
                /* Send username */
                writer.println(username);
                
                System.out.println("Connected as " + username + ". Type messages to send:");
                
                /* Start receiver thread */
                Thread receiverThread = new Thread(this::receiveMessages);
                receiverThread.setDaemon(true);
                receiverThread.start();
                
                /* Send messages from console */
                sendMessages();
                
            } catch (IOException e) {
                System.err.println("Connection failed");
            } finally {
                disconnect();
            }
        }
        
        /* Receive messages from server */
        private void receiveMessages() {
            try {
                String message;
                while ((message = reader.readLine()) != null) {
                    System.out.println(message);
                }
            } catch (IOException e) {
                System.err.println("Disconnected from server");
            }
        }
        
        /* Send messages to server */
        private void sendMessages() {
            try {
                String message;
                while ((message = consoleReader.readLine()) != null) {
                    /* Validate message before sending */
                    if (message.length() > 0 && message.length() <= MAX_MESSAGE_SIZE &&
                        VALID_INPUT.matcher(message).matches()) {
                        writer.println(message);
                    }
                }
            } catch (IOException e) {
                System.err.println("Input error");
            }
        }
        
        /* Disconnect from server */
        private void disconnect() {
            try {
                if (reader != null) reader.close();
            } catch (IOException e) {
                /* Ignore close errors */
            }
            try {
                if (writer != null) writer.close();
            } catch (Exception e) {
                /* Ignore close errors */
            }
            try {
                if (consoleReader != null) consoleReader.close();
            } catch (IOException e) {
                /* Ignore close errors */
            }
            try {
                if (socket != null && !socket.isClosed()) socket.close();
            } catch (IOException e) {
                /* Ignore close errors */
            }
        }
    }
    
    public static void main(String[] args) {
        /* Validate command line arguments */
        if (args.length < 1) {
            System.out.println("Usage:");
            System.out.println("  Server mode: java Task48 server");
            System.out.println("  Client mode: java Task48 client <username>");
            return;
        }
        
        try {
            if ("server".equals(args[0])) {
                /* Run server */
                ChatServer server = new ChatServer();
                
                /* Add shutdown hook for graceful shutdown */
                Runtime.getRuntime().addShutdownHook(new Thread(server::shutdown));
                
                server.start();
            } else if ("client".equals(args[0]) && args.length >= 2) {
                /* Run client */
                ChatClient client = new ChatClient(args[1]);
                client.connect();
            } else {
                System.out.println("Invalid arguments");
            }
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
