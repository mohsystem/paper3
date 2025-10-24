
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

public class Task90 {
    private static Set<ClientHandler> clients = ConcurrentHashMap.newKeySet();
    private static int clientCounter = 0;

    public static void startServer(int port) {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Chat server started on port " + port);
            
            while (true) {
                Socket clientSocket = serverSocket.accept();
                ClientHandler clientHandler = new ClientHandler(clientSocket, ++clientCounter);
                clients.add(clientHandler);
                new Thread(clientHandler).start();
            }
        } catch (IOException e) {
            System.err.println("Server error: " + e.getMessage());
        }
    }

    public static void broadcastMessage(String message, ClientHandler sender) {
        for (ClientHandler client : clients) {
            if (client != sender) {
                client.sendMessage(message);
            }
        }
    }

    public static void removeClient(ClientHandler client) {
        clients.remove(client);
    }

    static class ClientHandler implements Runnable {
        private Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        private int clientId;
        private String username;

        public ClientHandler(Socket socket, int clientId) {
            this.socket = socket;
            this.clientId = clientId;
            this.username = "User" + clientId;
        }

        @Override
        public void run() {
            try {
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                out = new PrintWriter(socket.getOutputStream(), true);
                
                out.println("Welcome to the chat! You are " + username);
                broadcastMessage(username + " has joined the chat", this);
                
                String message;
                while ((message = in.readLine()) != null) {
                    System.out.println(username + ": " + message);
                    broadcastMessage(username + ": " + message, this);
                }
            } catch (IOException e) {
                System.err.println("Client error: " + e.getMessage());
            } finally {
                cleanup();
            }
        }

        public void sendMessage(String message) {
            if (out != null) {
                out.println(message);
            }
        }

        private void cleanup() {
            removeClient(this);
            broadcastMessage(username + " has left the chat", this);
            try {
                if (in != null) in.close();
                if (out != null) out.close();
                if (socket != null) socket.close();
            } catch (IOException e) {
                System.err.println("Error closing resources: " + e.getMessage());
            }
        }
    }

    public static void main(String[] args) {
        // Test case: Start server on port 8888
        System.out.println("Test Case 1: Starting server on port 8888");
        new Thread(() -> startServer(8888)).start();
        
        // Simulate 5 test clients
        try {
            Thread.sleep(1000);
            
            for (int i = 1; i <= 5; i++) {
                final int clientNum = i;
                new Thread(() -> {
                    try {
                        Socket socket = new Socket("localhost", 8888);
                        PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                        BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                        
                        // Read welcome message
                        System.out.println("Client " + clientNum + " received: " + in.readLine());
                        
                        // Send test messages
                        out.println("Hello from client " + clientNum);
                        Thread.sleep(500);
                        out.println("Message 2 from client " + clientNum);
                        
                        // Read broadcasts
                        String line;
                        int count = 0;
                        while ((line = in.readLine()) != null && count < 10) {
                            System.out.println("Client " + clientNum + " received: " + line);
                            count++;
                        }
                        
                        socket.close();
                    } catch (Exception e) {
                        System.err.println("Test client " + clientNum + " error: " + e.getMessage());
                    }
                }).start();
                
                Thread.sleep(200);
            }
            
            Thread.sleep(5000);
            System.out.println("\\nTest completed successfully!");
            
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
