
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class Task132 {
    private static final String MAGIC = "ENC1";
    private static final int VERSION = 1;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 12;
    private static final int TAG_LENGTH = 128;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final String LOG_DIR = "security_logs";
    
    private final ConcurrentLinkedQueue<String> logQueue;
    private final ReentrantReadWriteLock lock;
    private final SecureRandom secureRandom;
    private final Path baseLogDir;
    
    public Task132(String logDirectory) throws IOException {
        if (logDirectory == null || logDirectory.trim().isEmpty()) {
            throw new IllegalArgumentException("Log directory cannot be null or empty");
        }
        
        this.logQueue = new ConcurrentLinkedQueue<>();
        this.lock = new ReentrantReadWriteLock();
        this.secureRandom = new SecureRandom();
        this.baseLogDir = Paths.get(logDirectory).normalize().toAbsolutePath();
        
        if (!Files.exists(baseLogDir)) {
            Files.createDirectories(baseLogDir);
        }
        
        if (!Files.isDirectory(baseLogDir)) {
            throw new IllegalArgumentException("Log path must be a directory");
        }
    }
    
    public void logSecurityEvent(String eventType, String eventDetails) {
        if (eventType == null || eventType.trim().isEmpty()) {
            throw new IllegalArgumentException("Event type cannot be null or empty");
        }
        if (eventDetails == null) {
            eventDetails = "";
        }
        
        String timestamp = Instant.now().toString();
        String logEntry = String.format("[%s] %s: %s", timestamp, sanitizeInput(eventType), sanitizeInput(eventDetails));
        
        lock.writeLock().lock();
        try {
            logQueue.offer(logEntry);
        } finally {
            lock.writeLock().unlock();
        }
    }
    
    private String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        return input.replaceAll("[\\\\r\\\\n]", " ").trim();
    }
    
    public void flushLogsToFile(String filename, String passphrase) throws Exception {
        if (filename == null || filename.trim().isEmpty()) {
            throw new IllegalArgumentException("Filename cannot be null or empty");
        }
        if (passphrase == null || passphrase.length() < 12) {
            throw new IllegalArgumentException("Passphrase must be at least 12 characters");
        }
        
        Path targetPath = baseLogDir.resolve(filename).normalize();
        if (!targetPath.startsWith(baseLogDir)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        if (Files.isSymbolicLink(targetPath)) {
            throw new SecurityException("Symlinks are not allowed");
        }
        
        List<String> logs = new ArrayList<>();
        lock.writeLock().lock();
        try {
            while (!logQueue.isEmpty()) {
                String entry = logQueue.poll();
                if (entry != null) {
                    logs.add(entry);
                }
            }
        } finally {
            lock.writeLock().unlock();
        }
        
        if (logs.isEmpty()) {
            return;
        }
        
        String plaintext = String.join("\\n", logs);
        byte[] encryptedData = encryptData(plaintext.getBytes(StandardCharsets.UTF_8), passphrase);
        
        Path tempFile = Files.createTempFile(baseLogDir, ".tmp", ".enc");
        try {
            Files.write(tempFile, encryptedData, StandardOpenOption.WRITE);
            Files.move(tempFile, targetPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
        } catch (Exception e) {
            try {
                Files.deleteIfExists(tempFile);
            } catch (IOException ignored) {
            }
            throw e;
        }
    }
    
    private byte[] encryptData(byte[] plaintext, String passphrase) throws Exception {
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        
        byte[] iv = new byte[IV_LENGTH];
        secureRandom.nextBytes(iv);
        
        SecretKey key = deriveKey(passphrase, salt);
        
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, spec);
        
        byte[] ciphertext = cipher.doFinal(plaintext);
        
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        output.write(MAGIC.getBytes(StandardCharsets.UTF_8));
        output.write(VERSION);
        output.write(salt);
        output.write(iv);
        output.write(ciphertext);
        
        return output.toByteArray();
    }
    
    public String readEncryptedLogs(String filename, String passphrase) throws Exception {
        if (filename == null || filename.trim().isEmpty()) {
            throw new IllegalArgumentException("Filename cannot be null or empty");
        }
        if (passphrase == null || passphrase.length() < 12) {
            throw new IllegalArgumentException("Passphrase must be at least 12 characters");
        }
        
        Path targetPath = baseLogDir.resolve(filename).normalize();
        if (!targetPath.startsWith(baseLogDir)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        if (!Files.exists(targetPath) || !Files.isRegularFile(targetPath)) {
            throw new FileNotFoundException("File not found or is not a regular file");
        }
        
        byte[] encryptedData = Files.readAllBytes(targetPath);
        byte[] plaintext = decryptData(encryptedData, passphrase);
        
        return new String(plaintext, StandardCharsets.UTF_8);
    }
    
    private byte[] decryptData(byte[] encryptedData, String passphrase) throws Exception {
        if (encryptedData.length < MAGIC.length() + 1 + SALT_LENGTH + IV_LENGTH + TAG_LENGTH / 8) {
            throw new IllegalArgumentException("Invalid encrypted data format");
        }
        
        ByteArrayInputStream input = new ByteArrayInputStream(encryptedData);
        
        byte[] magic = new byte[MAGIC.length()];
        input.read(magic);
        if (!Arrays.equals(magic, MAGIC.getBytes(StandardCharsets.UTF_8))) {
            throw new IllegalArgumentException("Invalid file format");
        }
        
        int version = input.read();
        if (version != VERSION) {
            throw new IllegalArgumentException("Unsupported version");
        }
        
        byte[] salt = new byte[SALT_LENGTH];
        input.read(salt);
        
        byte[] iv = new byte[IV_LENGTH];
        input.read(iv);
        
        byte[] ciphertext = new byte[input.available()];
        input.read(ciphertext);
        
        SecretKey key = deriveKey(passphrase, salt);
        
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_LENGTH, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, spec);
        
        return cipher.doFinal(ciphertext);
    }
    
    private SecretKey deriveKey(String passphrase, byte[] salt) throws Exception {
        KeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, PBKDF2_ITERATIONS, 256);
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] keyBytes = factory.generateSecret(spec).getEncoded();
        return new SecretKeySpec(keyBytes, "AES");
    }
    
    public static void main(String[] args) {
        try {
            Task132 securityLogger = new Task132(LOG_DIR);
            String testPassphrase = "SecurePassword123!@#";
            
            System.out.println("Test 1: Basic security event logging");
            securityLogger.logSecurityEvent("LOGIN_SUCCESS", "User admin logged in from 192.168.1.100");
            securityLogger.logSecurityEvent("LOGIN_FAILURE", "Failed login attempt for user guest");
            securityLogger.flushLogsToFile("test1.enc", testPassphrase);
            String logs1 = securityLogger.readEncryptedLogs("test1.enc", testPassphrase);
            System.out.println("Retrieved logs:\\n" + logs1);
            System.out.println();
            
            System.out.println("Test 2: Multiple security events");
            securityLogger.logSecurityEvent("FILE_ACCESS", "User accessed sensitive file: /etc/passwd");
            securityLogger.logSecurityEvent("PERMISSION_CHANGE", "Permissions changed on /var/log");
            securityLogger.logSecurityEvent("FIREWALL_BLOCK", "Blocked connection from 10.0.0.50");
            securityLogger.flushLogsToFile("test2.enc", testPassphrase);
            String logs2 = securityLogger.readEncryptedLogs("test2.enc", testPassphrase);
            System.out.println("Retrieved logs:\\n" + logs2);
            System.out.println();
            
            System.out.println("Test 3: Special characters in event details");
            securityLogger.logSecurityEvent("SQL_INJECTION_ATTEMPT", "Detected SQL injection: ' OR '1'='1");
            securityLogger.logSecurityEvent("XSS_ATTEMPT", "Detected XSS: <script>alert('test')</script>");
            securityLogger.flushLogsToFile("test3.enc", testPassphrase);
            String logs3 = securityLogger.readEncryptedLogs("test3.enc", testPassphrase);
            System.out.println("Retrieved logs:\\n" + logs3);
            System.out.println();
            
            System.out.println("Test 4: Input validation");
            try {
                securityLogger.logSecurityEvent("", "Empty event type should fail");
            } catch (IllegalArgumentException e) {
                System.out.println("Correctly rejected empty event type: " + e.getMessage());
            }
            System.out.println();
            
            System.out.println("Test 5: Path traversal protection");
            try {
                securityLogger.flushLogsToFile("../../../etc/passwd", testPassphrase);
            } catch (SecurityException e) {
                System.out.println("Correctly blocked path traversal: " + e.getMessage());
            }
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
