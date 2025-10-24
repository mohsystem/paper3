import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Task48 {

    private static final int PORT = 12345;
    private static final String HOST = "127.0.0.1";

    // Server implementation
    static class Server implements Runnable {
        private ServerSocket serverSocket;
        private final List<ClientHandler> clients = new CopyOnWriteArrayList<>();
        private boolean running = true;
        private ExecutorService pool = Executors.newCachedThreadPool();

        @Override
        public void run() {
            try {
                serverSocket = new ServerSocket(PORT);
                System.out.println("Server started on port " + PORT);
                while (running) {
                    try {
                        Socket clientSocket = serverSocket.accept();
                        ClientHandler clientHandler = new ClientHandler(clientSocket, this);
                        clients.add(clientHandler);
                        pool.execute(clientHandler);
                    } catch (IOException e) {
                        if (running) {
                            System.err.println("Error accepting client connection: " + e.getMessage());
                        }
                    }
                }
            } catch (IOException e) {
                if (running) {
                    System.err.println("Could not listen on port " + PORT + ": " + e.getMessage());
                }
            } finally {
                stopServer();
            }
        }

        public void broadcastMessage(String message, ClientHandler sender) {
            System.out.println("Broadcasting: " + message);
            for (ClientHandler client : clients) {
                if (client != sender) {
                    client.sendMessage(message);
                }
            }
        }

        public void removeClient(ClientHandler clientHandler) {
            clients.remove(clientHandler);
            System.out.println("Client disconnected. Total clients: " + clients.size());
        }

        public void stopServer() {
            running = false;
            pool.shutdown();
            try {
                if (serverSocket != null && !serverSocket.isClosed()) {
                    serverSocket.close();
                }
            } catch (IOException e) {
                System.err.println("Error closing server socket: " + e.getMessage());
            }
            System.out.println("Server stopped.");
        }
    }

    // Client Handler implementation
    static class ClientHandler implements Runnable {
        private Socket clientSocket;
        private Server server;
        private PrintWriter out;
        private BufferedReader in;

        public ClientHandler(Socket socket, Server server) {
            this.clientSocket = socket;
            this.server = server;
        }

        @Override
        public void run() {
            try {
                out = new PrintWriter(clientSocket.getOutputStream(), true);
                in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                
                String clientName = "Client" + Thread.currentThread().getId();
                System.out.println(clientName + " connected.");
                server.broadcastMessage(clientName + " has joined the chat.", this);

                String inputLine;
                while ((inputLine = in.readLine()) != null) {
                    server.broadcastMessage(clientName + ": " + inputLine, this);
                }
            } catch (IOException e) {
                // Client disconnected
            } finally {
                try {
                    in.close();
                    out.close();
                    clientSocket.close();
                } catch (IOException e) {
                    // Ignore
                }
                server.removeClient(this);
            }
        }

        public void sendMessage(String message) {
            out.println(message);
        }
    }

    // Client simulation
    static class Client implements Runnable {
        private final String name;
        private final String[] messages;

        public Client(String name, String[] messages) {
            this.name = name;
            this.messages = messages;
        }

        @Override
        public void run() {
            try (Socket socket = new Socket(HOST, PORT)) {
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

                // Thread to listen for messages from the server
                Thread listenerThread = new Thread(() -> {
                    try {
                        String fromServer;
                        while ((fromServer = in.readLine()) != null) {
                            System.out.println(name + " received: " + fromServer);
                        }
                    } catch (IOException e) {
                        // Socket closed, thread will terminate
                    }
                });
                listenerThread.start();
                
                // Send messages
                for (String msg : messages) {
                    out.println(msg);
                    Thread.sleep((long) (Math.random() * 1000) + 500);
                }
                // Give some time for final messages to be received
                Thread.sleep(1000);

            } catch (IOException | InterruptedException e) {
                System.err.println(name + " error: " + e.getMessage());
            } finally {
                System.out.println(name + " is done.");
            }
        }
    }

    // Main method to run the simulation
    public static void main(String[] args) {
        Server server = new Server();
        Thread serverThread = new Thread(server);
        serverThread.start();

        try {
            // Give server time to start
            Thread.sleep(1000);

            // Test cases: 5 clients connect and send messages
            Thread[] clientThreads = new Thread[5];
            clientThreads[0] = new Thread(new Client("TestClient1", new String[]{"Hello everyone!", "How are you?"}));
            clientThreads[1] = new Thread(new Client("TestClient2", new String[]{"Hi!", "I am fine, thanks!"}));
            clientThreads[2] = new Thread(new Client("TestClient3", new String[]{"Good day.", "Any news?"}));
            clientThreads[3] = new Thread(new Client("TestClient4", new String[]{"Hey there!", "Just joined."}));
            clientThreads[4] = new Thread(new Client("TestClient5", new String[]{"Greetings.", "Let's chat."}));

            for (Thread t : clientThreads) {
                t.start();
            }

            // Wait for all clients to finish
            for (Thread t : clientThreads) {
                t.join();
            }
            
            // Stop the server
            server.stopServer();
            serverThread.join();

        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.err.println("Main thread interrupted.");
        }
        System.out.println("Chat simulation finished.");
    }
}