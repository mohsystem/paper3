
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse137 {
    private static final Map<String, User> users = new HashMap<>();
    private static final Map<String, Session> sessions = new HashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    
    static class User {
        String username;
        String passwordHash;
        boolean isModerator;
        
        User(String username, String passwordHash, boolean isModerator) {
            this.username = username;
            this.passwordHash = passwordHash;
            this.isModerator = isModerator;
        }
    }
    
    static class Session {
        String sessionId;
        String username;
        long expiryTime;
        
        Session(String sessionId, String username, long expiryTime) {
            this.sessionId = sessionId;
            this.username = username;
            this.expiryTime = expiryTime;
        }
        
        boolean isValid() {
            return System.currentTimeMillis() < expiryTime;
        }
    }
    
    public static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }
    
    public static boolean auth_mod(String username, String password, String sessionId) {
        if (username == null || password == null || sessionId == null) {
            return false;
        }
        
        Session session = sessions.get(sessionId);
        if (session == null || !session.isValid() || !session.username.equals(username)) {
            return false;
        }
        
        User user = users.get(username);
        if (user == null) {
            return false;
        }
        
        String hashedPassword = hashPassword(password);
        if (!user.passwordHash.equals(hashedPassword)) {
            return false;
        }
        
        return user.isModerator;
    }
    
    public static boolean check_mod(String username, String password, String sessionId) {
        return auth_mod(username, password, sessionId);
    }
    
    public static void main(String[] args) {
        // Setup test data
        users.put("admin", new User("admin", hashPassword("admin123"), true));
        users.put("user1", new User("user1", hashPassword("pass123"), false));
        users.put("mod1", new User("mod1", hashPassword("modpass"), true));
        
        String validSession = "session123";
        sessions.put(validSession, new Session(validSession, "admin", System.currentTimeMillis() + 3600000));
        
        String validSession2 = "session456";
        sessions.put(validSession2, new Session(validSession2, "user1", System.currentTimeMillis() + 3600000));
        
        String validSession3 = "session789";
        sessions.put(validSession3, new Session(validSession3, "mod1", System.currentTimeMillis() + 3600000));
        
        // Test cases
        System.out.println("Test 1 (Valid moderator): " + check_mod("admin", "admin123", validSession));
        System.out.println("Test 2 (Valid non-moderator): " + check_mod("user1", "pass123", validSession2));
        System.out.println("Test 3 (Invalid password): " + check_mod("admin", "wrongpass", validSession));
        System.out.println("Test 4 (Invalid session): " + check_mod("admin", "admin123", "invalidsession"));
        System.out.println("Test 5 (Another valid moderator): " + check_mod("mod1", "modpass", validSession3));
    }
}
