
import javax.crypto.*;
import javax.crypto.spec.*;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Task53 {
    private static final int SESSION_TIMEOUT_SECONDS = 3600;
    private static final int TOKEN_SIZE = 32;
    private static final int SALT_SIZE = 16;
    private static final int IV_SIZE = 12;
    private static final int TAG_SIZE = 128;
    private static final int PBKDF2_ITERATIONS = 210000;
    
    private final ConcurrentHashMap<String, Session> sessions;
    private final SecureRandom secureRandom;
    
    public Task53() {
        this.sessions = new ConcurrentHashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    private static class Session {
        final String userId;
        final byte[] encryptedData;
        final long expiryTime;
        final byte[] salt;
        final byte[] iv;
        
        Session(String userId, byte[] encryptedData, long expiryTime, byte[] salt, byte[] iv) {
            this.userId = userId;
            this.encryptedData = encryptedData.clone();
            this.expiryTime = expiryTime;
            this.salt = salt.clone();
            this.iv = iv.clone();
        }
    }
    
    public String createSession(String userId, String sessionData, String masterKey) {
        if (userId == null || userId.isEmpty() || userId.length() > 256) {
            throw new IllegalArgumentException("Invalid userId");
        }
        if (sessionData == null || sessionData.isEmpty() || sessionData.length() > 10000) {
            throw new IllegalArgumentException("Invalid sessionData");
        }
        if (masterKey == null || masterKey.length() < 16) {
            throw new IllegalArgumentException("Invalid masterKey");
        }
        
        try {
            byte[] sessionToken = new byte[TOKEN_SIZE];
            secureRandom.nextBytes(sessionToken);
            String tokenHex = bytesToHex(sessionToken);
            
            byte[] salt = new byte[SALT_SIZE];
            secureRandom.nextBytes(salt);
            
            byte[] iv = new byte[IV_SIZE];
            secureRandom.nextBytes(iv);
            
            SecretKey key = deriveKey(masterKey, salt);
            byte[] encryptedData = encryptAES(sessionData.getBytes(StandardCharsets.UTF_8), key, iv);
            
            long expiryTime = Instant.now().getEpochSecond() + SESSION_TIMEOUT_SECONDS;
            
            Session session = new Session(userId, encryptedData, expiryTime, salt, iv);
            sessions.put(tokenHex, session);
            
            return tokenHex;
        } catch (Exception e) {
            throw new RuntimeException("Session creation failed");
        }
    }
    
    public String getSessionData(String token, String masterKey) {
        if (token == null || token.length() != TOKEN_SIZE * 2) {
            return null;
        }
        if (masterKey == null || masterKey.length() < 16) {
            return null;
        }
        
        Session session = sessions.get(token);
        if (session == null) {
            return null;
        }
        
        if (Instant.now().getEpochSecond() > session.expiryTime) {
            sessions.remove(token);
            return null;
        }
        
        try {
            SecretKey key = deriveKey(masterKey, session.salt);
            byte[] decryptedData = decryptAES(session.encryptedData, key, session.iv);
            return new String(decryptedData, StandardCharsets.UTF_8);
        } catch (Exception e) {
            return null;
        }
    }
    
    public boolean invalidateSession(String token) {
        if (token == null || token.length() != TOKEN_SIZE * 2) {
            return false;
        }
        return sessions.remove(token) != null;
    }
    
    public boolean validateSession(String token) {
        if (token == null || token.length() != TOKEN_SIZE * 2) {
            return false;
        }
        
        Session session = sessions.get(token);
        if (session == null) {
            return false;
        }
        
        if (Instant.now().getEpochSecond() > session.expiryTime) {
            sessions.remove(token);
            return false;
        }
        
        return true;
    }
    
    private SecretKey deriveKey(String passphrase, byte[] salt) throws Exception {
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        KeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, PBKDF2_ITERATIONS, 256);
        SecretKey tmp = factory.generateSecret(spec);
        return new SecretKeySpec(tmp.getEncoded(), "AES");
    }
    
    private byte[] encryptAES(byte[] plaintext, SecretKey key, byte[] iv) throws Exception {
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_SIZE, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, spec);
        return cipher.doFinal(plaintext);
    }
    
    private byte[] decryptAES(byte[] ciphertext, SecretKey key, byte[] iv) throws Exception {
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_SIZE, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, spec);
        return cipher.doFinal(ciphertext);
    }
    
    private String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }
    
    public static void main(String[] args) {
        Task53 manager = new Task53();
        String masterKey = "securemaster_passphrase_key_2024";
        
        System.out.println("Test 1: Create and validate session");
        String token1 = manager.createSession("user123", "sessionData1", masterKey);
        System.out.println("Created token: " + token1.substring(0, 16) + "...");
        System.out.println("Valid: " + manager.validateSession(token1));
        
        System.out.println("\\nTest 2: Retrieve session data");
        String data = manager.getSessionData(token1, masterKey);
        System.out.println("Retrieved data: " + data);
        
        System.out.println("\\nTest 3: Invalidate session");
        boolean invalidated = manager.invalidateSession(token1);
        System.out.println("Invalidated: " + invalidated);
        System.out.println("Still valid: " + manager.validateSession(token1));
        
        System.out.println("\\nTest 4: Multiple sessions");
        String token2 = manager.createSession("user456", "sessionData2", masterKey);
        String token3 = manager.createSession("user789", "sessionData3", masterKey);
        System.out.println("Token2 valid: " + manager.validateSession(token2));
        System.out.println("Token3 valid: " + manager.validateSession(token3));
        
        System.out.println("\\nTest 5: Invalid token handling");
        boolean validInvalid = manager.validateSession("invalidtoken123");
        System.out.println("Invalid token valid: " + validInvalid);
    }
}
