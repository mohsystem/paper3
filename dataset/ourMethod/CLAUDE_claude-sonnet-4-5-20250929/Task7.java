
import javax.crypto.*;
import javax.crypto.spec.*;
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.*;
import java.security.spec.KeySpec;
import java.util.*;
import java.util.concurrent.*;

public class Task7 {
    private static final int PORT = 8443;
    private static final String USERS_FILE = "users.dat";
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 12;
    private static final int TAG_LENGTH = 16;
    private static final int MAX_USERNAME_LENGTH = 64;
    private static final int MAX_PASSWORD_LENGTH = 128;
    private static final int MAX_MESSAGE_LENGTH = 4096;
    
    static class SecurePassword {
        private final byte[] salt;
        private final byte[] hash;
        
        public SecurePassword(byte[] salt, byte[] hash) {
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
        }
        
        public byte[] getSalt() {
            return Arrays.copyOf(salt, salt.length);
        }
        
        public byte[] getHash() {
            return Arrays.copyOf(hash, hash.length);
        }
    }
    
    static class ChatServer {
        private final Map<String, SecurePassword> users = new ConcurrentHashMap<>();
        private final SecureRandom secureRandom = new SecureRandom();
        private final Path usersFilePath;
        
        public ChatServer(String usersFile) throws Exception {
            this.usersFilePath = Paths.get(usersFile).toAbsolutePath().normalize();
            Path baseDir = Paths.get(".").toAbsolutePath().normalize();
            if (!usersFilePath.startsWith(baseDir)) {
                throw new SecurityException("Users file path outside allowed directory");
            }
            loadUsers();
        }
        
        private void loadUsers() throws Exception {
            if (!Files.exists(usersFilePath)) {
                return;
            }
            
            if (!Files.isRegularFile(usersFilePath)) {
                throw new SecurityException("Users file is not a regular file");
            }
            
            try (BufferedReader reader = Files.newBufferedReader(usersFilePath, StandardCharsets.UTF_8)) {
                String line;
                while ((line = reader.readLine()) != null) {
                    if (line.trim().isEmpty() || line.length() > MAX_MESSAGE_LENGTH) {
                        continue;
                    }
                    String[] parts = line.split(":", 3);
                    if (parts.length == 3) {
                        String username = validateUsername(parts[0]);
                        byte[] salt = Base64.getDecoder().decode(parts[1]);
                        byte[] hash = Base64.getDecoder().decode(parts[2]);
                        if (salt.length == SALT_LENGTH && hash.length == 32) {
                            users.put(username, new SecurePassword(salt, hash));
                        }
                    }
                }
            }
        }
        
        private void saveUsers() throws Exception {
            Path tempFile = Files.createTempFile(usersFilePath.getParent(), "users", ".tmp");
            try {
                try (BufferedWriter writer = Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8)) {
                    for (Map.Entry<String, SecurePassword> entry : users.entrySet()) {
                        String username = entry.getKey();
                        SecurePassword secPass = entry.getValue();
                        writer.write(username + ":" + 
                                   Base64.getEncoder().encodeToString(secPass.getSalt()) + ":" +
                                   Base64.getEncoder().encodeToString(secPass.getHash()));
                        writer.newLine();
                    }
                }
                Files.move(tempFile, usersFilePath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (Exception e) {
                Files.deleteIfExists(tempFile);
                throw e;
            }
        }
        
        private String validateUsername(String username) throws IllegalArgumentException {
            if (username == null || username.isEmpty() || username.length() > MAX_USERNAME_LENGTH) {
                throw new IllegalArgumentException("Invalid username length");
            }
            if (!username.matches("[a-zA-Z0-9_]+")) {
                throw new IllegalArgumentException("Invalid username format");
            }
            return username;
        }
        
        private String validatePassword(String password) throws IllegalArgumentException {
            if (password == null || password.isEmpty() || password.length() > MAX_PASSWORD_LENGTH) {
                throw new IllegalArgumentException("Invalid password length");
            }
            if (password.length() < 8) {
                throw new IllegalArgumentException("Password too short");
            }
            return password;
        }
        
        private byte[] hashPassword(String password, byte[] salt) throws Exception {
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, 256);
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return factory.generateSecret(spec).getEncoded();
        }
        
