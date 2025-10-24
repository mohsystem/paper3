
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Task43 {
    private static final int GCM_IV_LENGTH = 12;
    private static final int GCM_TAG_LENGTH = 128;
    private static final int SESSION_TIMEOUT_SECONDS = 1800;
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    
    private final Map<String, SessionData> sessions = new ConcurrentHashMap<>();
    private final SecretKey encryptionKey;
    
    private static class SessionData {
        final String userId;
        final byte[] encryptedData;
        final long creationTime;
        final long expirationTime;
        
        SessionData(String userId, byte[] encryptedData, long creationTime, long expirationTime) {
            this.userId = userId;
            this.encryptedData = encryptedData;
            this.creationTime = creationTime;
            this.expirationTime = expirationTime;
        }
    }
    
    public Task43() throws Exception {
        KeyGenerator keyGen = KeyGenerator.getInstance("AES");
        keyGen.init(256, SECURE_RANDOM);
        this.encryptionKey = keyGen.generateKey();
    }
    
    public String createSession(String userId, String userData) throws Exception {
        if (userId == null || userId.trim().isEmpty() || userId.length() > 256) {
            throw new IllegalArgumentException("Invalid userId");
        }
        if (userData == null || userData.length() > 4096) {
            throw new IllegalArgumentException("Invalid userData");
        }
        
        String sessionId = generateSecureSessionId();
        byte[] encryptedData = encryptData(userData);
        long now = Instant.now().getEpochSecond();
        long expiration = now + SESSION_TIMEOUT_SECONDS;
        
        SessionData session = new SessionData(userId, encryptedData, now, expiration);
        sessions.put(sessionId, session);
        
        return sessionId;
    }
    
    public String getSessionData(String sessionId) throws Exception {
        if (sessionId == null || !isValidSessionId(sessionId)) {
            throw new IllegalArgumentException("Invalid sessionId");
        }
        
        SessionData session = sessions.get(sessionId);
        if (session == null) {
            throw new IllegalStateException("Session not found");
        }
        
        long now = Instant.now().getEpochSecond();
        if (now > session.expirationTime) {
            sessions.remove(sessionId);
            throw new IllegalStateException("Session expired");
        }
        
        return decryptData(session.encryptedData);
    }
    
    public boolean terminateSession(String sessionId) {
        if (sessionId == null || !isValidSessionId(sessionId)) {
            return false;
        }
        
        SessionData removed = sessions.remove(sessionId);
        if (removed != null) {
            Arrays.fill(removed.encryptedData, (byte) 0);
            return true;
        }
        return false;
    }
    
    public void cleanupExpiredSessions() {
        long now = Instant.now().getEpochSecond();
        sessions.entrySet().removeIf(entry -> {
            if (now > entry.getValue().expirationTime) {
                Arrays.fill(entry.getValue().encryptedData, (byte) 0);
                return true;
            }
            return false;
        });
    }
    
    private String generateSecureSessionId() throws Exception {
        byte[] randomBytes = new byte[32];
        SECURE_RANDOM.nextBytes(randomBytes);
        
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        byte[] hash = digest.digest(randomBytes);
        
        return Base64.getUrlEncoder().withoutPadding().encodeToString(hash);
    }
    
    private boolean isValidSessionId(String sessionId) {
        if (sessionId.length() != 43) {
            return false;
        }
        return sessionId.matches("^[A-Za-z0-9_-]+$");
    }
    
    private byte[] encryptData(String plaintext) throws Exception {
        byte[] iv = new byte[GCM_IV_LENGTH];
        SECURE_RANDOM.nextBytes(iv);
        
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, encryptionKey, spec);
        
        byte[] plaintextBytes = plaintext.getBytes(StandardCharsets.UTF_8);
        byte[] ciphertext = cipher.doFinal(plaintextBytes);
        
        ByteBuffer buffer = ByteBuffer.allocate(iv.length + ciphertext.length);
        buffer.put(iv);
        buffer.put(ciphertext);
        
        return buffer.array();
    }
    
    private String decryptData(byte[] encryptedData) throws Exception {
        if (encryptedData.length < GCM_IV_LENGTH + 16) {
            throw new IllegalArgumentException("Invalid encrypted data");
        }
        
        ByteBuffer buffer = ByteBuffer.wrap(encryptedData);
        byte[] iv = new byte[GCM_IV_LENGTH];
        buffer.get(iv);
        
        byte[] ciphertext = new byte[buffer.remaining()];
        buffer.get(ciphertext);
        
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.DECRYPT_MODE, encryptionKey, spec);
        
        byte[] plaintext = cipher.doFinal(ciphertext);
        return new String(plaintext, StandardCharsets.UTF_8);
    }
    
    public static void main(String[] args) {
        try {
            Task43 sessionManager = new Task43();
            
            // Test 1: Create and retrieve session
            System.out.println("Test 1: Create and retrieve session");
            String sessionId1 = sessionManager.createSession("user123", "userData1");
            System.out.println("Session created: " + sessionId1);
            String data1 = sessionManager.getSessionData(sessionId1);
            System.out.println("Retrieved data: " + data1);
            System.out.println("Test 1 passed: " + data1.equals("userData1"));
            
            // Test 2: Multiple sessions
            System.out.println("\\nTest 2: Multiple sessions");
            String sessionId2 = sessionManager.createSession("user456", "userData2");
            String sessionId3 = sessionManager.createSession("user789", "userData3");
            System.out.println("Sessions are unique: " + 
                (!sessionId1.equals(sessionId2) && !sessionId2.equals(sessionId3)));
            
            // Test 3: Terminate session
            System.out.println("\\nTest 3: Terminate session");
            boolean terminated = sessionManager.terminateSession(sessionId1);
            System.out.println("Session terminated: " + terminated);
            try {
                sessionManager.getSessionData(sessionId1);
                System.out.println("Test 3 failed: Session should not exist");
            } catch (IllegalStateException e) {
                System.out.println("Test 3 passed: " + e.getMessage());
            }
            
            // Test 4: Invalid session ID
            System.out.println("\\nTest 4: Invalid session ID");
            try {
                sessionManager.getSessionData("invalid-session-id");
                System.out.println("Test 4 failed: Should reject invalid session ID");
            } catch (IllegalArgumentException e) {
                System.out.println("Test 4 passed: " + e.getMessage());
            }
            
            // Test 5: Cleanup expired sessions
            System.out.println("\\nTest 5: Active sessions cleanup");
            sessionManager.cleanupExpiredSessions();
            String data2 = sessionManager.getSessionData(sessionId2);
            System.out.println("Test 5 passed: Active session still accessible: " + 
                data2.equals("userData2"));
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
