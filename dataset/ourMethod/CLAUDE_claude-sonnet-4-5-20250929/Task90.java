
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class Task90 {
    private static final int MAX_CLIENTS = 100;
    private static final int MAX_MESSAGE_LENGTH = 1024;
    private static final int MIN_PORT = 1024;
    private static final int MAX_PORT = 65535;
    private static final Set<ClientHandler> clients = ConcurrentHashMap.newKeySet();
    private static final AtomicInteger clientCounter = new AtomicInteger(0);

    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: Start server on port 8080");
        testServer(8080);
        
        System.out.println("\\nTest 2: Start server on port 9000");
        testServer(9000);
        
        System.out.println("\\nTest 3: Start server on port 7777");
        testServer(7777);
        
        System.out.println("\\nTest 4: Start server on port 5555");
        testServer(5555);
        
        System.out.println("\\nTest 5: Start server on port 6666");
        testServer(6666);
    }

    private static void testServer(int port) {
        Thread serverThread = new Thread(() -> {
            try {
                startServer(port);
            } catch (IOException e) {
                System.err.println("Server error: " + e.getMessage());
            }
        });
        serverThread.setDaemon(true);
        serverThread.start();
        
        try {
            Thread.sleep(1000);
            testClient(port, "Client1");
            Thread.sleep(500);
            testClient(port, "Client2");
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }

    private static void testClient(int port, String clientName) {
        Thread clientThread = new Thread(() -> {
            try (Socket socket = new Socket("localhost", port);
                 PrintWriter out = new PrintWriter(new OutputStreamWriter(
                     socket.getOutputStream(), StandardCharsets.UTF_8), true);
                 BufferedReader in = new BufferedReader(new InputStreamReader(
                     socket.getInputStream(), StandardCharsets.UTF_8))) {
                
                out.println("Hello from " + clientName);
                String response = in.readLine();
                if (response != null) {
                    System.out.println(clientName + " received: " + response);
                }
            } catch (IOException e) {
                System.err.println(clientName + " error: " + e.getMessage());
            }
        });
        clientThread.setDaemon(true);
        clientThread.start();
    }

    public static void startServer(int port) throws IOException {
        if (port < MIN_PORT || port > MAX_PORT) {
            throw new IllegalArgumentException("Port must be between " + MIN_PORT + " and " + MAX_PORT);
        }

        try (ServerSocket serverSocket = new ServerSocket(port)) {
            serverSocket.setSoTimeout(5000);
            System.out.println("Server started on port " + port);

            while (clientCounter.get() < MAX_CLIENTS) {
                try {
                    Socket clientSocket = serverSocket.accept();
                    if (clientCounter.get() >= MAX_CLIENTS) {
                        clientSocket.close();
                        continue;
                    }
                    
                    ClientHandler handler = new ClientHandler(clientSocket);
                    clients.add(handler);
                    clientCounter.incrementAndGet();
                    new Thread(handler).start();
                } catch (SocketTimeoutException e) {
                    break;
                } catch (IOException e) {
                    System.err.println("Error accepting client: " + e.getMessage());
                }
            }
        }
    }

    private static class ClientHandler implements Runnable {
        private final Socket socket;
        private PrintWriter out;
        private BufferedReader in;
        private final int clientId;

        public ClientHandler(Socket socket) {
            this.socket = socket;
            this.clientId = clientCounter.get();
        }

        @Override
        public void run() {
            try {
                in = new BufferedReader(new InputStreamReader(
                    socket.getInputStream(), StandardCharsets.UTF_8));
                out = new PrintWriter(new OutputStreamWriter(
                    socket.getOutputStream(), StandardCharsets.UTF_8), true);

                String message;
                while ((message = in.readLine()) != null) {
                    if (message.length() > MAX_MESSAGE_LENGTH) {
                        message = message.substring(0, MAX_MESSAGE_LENGTH);
                    }
                    
                    String sanitized = sanitizeMessage(message);
                    broadcast("[Client" + clientId + "]: " + sanitized);
                }
            } catch (IOException e) {
                System.err.println("Client handler error: " + e.getMessage());
            } finally {
                cleanup();
            }
        }

        private String sanitizeMessage(String message) {
            if (message == null) {
                return "";
            }
            return message.replaceAll("[^\\\\p{Print}\\\\p{Space}]", "").trim();
        }

        private void broadcast(String message) {
            for (ClientHandler client : clients) {
                if (client.out != null) {
                    try {
                        client.out.println(message);
                    } catch (Exception e) {
                        System.err.println("Broadcast error: " + e.getMessage());
                    }
                }
            }
        }

        private void cleanup() {
            try {
                clients.remove(this);
                clientCounter.decrementAndGet();
                if (in != null) in.close();
                if (out != null) out.close();
                if (socket != null && !socket.isClosed()) socket.close();
            } catch (IOException e) {
                System.err.println("Cleanup error: " + e.getMessage());
            }
        }
    }
}
