
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task89 {
    private static final int SALT_LENGTH = 16;
    
    public static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null || a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    public static byte[] hashPassword(String password, byte[] salt) {
        if (password == null || salt == null || salt.length != SALT_LENGTH) {
            throw new IllegalArgumentException("Invalid password or salt");
        }
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            digest.update(salt);
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            for (int i = 0; i < 10000; i++) {
                digest.reset();
                hash = digest.digest(hash);
            }
            return hash;
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 not available", e);
        }
    }
    
    public static boolean verifyPassword(String storedPasswordFilePath, String enteredPassword) {
        if (storedPasswordFilePath == null || enteredPassword == null) {
            return false;
        }
        
        Path filePath = Paths.get(storedPasswordFilePath).normalize();
        Path baseDir = Paths.get(System.getProperty("user.dir")).normalize();
        
        if (!filePath.startsWith(baseDir)) {
            return false;
        }
        
        if (!Files.exists(filePath) || !Files.isRegularFile(filePath)) {
            return false;
        }
        
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath.toFile(), StandardCharsets.UTF_8))) {
            String saltHex = reader.readLine();
            String hashHex = reader.readLine();
            
            if (saltHex == null || hashHex == null) {
                return false;
            }
            
            byte[] salt = hexToBytes(saltHex);
            byte[] storedHash = hexToBytes(hashHex);
            
            if (salt.length != SALT_LENGTH) {
                return false;
            }
            
            byte[] enteredHash = hashPassword(enteredPassword, salt);
            
            return constantTimeEquals(storedHash, enteredHash);
        } catch (IOException | IllegalArgumentException e) {
            return false;
        }
    }
    
    public static void storePassword(String filePath, String password) throws IOException {
        if (filePath == null || password == null) {
            throw new IllegalArgumentException("Invalid file path or password");
        }
        
        Path path = Paths.get(filePath).normalize();
        Path baseDir = Paths.get(System.getProperty("user.dir")).normalize();
        
        if (!path.startsWith(baseDir)) {
            throw new IllegalArgumentException("Invalid file path");
        }
        
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        
        byte[] hash = hashPassword(password, salt);
        
        String content = bytesToHex(salt) + "\\n" + bytesToHex(hash);
        Files.writeString(path, content, StandardCharsets.UTF_8);
    }
    
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
    
    public static void main(String[] args) {
        try {
            storePassword("test_password1.txt", "SecurePass123!");
            System.out.println("Test 1: " + verifyPassword("test_password1.txt", "SecurePass123!"));
            
            storePassword("test_password2.txt", "MyP@ssw0rd");
            System.out.println("Test 2: " + verifyPassword("test_password2.txt", "MyP@ssw0rd"));
            
            System.out.println("Test 3: " + verifyPassword("test_password2.txt", "WrongPassword"));
            
            storePassword("test_password3.txt", "Admin2024#");
            System.out.println("Test 4: " + verifyPassword("test_password3.txt", "Admin2024#"));
            
            System.out.println("Test 5: " + verifyPassword("test_password1.txt", "WrongPass"));
            
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
