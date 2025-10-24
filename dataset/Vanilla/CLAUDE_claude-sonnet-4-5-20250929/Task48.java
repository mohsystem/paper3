
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

class Task48 {
    private static final int PORT = 12345;
    private static Set<ClientHandler> clients = ConcurrentHashMap.newKeySet();
    
    static class ClientHandler implements Runnable {
        private Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        private String clientName;
        
        public ClientHandler(Socket socket) {
            this.socket = socket;
        }
        
        @Override
        public void run() {
            try {
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                out = new PrintWriter(socket.getOutputStream(), true);
                
                out.println("Enter your name:");
                clientName = in.readLine();
                broadcast(clientName + " has joined the chat!");
                
                String message;
                while ((message = in.readLine()) != null) {
                    if (message.equalsIgnoreCase("/quit")) {
                        break;
                    }
                    broadcast(clientName + ": " + message);
                }
            } catch (IOException e) {
                System.out.println("Error handling client: " + e.getMessage());
            } finally {
                cleanup();
            }
        }
        
        private void cleanup() {
            try {
                clients.remove(this);
                if (clientName != null) {
                    broadcast(clientName + " has left the chat!");
                }
                if (socket != null) socket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        
        public void sendMessage(String message) {
            if (out != null) {
                out.println(message);
            }
        }
    }
    
    private static void broadcast(String message) {
        System.out.println("[Server] " + message);
        for (ClientHandler client : clients) {
            client.sendMessage(message);
        }
    }
    
    public static void startServer() {
        System.out.println("Chat server starting on port " + PORT);
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            while (true) {
                Socket clientSocket = serverSocket.accept();
                ClientHandler handler = new ClientHandler(clientSocket);
                clients.add(handler);
                new Thread(handler).start();
            }
        } catch (IOException e) {
            System.out.println("Server error: " + e.getMessage());
        }
    }
    
    static class ChatClient {
        private Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        
        public void connect(String host, int port, String name) throws IOException {
            socket = new Socket(host, port);
            out = new PrintWriter(socket.getOutputStream(), true);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            
            System.out.println(in.readLine());
            out.println(name);
            
            new Thread(() -> {
                try {
                    String message;
                    while ((message = in.readLine()) != null) {
                        System.out.println(message);
                    }
                } catch (IOException e) {
                    System.out.println("Connection closed");
                }
            }).start();
        }
        
        public void sendMessage(String message) {
            if (out != null) {
                out.println(message);
            }
        }
        
        public void disconnect() throws IOException {
            if (socket != null) socket.close();
        }
    }
    
    public static void main(String[] args) {
        Thread serverThread = new Thread(() -> startServer());
        serverThread.setDaemon(true);
        serverThread.start();
        
        try {
            Thread.sleep(1000);
            
            System.out.println("\\n=== Test Case 1: Single client connection ===");
            ChatClient client1 = new ChatClient();
            client1.connect("localhost", PORT, "Alice");
            Thread.sleep(500);
            client1.sendMessage("Hello from Alice!");
            Thread.sleep(500);
            
            System.out.println("\\n=== Test Case 2: Multiple clients ===");
            ChatClient client2 = new ChatClient();
            client2.connect("localhost", PORT, "Bob");
            Thread.sleep(500);
            client2.sendMessage("Hi everyone from Bob!");
            Thread.sleep(500);
            
            System.out.println("\\n=== Test Case 3: Broadcasting ===");
            client1.sendMessage("This is Alice speaking");
            Thread.sleep(500);
            client2.sendMessage("Bob here too");
            Thread.sleep(500);
            
            System.out.println("\\n=== Test Case 4: Third client joins ===");
            ChatClient client3 = new ChatClient();
            client3.connect("localhost", PORT, "Charlie");
            Thread.sleep(500);
            client3.sendMessage("Charlie joined the chat");
            Thread.sleep(500);
            
            System.out.println("\\n=== Test Case 5: Client disconnection ===");
            client1.sendMessage("/quit");
            Thread.sleep(500);
            client2.sendMessage("Alice left");
            Thread.sleep(500);
            
            client2.disconnect();
            client3.disconnect();
            Thread.sleep(1000);
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
