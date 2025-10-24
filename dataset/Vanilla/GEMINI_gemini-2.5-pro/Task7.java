import java.io.*;
import java.net.*;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class Task7 {

    private static final int PORT = 12345;
    private static final String USER_FILE = "users.txt";
    private static final String HOST = "127.0.0.1";
    private static volatile boolean serverReady = false;
    private static volatile boolean serverShouldStop = false;

    // Server Component
    private static class Server {
        private ServerSocket serverSocket;
        private ExecutorService pool = Executors.newCachedThreadPool();
        private Map<String, String> users = new HashMap<>();

        public void start() {
            try {
                loadUsers();
                serverSocket = new ServerSocket(PORT);
                System.out.println("Server started on port " + PORT);
                serverReady = true; // Signal that the server is ready

                while (!serverShouldStop) {
                    try {
                        Socket clientSocket = serverSocket.accept();
                        pool.execute(new ClientHandler(clientSocket, this));
                    } catch (SocketException e) {
                        if (serverShouldStop) {
                            System.out.println("Server is shutting down.");
                        } else {
                            e.printStackTrace();
                        }
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                stopServer();
            }
        }

        public void stopServer() {
            serverShouldStop = true;
            pool.shutdownNow();
            try {
                if (serverSocket != null && !serverSocket.isClosed()) {
                    serverSocket.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
            System.out.println("Server stopped.");
        }

        private void loadUsers() {
            try (BufferedReader reader = new BufferedReader(new FileReader(USER_FILE))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    String[] parts = line.split(":", 2);
                    if (parts.length == 2) {
                        users.put(parts[0], parts[1]);
                    }
                }
                System.out.println("Users loaded from " + USER_FILE);
            } catch (IOException e) {
                System.err.println("Could not load users file: " + e.getMessage());
            }
        }

        public String authenticate(String username, String password) {
            if (!users.containsKey(username)) {
                return "LOGIN_FAIL User not found";
            }
            if (users.get(username).equals(password)) {
                return "LOGIN_SUCCESS";
            } else {
                return "LOGIN_FAIL Invalid password";
            }
        }
    }

    private static class ClientHandler implements Runnable {
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

                String line;
                while ((line = in.readLine()) != null) {
                    System.out.println("Server received: " + line);
                    String[] parts = line.split(" ");
                    String command = parts[0];

                    if ("LOGIN".equalsIgnoreCase(command) && parts.length == 3) {
                        String response = server.authenticate(parts[1], parts[2]);
                        out.println(response);
                    } else if ("SHUTDOWN".equalsIgnoreCase(command)){
                         serverShouldStop = true;
                         // Force the serverSocket.accept() to unblock
                         new Socket(HOST, PORT).close();
                         break;
                    }else {
                        out.println("ERROR Unknown command");
                    }
                }
            } catch (IOException e) {
                // Connection closed
            } finally {
                try {
                    clientSocket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    // Client Component
    public static void runClient() {
        System.out.println("\n--- Starting Client Test Cases ---");
        try (Socket socket = new Socket(HOST, PORT);
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))) {
            
            // Test Case 1: Successful login
            System.out.println("Client> Test 1: Correct credentials (alice)");
            String request1 = "LOGIN alice pass123";
            out.println(request1);
            System.out.println("Client sent: " + request1);
            System.out.println("Server response: " + in.readLine());

            // Test Case 2: Incorrect password
            System.out.println("\nClient> Test 2: Incorrect password (bob)");
            String request2 = "LOGIN bob wrongpass";
            out.println(request2);
            System.out.println("Client sent: " + request2);
            System.out.println("Server response: " + in.readLine());

            // Test Case 3: User not found
            System.out.println("\nClient> Test 3: User not found (dave)");
            String request3 = "LOGIN dave somepass";
            out.println(request3);
            System.out.println("Client sent: " + request3);
            System.out.println("Server response: " + in.readLine());

            // Test Case 4: Another successful login
            System.out.println("\nClient> Test 4: Correct credentials (charlie)");
            String request4 = "LOGIN charlie qwerty";
            out.println(request4);
            System.out.println("Client sent: " + request4);
            System.out.println("Server response: " + in.readLine());

            // Test Case 5: Malformed request
            System.out.println("\nClient> Test 5: Malformed request");
            String request5 = "LOGI charlie qwerty";
            out.println(request5);
            System.out.println("Client sent: " + request5);
            System.out.println("Server response: " + in.readLine());
            
            // Send shutdown signal
            out.println("SHUTDOWN");


        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println("--- Client Test Cases Finished ---");
    }
    
    // Main method to run both
    public static void main(String[] args) {
        // 1. Create the user file for the server
        try (PrintWriter writer = new PrintWriter(new FileWriter(USER_FILE))) {
            writer.println("alice:pass123");
            writer.println("bob:secret");
            writer.println("charlie:qwerty");
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        // 2. Start the server in a new thread
        Thread serverThread = new Thread(() -> {
            Server server = new Server();
            server.start();
        });
        serverThread.start();

        // 3. Wait for the server to be ready
        while (!serverReady) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }

        // 4. Run the client to perform test cases
        runClient();

        // 5. Wait for the server thread to finish
        try {
            serverThread.join(2000); // Wait for up to 2 seconds
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        
        // 6. Clean up the user file
        new File(USER_FILE).delete();
        System.out.println("\nProgram finished.");
    }
}