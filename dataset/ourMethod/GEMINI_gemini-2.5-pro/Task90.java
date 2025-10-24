import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Task90 {

    private static final int PORT = 12345;
    private static final int MAX_CLIENTS = 10;
    private static final List<ClientHandler> clients = new ArrayList<>();
    private static final ExecutorService pool = Executors.newFixedThreadPool(MAX_CLIENTS);

    public static void main(String[] args) {
        System.out.println("Java Chat Server is running...");
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            while (true) {
                try {
                    Socket clientSocket = serverSocket.accept();
                    synchronized (clients) {
                        if (clients.size() < MAX_CLIENTS) {
                            ClientHandler clientThread = new ClientHandler(clientSocket);
                            clients.add(clientThread);
                            pool.execute(clientThread);
                        } else {
                            System.out.println("Connection refused: maximum clients reached.");
                            try (PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true, StandardCharsets.UTF_8)) {
                                out.println("Server is full. Please try again later.");
                            }
                            clientSocket.close();
                        }
                    }
                } catch (IOException e) {
                    System.err.println("Error accepting client connection: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Could not listen on port " + PORT + ": " + e.getMessage());
        } finally {
            pool.shutdown();
        }
    }

    private static void broadcastMessage(String message, ClientHandler sender) {
        synchronized (clients) {
            for (ClientHandler client : clients) {
                if (client != sender) {
                    client.sendMessage(message);
                }
            }
        }
    }
    
    private static void removeClient(ClientHandler client) {
        synchronized (clients) {
            clients.remove(client);
            System.out.println("Client " + client.getClientName() + " disconnected.");
        }
    }

    private static class ClientHandler implements Runnable {
        private final Socket clientSocket;
        private PrintWriter out;
        private String clientName;
        private static int connectionCounter = 0;

        public ClientHandler(Socket socket) {
            this.clientSocket = socket;
            synchronized (ClientHandler.class) {
                this.clientName = "Client-" + (++connectionCounter);
            }
        }
        
        public String getClientName() {
            return clientName;
        }

        @Override
        public void run() {
            try (
                BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream(), StandardCharsets.UTF_8))
            ) {
                out = new PrintWriter(clientSocket.getOutputStream(), true, StandardCharsets.UTF_8);
                System.out.println(clientName + " connected from " + clientSocket.getInetAddress().getHostAddress());
                broadcastMessage(clientName + " has joined the chat.", this);

                String inputLine;
                while ((inputLine = in.readLine()) != null) {
                    if (inputLine.trim().isEmpty()) continue;
                    String message = "[" + clientName + "]: " + inputLine;
                    System.out.print("Broadcasting: " + message + "\n");
                    broadcastMessage(message, this);
                }
            } catch (IOException e) {
                // This can happen if the client disconnects abruptly
            } finally {
                removeClient(this);
                broadcastMessage(clientName + " has left the chat.", this);
                try {
                    if (out != null) out.close();
                    clientSocket.close();
                } catch (IOException e) {
                    // Ignore
                }
            }
        }

        public void sendMessage(String message) {
            if (out != null) {
                out.println(message);
            }
        }
    }

    // A main method is provided to run the server.
    // To test, you can use a netcat or telnet client.
    // For example, in 5 separate terminals, run:
    // 1. Terminal 1: nc localhost 12345
    // 2. Terminal 2: nc localhost 12345
    // 3. Terminal 3: nc localhost 12345
    // 4. Terminal 4: nc localhost 12345
    // 5. Terminal 5: nc localhost 12345
    // Type messages in any terminal, and they should appear in all others.
}