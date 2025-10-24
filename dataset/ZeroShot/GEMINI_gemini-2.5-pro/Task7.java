import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.*;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Task7 {
    private static final String USER_FILE = "users.dat";
    private static final int PORT = 12345;
    private static final String HOST = "127.0.0.1";

    // --- Utility for Password Hashing (PBKDF2) ---
    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    private static byte[] hexToBytes(String hex) {
        int len = hex.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(hex.charAt(i), 16) << 4)
                                 + Character.digit(hex.charAt(i + 1), 16));
        }
        return data;
    }

    private static byte[] getSalt() throws NoSuchAlgorithmException {
        SecureRandom sr = SecureRandom.getInstance("SHA1PRNG");
        byte[] salt = new byte[16];
        sr.nextBytes(salt);
        return salt;
    }

    private static String hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, 65536, 128);
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
        byte[] hash = factory.generateSecret(spec).getEncoded();
        return bytesToHex(hash);
    }

    // --- Main method to run Client, Server, or Add User ---
    public static void main(String[] args) {
        if (args.length == 0) {
            printUsage();
            return;
        }

        switch (args[0].toLowerCase()) {
            case "server":
                new Server().start();
                break;
            case "client":
                new Client().start();
                break;
            case "adduser":
                if (args.length != 3) {
                    System.out.println("Usage: java Task7 adduser <username> <password>");
                } else {
                    addUser(args[1], args[2]);
                }
                break;
            default:
                printUsage();
                break;
        }
    }

    private static void printUsage() {
        System.out.println("--- Secure Chat System ---");
        System.out.println("1. To add a user (run this first):");
        System.out.println("   java Task7 adduser <username> <password>");
        System.out.println("\n2. To start the server:");
        System.out.println("   java Task7 server");
        System.out.println("\n3. To start the client (in a new terminal):");
        System.out.println("   java Task7 client");
        System.out.println("\n--- Test Cases to run on Client ---");
        System.out.println("1. Login with correct credentials.");
        System.out.println("2. Try to login with correct user, wrong password.");
        System.out.println("3. Try to login with a non-existent user.");
        System.out.println("4. Login correctly and send a message.");
        System.out.println("5. Try to login with an empty username or password.");
    }
    
    private static void addUser(String username, String password) {
        if (username.trim().isEmpty() || password.trim().isEmpty()) {
            System.out.println("Username and password cannot be empty.");
            return;
        }
        try (PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(USER_FILE, true)))) {
            byte[] salt = getSalt();
            String hashedPassword = hashPassword(password, salt);
            out.println(username + ":" + bytesToHex(salt) + ":" + hashedPassword);
            System.out.println("User '" + username + "' added successfully.");
        } catch (IOException | NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error adding user: " + e.getMessage());
        }
    }

    // --- Server Implementation ---
    static class Server {
        private final Map<String, String> userCredentials = new HashMap<>();
        private final ExecutorService pool = Executors.newCachedThreadPool();
        private final Map<String, PrintWriter> connectedClients = new ConcurrentHashMap<>();

        public Server() {
            loadUsers();
        }

        private void loadUsers() {
            File file = new File(USER_FILE);
            if (!file.exists()) {
                System.out.println("User file not found. Please add users first.");
                return;
            }
            try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    userCredentials.put(line.split(":")[0], line);
                }
                System.out.println("Loaded " + userCredentials.size() + " users.");
            } catch (IOException e) {
                System.err.println("Error loading user file: " + e.getMessage());
            }
        }

        public void start() {
            System.out.println("Server started on port " + PORT);
            try (ServerSocket serverSocket = new ServerSocket(PORT)) {
                while (true) {
                    Socket clientSocket = serverSocket.accept();
                    pool.execute(new ClientHandler(clientSocket));
                }
            } catch (IOException e) {
                System.err.println("Server error: " + e.getMessage());
            }
        }

        private class ClientHandler implements Runnable {
            private final Socket socket;
            private BufferedReader in;
            private PrintWriter out;
            private String username;

            ClientHandler(Socket socket) {
                this.socket = socket;
            }

            @Override
            public void run() {
                try {
                    in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                    out = new PrintWriter(socket.getOutputStream(), true);

                    if (authenticate()) {
                        connectedClients.put(username, out);
                        broadcast(username + " has joined the chat.");
                        
                        String message;
                        while ((message = in.readLine()) != null) {
                            if (message.equalsIgnoreCase("LOGOUT")) {
                                break;
                            }
                             broadcast(username + ": " + message);
                        }
                    }
                } catch (IOException e) {
                    System.err.println("Client handler error: " + e.getMessage());
                } finally {
                    if (username != null) {
                        connectedClients.remove(username);
                        broadcast(username + " has left the chat.");
                    }
                    try {
                        socket.close();
                    } catch (IOException e) {
                        // Ignore
                    }
                }
            }
            
            private boolean authenticate() throws IOException {
                out.println("SUBMIT_credentials");
                String line = in.readLine(); // "LOGIN username password"
                if (line == null || !line.startsWith("LOGIN ")) {
                    out.println("LOGIN_FAIL Invalid request format.");
                    return false;
                }
                String[] parts = line.split(" ", 3);
                 if (parts.length != 3) {
                    out.println("LOGIN_FAIL Invalid credentials format.");
                    return false;
                }
                
                String providedUsername = parts[1];
                String providedPassword = parts[2];
                
                String storedRecord = userCredentials.get(providedUsername);
                if(storedRecord == null){
                     out.println("LOGIN_FAIL Invalid username or password.");
                     return false;
                }
                
                String[] creds = storedRecord.split(":");
                byte[] salt = hexToBytes(creds[1]);
                String storedHash = creds[2];
                
                try{
                    String providedHash = hashPassword(providedPassword, salt);
                    if(providedHash.equals(storedHash)){
                        this.username = providedUsername;
                        out.println("LOGIN_SUCCESS");
                        return true;
                    }
                } catch(NoSuchAlgorithmException | InvalidKeySpecException e){
                    System.err.println("Authentication error: " + e.getMessage());
                }
                
                out.println("LOGIN_FAIL Invalid username or password.");
                return false;
            }

            private void broadcast(String message) {
                System.out.println("Broadcasting: " + message);
                for (PrintWriter writer : connectedClients.values()) {
                    writer.println(message);
                }
            }
        }
    }

    // --- Client Implementation ---
    static class Client {
        public void start() {
            try (Socket socket = new Socket(HOST, PORT);
                 PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                 BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                 BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in))) {
                
                System.out.println("Connected to server.");
                
                String serverResponse = in.readLine();
                if ("SUBMIT_credentials".equals(serverResponse)) {
                    System.out.print("Enter username: ");
                    String username = consoleReader.readLine();
                    System.out.print("Enter password: ");
                    String password = consoleReader.readLine();

                    out.println("LOGIN " + username + " " + password);
                    
                    String loginResponse = in.readLine();
                    System.out.println("Server: " + loginResponse);
                    
                    if ("LOGIN_SUCCESS".equals(loginResponse)) {
                        System.out.println("Login successful! You can now chat. Type 'LOGOUT' to exit.");
                        
                        // Thread to listen for server messages
                        Thread listenerThread = new Thread(() -> {
                            try {
                                String fromServer;
                                while ((fromServer = in.readLine()) != null) {
                                    System.out.println(fromServer);
                                }
                            } catch (IOException e) {
                                System.out.println("Connection to server lost.");
                            }
                        });
                        listenerThread.start();
                        
                        // Main thread to send messages
                        String fromUser;
                        while ((fromUser = consoleReader.readLine()) != null) {
                             if ("LOGOUT".equalsIgnoreCase(fromUser)) {
                                out.println("LOGOUT");
                                break;
                            }
                            out.println(fromUser);
                        }
                        listenerThread.interrupt();
                    }
                }
            } catch (IOException e) {
                System.err.println("Client error: " + e.getMessage());
            }
        }
    }
}