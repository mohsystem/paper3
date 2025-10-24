
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

public class Task48 {
    private static final int PORT = 12345;
    private static final int MAX_CLIENTS = 10;
    private static final int MAX_MESSAGE_LENGTH = 1024;
    
    static class ChatServer {
        private Set<ClientHandler> clients = ConcurrentHashMap.newKeySet();
        private ExecutorService pool = Executors.newFixedThreadPool(MAX_CLIENTS);
        private ServerSocket serverSocket;
        
        public void start() {
            try {
                serverSocket = new ServerSocket(PORT);
                System.out.println("Server started on port " + PORT);
                
                while (true) {
                    if (clients.size() >= MAX_CLIENTS) {
                        Thread.sleep(100);
                        continue;
                    }
                    Socket clientSocket = serverSocket.accept();
                    ClientHandler handler = new ClientHandler(clientSocket, this);
                    clients.add(handler);
                    pool.execute(handler);
                }
            } catch (Exception e) {
                System.err.println("Server error: " + e.getMessage());
            } finally {
                shutdown();
            }
        }
        
        public void broadcast(String message, ClientHandler sender) {
            if (message == null || message.length() > MAX_MESSAGE_LENGTH) {
                return;
            }
            String sanitized = sanitizeMessage(message);
            for (ClientHandler client : clients) {
                if (client != sender) {
                    client.sendMessage(sanitized);
                }
            }
        }
        
        private String sanitizeMessage(String message) {
            return message.replaceAll("[^\\\\x20-\\\\x7E]", "");
        }
        
        public void removeClient(ClientHandler client) {
            clients.remove(client);
        }
        
        public void shutdown() {
            try {
                if (serverSocket != null) serverSocket.close();
                pool.shutdownNow();
            } catch (IOException e) {
                System.err.println("Shutdown error: " + e.getMessage());
            }
        }
    }
    
    static class ClientHandler implements Runnable {
        private Socket socket;
        private ChatServer server;
        private PrintWriter out;
        private BufferedReader in;
        private String clientId;
        
        public ClientHandler(Socket socket, ChatServer server) {
            this.socket = socket;
            this.server = server;
            this.clientId = socket.getRemoteSocketAddress().toString();
        }
        
        @Override
        public void run() {
            try {
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                out = new PrintWriter(socket.getOutputStream(), true);
                
                out.println("Connected to chat server. Type messages to send.");
                server.broadcast(clientId + " joined the chat", this);
                
                String message;
                while ((message = in.readLine()) != null) {
                    if (message.length() > MAX_MESSAGE_LENGTH) {
                        message = message.substring(0, MAX_MESSAGE_LENGTH);
                    }
                    if (!message.trim().isEmpty()) {
                        server.broadcast(clientId + ": " + message, this);
                    }
                }
            } catch (IOException e) {
                System.err.println("Client error: " + e.getMessage());
            } finally {
                cleanup();
            }
        }
        
        public void sendMessage(String message) {
            if (out != null && message != null) {
                out.println(message);
            }
        }
        
        private void cleanup() {
            try {
                server.removeClient(this);
                server.broadcast(clientId + " left the chat", this);
                if (in != null) in.close();
                if (out != null) out.close();
                if (socket != null) socket.close();
            } catch (IOException e) {
                System.err.println("Cleanup error: " + e.getMessage());
            }
        }
    }
    
    static class ChatClient {
        private Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        
        public void connect(String host, int port) {
            try {
                socket = new Socket(host, port);
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                out = new PrintWriter(socket.getOutputStream(), true);
                
                new Thread(() -> {
                    try {
                        String message;
                        while ((message = in.readLine()) != null) {
                            System.out.println(message);
                        }
                    } catch (IOException e) {
                        System.err.println("Connection closed");
                    }
                }).start();
                
            } catch (IOException e) {
                System.err.println("Connection error: " + e.getMessage());
            }
        }
        
        public void sendMessage(String message) {
            if (out != null && message != null && message.length() <= MAX_MESSAGE_LENGTH) {
                out.println(message);
            }
        }
        
        public void disconnect() {
            try {
                if (in != null) in.close();
                if (out != null) out.close();
                if (socket != null) socket.close();
            } catch (IOException e) {
                System.err.println("Disconnect error: " + e.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Start server");
        Thread serverThread = new Thread(() -> {
            ChatServer server = new ChatServer();
            server.start();
        });
        serverThread.start();
        
        try {
            Thread.sleep(1000);
            
            System.out.println("\\nTest Case 2: Connect client 1");
            ChatClient client1 = new ChatClient();
            client1.connect("localhost", PORT);
            Thread.sleep(500);
            
            System.out.println("\\nTest Case 3: Connect client 2");
            ChatClient client2 = new ChatClient();
            client2.connect("localhost", PORT);
            Thread.sleep(500);
            
            System.out.println("\\nTest Case 4: Send messages");
            client1.sendMessage("Hello from client 1");
            Thread.sleep(500);
            client2.sendMessage("Hello from client 2");
            Thread.sleep(500);
            
            System.out.println("\\nTest Case 5: Disconnect clients");
            client1.disconnect();
            Thread.sleep(500);
            client2.disconnect();
            
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
