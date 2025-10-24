
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

public class Task90 {
    private static final int PORT = 8888;
    private static final int MAX_CLIENTS = 100;
    private static final int MAX_MESSAGE_LENGTH = 1024;
    private static final Set<ClientHandler> clients = ConcurrentHashMap.newKeySet();
    private static final ExecutorService threadPool = Executors.newFixedThreadPool(MAX_CLIENTS);

    public static void main(String[] args) {
        System.out.println("Chat Server Starting on port " + PORT);
        startServer(PORT);
    }

    public static void startServer(int port) {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            serverSocket.setSoTimeout(0);
            System.out.println("Server started. Waiting for clients...");

            while (true) {
                try {
                    if (clients.size() >= MAX_CLIENTS) {
                        Thread.sleep(1000);
                        continue;
                    }
                    Socket clientSocket = serverSocket.accept();
                    clientSocket.setSoTimeout(300000); // 5 minute timeout
                    
                    ClientHandler clientHandler = new ClientHandler(clientSocket);
                    clients.add(clientHandler);
                    threadPool.execute(clientHandler);
                    
                    System.out.println("New client connected: " + clientSocket.getInetAddress());
                } catch (IOException e) {
                    System.err.println("Error accepting client: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Server error: " + e.getMessage());
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        } finally {
            shutdown();
        }
    }

    public static void broadcast(String message, ClientHandler sender) {
        if (message == null || message.length() > MAX_MESSAGE_LENGTH) {
            return;
        }
        
        String sanitizedMessage = sanitizeMessage(message);
        for (ClientHandler client : clients) {
            if (client != sender) {
                client.sendMessage(sanitizedMessage);
            }
        }
    }

    private static String sanitizeMessage(String message) {
        if (message == null) return "";
        return message.replaceAll("[^\\\\x20-\\\\x7E\\\\n\\\\r\\\\t]", "")
                     .substring(0, Math.min(message.length(), MAX_MESSAGE_LENGTH));
    }

    public static void removeClient(ClientHandler client) {
        clients.remove(client);
    }

    private static void shutdown() {
        for (ClientHandler client : clients) {
            client.close();
        }
        threadPool.shutdown();
    }

    static class ClientHandler implements Runnable {
        private final Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        private String clientId;

        public ClientHandler(Socket socket) {
            this.socket = socket;
            this.clientId = socket.getInetAddress().toString() + ":" + socket.getPort();
        }

        @Override
        public void run() {
            try {
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                out = new PrintWriter(socket.getOutputStream(), true);

                out.println("Welcome to the chat server!");
                broadcast(clientId + " joined the chat", this);

                String message;
                while ((message = in.readLine()) != null) {
                    if (message.trim().isEmpty()) continue;
                    if (message.length() > MAX_MESSAGE_LENGTH) {
                        message = message.substring(0, MAX_MESSAGE_LENGTH);
                    }
                    System.out.println(clientId + ": " + message);
                    broadcast(clientId + ": " + message, this);
                }
            } catch (IOException e) {
                System.err.println("Client handler error: " + e.getMessage());
            } finally {
                close();
                removeClient(this);
                broadcast(clientId + " left the chat", this);
            }
        }

        public void sendMessage(String message) {
            if (out != null && !socket.isClosed()) {
                out.println(message);
            }
        }

        public void close() {
            try {
                if (in != null) in.close();
                if (out != null) out.close();
                if (socket != null && !socket.isClosed()) socket.close();
            } catch (IOException e) {
                System.err.println("Error closing client: " + e.getMessage());
            }
        }
    }
}
