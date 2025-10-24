import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

public class Task7 {

    private static final String USER_DB_FILE = "users.db";
    private static final int PORT = 12345;
    private static final Pattern VALID_USERNAME = Pattern.compile("^[a-zA-Z0-9_]{3,16}$");

    private static class PasswordUtils {
        private static final int SALT_LENGTH = 16;
        private static final int ITERATIONS = 210000;
        private static final int KEY_LENGTH = 256;
        private static final String ALGORITHM = "PBKDF2WithHmacSHA256";

        public static byte[] getSalt() {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH];
            random.nextBytes(salt);
            return salt;
        }

        public static byte[] hashPassword(char[] password, byte[] salt)
                throws NoSuchAlgorithmException, InvalidKeySpecException {
            KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            return factory.generateSecret(spec).getEncoded();
        }

        public static String encode(byte[] data) {
            return Base64.getEncoder().encodeToString(data);
        }

        public static byte[] decode(String data) {
            return Base64.getDecoder().decode(data);
        }
    }

    private static class UserStore {
        private final Path dbPath;

        public UserStore(String filename) {
            this.dbPath = Paths.get(filename).toAbsolutePath();
        }

        public synchronized boolean addUser(String username, String password) {
            if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()
                    || !VALID_USERNAME.matcher(username).matches()) {
                System.err.println("Invalid username or password format.");
                return false;
            }

            try (BufferedReader reader = Files.newBufferedReader(dbPath, StandardCharsets.UTF_8)) {
                String line;
                while ((line = reader.readLine()) != null) {
                    if (line.startsWith(username + ":")) {
                        return false; // User already exists
                    }
                }
            } catch (IOException e) {
                // File does not exist, which is fine for the first user.
            }

            try (BufferedWriter writer = Files.newBufferedWriter(dbPath, StandardCharsets.UTF_8,
                    java.nio.file.StandardOpenOption.CREATE, java.nio.file.StandardOpenOption.APPEND)) {
                byte[] salt = PasswordUtils.getSalt();
                byte[] hashedPassword = PasswordUtils.hashPassword(password.toCharArray(), salt);
                String saltStr = PasswordUtils.encode(salt);
                String hashStr = PasswordUtils.encode(hashedPassword);
                writer.write(username + ":" + saltStr + ":" + hashStr);
                writer.newLine();
                return true;
            } catch (IOException | NoSuchAlgorithmException | InvalidKeySpecException e) {
                e.printStackTrace();
                return false;
            }
        }

        public synchronized boolean authenticate(String username, String password) {
            try (BufferedReader reader = Files.newBufferedReader(dbPath, StandardCharsets.UTF_8)) {
                String line;
                while ((line = reader.readLine()) != null) {
                    String[] parts = line.split(":", 3);
                    if (parts.length == 3 && parts[0].equals(username)) {
                        byte[] salt = PasswordUtils.decode(parts[1]);
                        byte[] storedHash = PasswordUtils.decode(parts[2]);
                        byte[] providedHash = PasswordUtils.hashPassword(password.toCharArray(), salt);
                        return Arrays.equals(storedHash, providedHash);
                    }
                }
            } catch (IOException | NoSuchAlgorithmException | InvalidKeySpecException e) {
                e.printStackTrace();
            }
            return false;
        }

        public void cleanup() {
            try {
                Files.deleteIfExists(dbPath);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private static class Server implements Runnable {
        private volatile boolean running = true;
        private ServerSocket serverSocket;
        private final UserStore userStore = new UserStore(USER_DB_FILE);
        private final ExecutorService pool = Executors.newCachedThreadPool();

        public void stop() {
            running = false;
            try {
                if (serverSocket != null && !serverSocket.isClosed()) {
                    serverSocket.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
            pool.shutdown();
            try {
                if (!pool.awaitTermination(5, TimeUnit.SECONDS)) {
                    pool.shutdownNow();
                }
            } catch (InterruptedException e) {
                pool.shutdownNow();
                Thread.currentThread().interrupt();
            }
            userStore.cleanup();
            System.out.println("Server stopped.");
        }
        
        @Override
        public void run() {
            try {
                serverSocket = new ServerSocket(PORT);
                System.out.println("Server listening on port " + PORT);
                while (running) {
                    try {
                        Socket clientSocket = serverSocket.accept();
                        pool.execute(() -> handleClient(clientSocket));
                    } catch (IOException e) {
                        if (!running) {
                            break; 
                        }
                        e.printStackTrace();
                    }
                }
            } catch (IOException e) {
                if(running) e.printStackTrace();
            } finally {
                stop();
            }
        }

        private void handleClient(Socket clientSocket) {
            try (BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream(), StandardCharsets.UTF_8));
                 PrintWriter out = new PrintWriter(new OutputStreamWriter(clientSocket.getOutputStream(), StandardCharsets.UTF_8), true)) {
                
                String request = in.readLine();
                if (request == null) return;
                
                String[] parts = request.split(" ", 4);
                String command = parts[0];
                
                if ("REGISTER".equals(command) && parts.length == 3) {
                    if (userStore.addUser(parts[1], parts[2])) {
                        out.println("REGISTER_OK");
                    } else {
                        out.println("REGISTER_FAIL");
                    }
                } else if ("LOGIN".equals(command) && parts.length == 3) {
                    if (userStore.authenticate(parts[1], parts[2])) {
                        out.println("LOGIN_OK");
                    } else {
                        out.println("LOGIN_FAIL");
                    }
                } else {
                    out.println("INVALID_COMMAND");
                }
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                try {
                    clientSocket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private static class Client {
        public String sendRequest(String request) {
            try (Socket socket = new Socket("localhost", PORT);
                 PrintWriter out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8), true);
                 BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8))) {

                out.println(request);
                return in.readLine();
            } catch (IOException e) {
                return "ERROR: " + e.getMessage();
            }
        }
    }

    public static void main(String[] args) throws InterruptedException {
        Server server = new Server();
        Thread serverThread = new Thread(server);
        serverThread.start();
        
        // Give the server a moment to start up
        Thread.sleep(500);

        Client client = new Client();

        System.out.println("--- Test Cases ---");

        // Test Case 1: Register a new user
        System.out.println("Test 1: Register user 'testuser1'");
        String response1 = client.sendRequest("REGISTER testuser1 StrongPass123!");
        System.out.println("Server response: " + response1); // Expected: REGISTER_OK

        // Test Case 2: Successful login
        System.out.println("\nTest 2: Login with 'testuser1' (correct password)");
        String response2 = client.sendRequest("LOGIN testuser1 StrongPass123!");
        System.out.println("Server response: " + response2); // Expected: LOGIN_OK

        // Test Case 3: Failed login (wrong password)
        System.out.println("\nTest 3: Login with 'testuser1' (wrong password)");
        String response3 = client.sendRequest("LOGIN testuser1 WrongPassword");
        System.out.println("Server response: " + response3); // Expected: LOGIN_FAIL

        // Test Case 4: Failed login (non-existent user)
        System.out.println("\nTest 4: Login with 'no_such_user'");
        String response4 = client.sendRequest("LOGIN no_such_user some_password");
        System.out.println("Server response: " + response4); // Expected: LOGIN_FAIL

        // Test Case 5: Register another user and login
        System.out.println("\nTest 5: Register and login 'testuser2'");
        String response5a = client.sendRequest("REGISTER testuser2 AnotherSecurePass$");
        System.out.println("Server response (Register): " + response5a); // Expected: REGISTER_OK
        String response5b = client.sendRequest("LOGIN testuser2 AnotherSecurePass$");
        System.out.println("Server response (Login): " + response5b); // Expected: LOGIN_OK
        
        System.out.println("\n--- Tests Complete ---");

        server.stop();
        serverThread.join();
    }
}