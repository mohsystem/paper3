
import javax.crypto.*;
import javax.crypto.spec.*;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.*;
import java.security.spec.KeySpec;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Task138 {
    private static final String MAGIC = "ENC1";
    private static final int VERSION = 1;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 12;
    private static final int TAG_LENGTH = 16;
    private static final int ITERATIONS = 210000;
    private static final String ALGORITHM = "AES/GCM/NoPadding";
    
    private final Map<String, User> users;
    private final SecureRandom secureRandom;
    
    public Task138() {
        this.users = new ConcurrentHashMap<>();
        try {
            this.secureRandom = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Unable to initialize SecureRandom", e);
        }
    }
    
    private static class User {
        private final String username;
        private final byte[] passwordHash;
        private final byte[] salt;
        private final Set<String> permissions;
        
        public User(String username, byte[] passwordHash, byte[] salt, Set<String> permissions) {
            if (username == null || username.trim().isEmpty()) {
                throw new IllegalArgumentException("Username cannot be null or empty");
            }
            if (passwordHash == null || passwordHash.length == 0) {
                throw new IllegalArgumentException("Password hash cannot be null or empty");
            }
            if (salt == null || salt.length != SALT_LENGTH) {
                throw new IllegalArgumentException("Salt must be " + SALT_LENGTH + " bytes");
            }
            this.username = username;
            this.passwordHash = Arrays.copyOf(passwordHash, passwordHash.length);
            this.salt = Arrays.copyOf(salt, salt.length);
            this.permissions = new HashSet<>(permissions != null ? permissions : Collections.emptySet());
        }
        
        public String getUsername() {
            return username;
        }
        
        public byte[] getPasswordHash() {
            return Arrays.copyOf(passwordHash, passwordHash.length);
        }
        
        public byte[] getSalt() {
            return Arrays.copyOf(salt, salt.length);
        }
        
        public Set<String> getPermissions() {
            return new HashSet<>(permissions);
        }
    }
    
    private byte[] hashPassword(String password, byte[] salt) {
        if (password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Password cannot be null or empty");
        }
        if (salt == null || salt.length != SALT_LENGTH) {
            throw new IllegalArgumentException("Salt must be " + SALT_LENGTH + " bytes");
        }
        
        try {
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, 256);
            SecretKey key = factory.generateSecret(spec);
            return key.getEncoded();
        } catch (Exception e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    private boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null || a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    public boolean createUser(String username, String password, Set<String> permissions) {
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        if (password == null || password.length() < 8) {
            return false;
        }
        if (!isPasswordComplex(password)) {
            return false;
        }
        
        String normalizedUsername = username.trim().toLowerCase();
        if (normalizedUsername.length() > 50) {
            return false;
        }
        
        if (users.containsKey(normalizedUsername)) {
            return false;
        }
        
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        byte[] passwordHash = hashPassword(password, salt);
        
        Set<String> validatedPermissions = new HashSet<>();
        if (permissions != null) {
            for (String perm : permissions) {
                if (perm != null && !perm.trim().isEmpty() && perm.length() <= 50) {
                    validatedPermissions.add(perm.trim());
                }
            }
        }
        
        User user = new User(normalizedUsername, passwordHash, salt, validatedPermissions);
        users.put(normalizedUsername, user);
        return true;
    }
    
    private boolean isPasswordComplex(String password) {
        if (password == null || password.length() < 8) {
            return false;
        }
        boolean hasUpper = false;
        boolean hasLower = false;
        boolean hasDigit = false;
        boolean hasSpecial = false;
        
        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
    
    public boolean authenticate(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        String normalizedUsername = username.trim().toLowerCase();
        User user = users.get(normalizedUsername);
        if (user == null) {
            return false;
        }
        
        byte[] providedHash = hashPassword(password, user.getSalt());
        return constantTimeEquals(providedHash, user.getPasswordHash());
    }
    
    public boolean grantPermission(String username, String permission) {
        if (username == null || permission == null || permission.trim().isEmpty()) {
            return false;
        }
        if (permission.length() > 50) {
            return false;
        }
        
        String normalizedUsername = username.trim().toLowerCase();
        User user = users.get(normalizedUsername);
        if (user == null) {
            return false;
        }
        
        user.permissions.add(permission.trim());
        return true;
    }
    
    public boolean revokePermission(String username, String permission) {
        if (username == null || permission == null) {
            return false;
        }
        
        String normalizedUsername = username.trim().toLowerCase();
        User user = users.get(normalizedUsername);
        if (user == null) {
            return false;
        }
        
        return user.permissions.remove(permission.trim());
    }
    
    public boolean hasPermission(String username, String permission) {
        if (username == null || permission == null) {
            return false;
        }
        
        String normalizedUsername = username.trim().toLowerCase();
        User user = users.get(normalizedUsername);
        if (user == null) {
            return false;
        }
        
        return user.permissions.contains(permission.trim());
    }
    
    public Set<String> getUserPermissions(String username) {
        if (username == null) {
            return Collections.emptySet();
        }
        
        String normalizedUsername = username.trim().toLowerCase();
        User user = users.get(normalizedUsername);
        if (user == null) {
            return Collections.emptySet();
        }
        
        return user.getPermissions();
    }
    
    public static void main(String[] args) {
        Task138 manager = new Task138();
        
        System.out.println("Test 1: Create user with valid password");
        boolean result1 = manager.createUser("alice", "Pass@123", new HashSet<>(Arrays.asList("read", "write")));
        System.out.println("User created: " + result1);
        
        System.out.println("\\nTest 2: Authenticate with correct credentials");
        boolean result2 = manager.authenticate("alice", "Pass@123");
        System.out.println("Authentication successful: " + result2);
        
        System.out.println("\\nTest 3: Authenticate with incorrect password");
        boolean result3 = manager.authenticate("alice", "WrongPass");
        System.out.println("Authentication successful: " + result3);
        
        System.out.println("\\nTest 4: Grant and check permission");
        manager.grantPermission("alice", "admin");
        boolean result4 = manager.hasPermission("alice", "admin");
        System.out.println("Has admin permission: " + result4);
        
        System.out.println("\\nTest 5: Revoke permission and check");
        manager.revokePermission("alice", "write");
        Set<String> permissions = manager.getUserPermissions("alice");
        System.out.println("Remaining permissions: " + permissions);
    }
}
