import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Task7 {

    private static final String USER_FILE = "users.dat";
    private static final int PORT = 8080;
    private static final String HOST = "127.0.0.1";

    // --- Security Utilities ---
    private static class SecurityUtils {

        public static byte[] getSalt() throws NoSuchAlgorithmException {
            SecureRandom sr = SecureRandom.getInstance("SHA1PRNG");
            byte[] salt = new byte[16];
            sr.nextBytes(salt);
            return salt;
        }

        public static String hashPassword(String password, byte[] salt) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                md.update(salt);
                byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));
                return Base64.getEncoder().encodeToString(hashedPassword);
            } catch (NoSuchAlgorithmException e) {
                throw new RuntimeException("Could not find SHA-256 algorithm", e);
            }
        }

        public static boolean verifyPassword(String originalPassword, String storedHash, String saltStr) {
            byte[] salt = Base64.getDecoder().decode(saltStr);
            String newHash = hashPassword(originalPassword, salt);
            return newHash.equals(storedHash);
        }

        public static void addUser(String username, String password) {
            if (isUserExists(username)) {
                System.out.println("Error: User '" + username + "' already exists.");
                return;
            }
            try (PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(USER_FILE, true)))) {
                byte[] salt = getSalt();
                String hashedPassword = hashPassword(password, salt);
                String saltStr = Base64.getEncoder().encodeToString(salt);
                out.println(username + ":" + hashedPassword + ":" + saltStr);
                System.out.println("User '" + username + "' added successfully.");
            } catch (IOException | NoSuchAlgorithmException e) {
                System.err.println("Error adding user: " + e.getMessage());
            }
        }

        public static boolean isUserExists(String username) {
             File db = new File(USER_FILE);
             if (!db.exists()) return false;
             try (BufferedReader reader = new BufferedReader(new FileReader(USER_FILE))) {
                 String line;
                 while ((line = reader.readLine()) != null) {
                     String[] parts = line.split(":", 3);
                     if (parts.length > 0 && parts[0].equals(username)) {
                         return true;
                     }
                 }
             } catch (IOException e) {
                 System.err.println("Error checking user existence: " + e.getMessage());
             }
             return false;
        }
    }

    // --- Server ---
    private static class Server {
        public void start() {
            ExecutorService pool = Executors.newCachedThreadPool();
            try (ServerSocket serverSocket = new ServerSocket(PORT)) {
                System.out.println("Server listening on port " + PORT);
                while (true) {
                    Socket clientSocket = serverSocket.accept();
                    pool.execute(new ClientHandler(clientSocket));
                }
            } catch (IOException e) {
                System.err.println("Server error: " + e.getMessage());
            }
        }

        private static class ClientHandler implements Runnable {
            private final Socket clientSocket;

            public ClientHandler(Socket socket) {
                this.clientSocket = socket;
            }

            @Override
            public void run() {
                try (
                    BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                    PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true)
                ) {
                    String request = in.readLine();
                    if (request != null) {
                        System.out.println("Server received: " + request.split(" ")[0] + " " + request.split(" ")[1] + " *****");
                        boolean authenticated = processRequest(request);
                        if (authenticated) {
                            out.println("LOGIN_SUCCESS");
                            System.out.println("Authentication successful for " + request.split(" ")[1]);
                        } else {
                            out.println("LOGIN_FAIL");
                             System.out.println("Authentication failed for " + request.split(" ")[1]);
                        }
                    }
                } catch (IOException e) {
                    // System.err.println("Handler error: " + e.getMessage());
                } finally {
                    try {
                        clientSocket.close();
                    } catch (IOException e) {
                        // ignore
                    }
                }
            }

            private boolean processRequest(String request) {
                String[] parts = request.split(" ", 3);
                if (parts.length == 3 && parts[0].equals("LOGIN")) {
                    return authenticate(parts[1], parts[2]);
                }
                return false;
            }

            private boolean authenticate(String username, String password) {
                 File db = new File(USER_FILE);
                 if (!db.exists()) return false;
                 try (BufferedReader reader = new BufferedReader(new FileReader(USER_FILE))) {
                    String line;
                    while ((line = reader.readLine()) != null) {
                        String[] parts = line.split(":", 3);
                        if (parts.length == 3 && parts[0].equals(username)) {
                            return SecurityUtils.verifyPassword(password, parts[1], parts[2]);
                        }
                    }
                } catch (IOException e) {
                    System.err.println("Authentication error: " + e.getMessage());
                }
                return false;
            }
        }
    }

    // --- Client ---
    private static class Client {
        public void login(String username, String password) {
            try (
                Socket socket = new Socket(HOST, PORT);
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))
            ) {
                String request = "LOGIN " + username + " " + password;
                out.println(request);
                System.out.println("Client sent login request for user: " + username);

                String response = in.readLine();
                System.out.println("Server response: " + response);

            } catch (UnknownHostException e) {
                System.err.println("Don't know about host " + HOST);
            } catch (IOException e) {
                System.err.println("Couldn't get I/O for the connection to " + HOST + ". Is the server running?");
            }
        }
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            printUsage();
            return;
        }

        String mode = args[0];
        if ("server".equalsIgnoreCase(mode)) {
            new Server().start();
        } else if ("adduser".equalsIgnoreCase(mode)) {
            if (args.length != 3) {
                System.out.println("Usage: java Task7 adduser <username> <password>");
                return;
            }
            SecurityUtils.addUser(args[1], args[2]);
        } else if ("client".equalsIgnoreCase(mode)) {
            runClientTests();
        } else {
            printUsage();
        }
    }

    private static void printUsage() {
        System.out.println("Usage: java Task7 <mode> [options]");
        System.out.println("Modes:");
        System.out.println("  server                  - Start the chat server.");
        System.out.println("  adduser <user> <pass>   - Add a new user to the user file.");
        System.out.println("  client                  - Run predefined client test cases.");
    }

    private static void runClientTests() {
        System.out.println("\n--- Running Client Test Cases ---");
        System.out.println("Instructions:");
        System.out.println("1. Start the server in a separate terminal: java Task7 server");
        System.out.println("2. Add a test user in another terminal: java Task7 adduser testuser testpass123");
        System.out.println("3. The test cases below will now run against the server.\n");

        Client client = new Client();
        
        System.out.println("--- Test Case 1: Successful Login ---");
        client.login("testuser", "testpass123");
        
        System.out.println("\n--- Test Case 2: Incorrect Password ---");
        client.login("testuser", "wrongpassword");

        System.out.println("\n--- Test Case 3: Non-existent User ---");
        client.login("nouser", "anypassword");

        System.out.println("\n--- Test Case 4: Another Successful Login ---");
        client.login("testuser", "testpass123");

        System.out.println("\n--- Test Case 5: Empty Password ---");
        client.login("testuser", "");
    }
}