        private boolean verifyPassword(String password, SecurePassword secPass) throws Exception {
            byte[] hash = hashPassword(password, secPass.getSalt());
            return MessageDigest.isEqual(hash, secPass.getHash());
        }
        
        public synchronized String registerUser(String username, String password) throws Exception {
            username = validateUsername(username);
            password = validatePassword(password);
            
            if (users.containsKey(username)) {
                return "ERROR:User already exists";
            }
            
            byte[] salt = new byte[SALT_LENGTH];
            secureRandom.nextBytes(salt);
            byte[] hash = hashPassword(password, salt);
            
            users.put(username, new SecurePassword(salt, hash));
            saveUsers();
            
            return "SUCCESS:User registered";
        }
        
        public String authenticateUser(String username, String password) throws Exception {
            username = validateUsername(username);
            password = validatePassword(password);
            
            SecurePassword secPass = users.get(username);
            if (secPass == null) {
                return "ERROR:Authentication failed";
            }
            
            if (verifyPassword(password, secPass)) {
                return "SUCCESS:Authenticated";
            } else {
                return "ERROR:Authentication failed";
            }
        }
        
        public String processRequest(String request) {
            if (request == null || request.length() > MAX_MESSAGE_LENGTH) {
                return "ERROR:Invalid request";
            }
            
            try {
                String[] parts = request.split(":", 3);
                if (parts.length < 3) {
                    return "ERROR:Invalid request format";
                }
                
                String command = parts[0];
                String username = parts[1];
                String password = parts[2];
                
                if ("REGISTER".equals(command)) {
                    return registerUser(username, password);
                } else if ("LOGIN".equals(command)) {
                    return authenticateUser(username, password);
                } else {
                    return "ERROR:Unknown command";
                }
            } catch (IllegalArgumentException e) {
                return "ERROR:Invalid input";
            } catch (Exception e) {
                return "ERROR:Server error";
            }
        }
    }
    
    static class ChatClient {
        private String serverHost;
        private int serverPort;
        
        public ChatClient(String serverHost, int serverPort) {
            this.serverHost = serverHost;
            this.serverPort = serverPort;
        }
        
        public String sendRequest(String request) throws Exception {
            if (request == null || request.length() > MAX_MESSAGE_LENGTH) {
                throw new IllegalArgumentException("Invalid request");
            }
            
            try (Socket socket = new Socket(serverHost, serverPort);
                 BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8));
                 PrintWriter out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8), true)) {
                
                socket.setSoTimeout(5000);
                out.println(request);
                String response = in.readLine();
                
                if (response == null || response.length() > MAX_MESSAGE_LENGTH) {
                    throw new IOException("Invalid response");
                }
                
                return response;
            }
        }
    }
    
    public static void main(String[] args) throws Exception {
        System.out.println("Test Case 1: Register user");
        ChatServer server1 = new ChatServer("test_users1.dat");
        String result1 = server1.processRequest("REGISTER:alice:SecurePass123!");
        System.out.println(result1);
        
        System.out.println("\\nTest Case 2: Login with correct credentials");
        String result2 = server1.processRequest("LOGIN:alice:SecurePass123!");
        System.out.println(result2);
        
        System.out.println("\\nTest Case 3: Login with incorrect password");
        String result3 = server1.processRequest("LOGIN:alice:WrongPassword");
        System.out.println(result3);
        
        System.out.println("\\nTest Case 4: Register duplicate user");
        String result4 = server1.processRequest("REGISTER:alice:AnotherPass456!");
        System.out.println(result4);
        
        System.out.println("\\nTest Case 5: Invalid command");
        String result5 = server1.processRequest("INVALID:bob:password123");
        System.out.println(result5);
        
        Files.deleteIfExists(Paths.get("test_users1.dat"));
    }
}
