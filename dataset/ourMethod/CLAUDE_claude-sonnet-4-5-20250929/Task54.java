
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task54 {
    private static final int OTP_LENGTH = 6;
    private static final int TIME_STEP = 30;
    private static final int SECRET_KEY_LENGTH = 32;
    private static final int SALT_LENGTH = 16;
    private static final int PBKDF2_ITERATIONS = 210000;
    
    private final Map<String, UserData> userDatabase;
    private final SecureRandom secureRandom;
    
    private static class UserData {
        final byte[] passwordHash;
        final byte[] salt;
        final byte[] totpSecret;
        
        UserData(byte[] passwordHash, byte[] salt, byte[] totpSecret) {
            this.passwordHash = passwordHash != null ? passwordHash.clone() : null;
            this.salt = salt != null ? salt.clone() : null;
            this.totpSecret = totpSecret != null ? totpSecret.clone() : null;
        }
    }
    
    public Task54() {
        this.userDatabase = new HashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    public String registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || username.length() > 100) {
            return "ERROR: Invalid username";
        }
        if (password == null || password.length() < 8 || password.length() > 128) {
            return "ERROR: Password must be 8-128 characters";
        }
        if (!isStrongPassword(password)) {
            return "ERROR: Password must contain uppercase, lowercase, digit, and special character";
        }
        if (userDatabase.containsKey(username)) {
            return "ERROR: User already exists";
        }
        
        try {
            byte[] salt = new byte[SALT_LENGTH];
            secureRandom.nextBytes(salt);
            
            byte[] passwordHash = hashPassword(password, salt);
            
            byte[] totpSecret = new byte[SECRET_KEY_LENGTH];
            secureRandom.nextBytes(totpSecret);
            
            userDatabase.put(username, new UserData(passwordHash, salt, totpSecret));
            
            String secretBase64 = Base64.getEncoder().encodeToString(totpSecret);
            return "SUCCESS: User registered. TOTP Secret: " + secretBase64;
            
        } catch (Exception e) {
            return "ERROR: Registration failed";
        }
    }
    
    public String login(String username, String password, String otp) {
        if (username == null || username.trim().isEmpty()) {
            return "ERROR: Invalid username";
        }
        if (password == null || password.isEmpty()) {
            return "ERROR: Invalid password";
        }
        if (otp == null || !otp.matches("\\\\d{6}")) {
            return "ERROR: Invalid OTP format";
        }
        
        UserData user = userDatabase.get(username);
        if (user == null) {
            performDummyWork();
            return "ERROR: Authentication failed";
        }
        
        try {
            byte[] computedHash = hashPassword(password, user.salt);
            
            if (!constantTimeEquals(computedHash, user.passwordHash)) {
                return "ERROR: Authentication failed";
            }
            
            if (!verifyTOTP(user.totpSecret, otp)) {
                return "ERROR: Invalid OTP";
            }
            
            return "SUCCESS: Login successful";
            
        } catch (Exception e) {
            return "ERROR: Authentication failed";
        }
    }
    
    public String generateOTP(String username) {
        if (username == null || username.trim().isEmpty()) {
            return "ERROR: Invalid username";
        }
        
        UserData user = userDatabase.get(username);
        if (user == null) {
            return "ERROR: User not found";
        }
        
        try {
            long timeCounter = Instant.now().getEpochSecond() / TIME_STEP;
            String otp = generateTOTP(user.totpSecret, timeCounter);
            return otp;
        } catch (Exception e) {
            return "ERROR: OTP generation failed";
        }
    }
    
    private boolean isStrongPassword(String password) {
        if (password == null || password.length() < 8) {
            return false;
        }
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
    
    private byte[] hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeyException {
        SecretKeySpec keySpec = new SecretKeySpec(password.getBytes(StandardCharsets.UTF_8), "PBKDF2WithHmacSHA256");
        Mac mac = Mac.getInstance("HmacSHA256");
        mac.init(keySpec);
        
        byte[] result = new byte[32];
        byte[] u = new byte[32];
        byte[] saltWithCounter = new byte[salt.length + 4];
        System.arraycopy(salt, 0, saltWithCounter, 0, salt.length);
        saltWithCounter[salt.length + 3] = 1;
        
        mac.update(saltWithCounter);
        u = mac.doFinal();
        System.arraycopy(u, 0, result, 0, 32);
        
        for (int i = 1; i < PBKDF2_ITERATIONS; i++) {
            mac.update(u);
            u = mac.doFinal();
            for (int j = 0; j < 32; j++) {
                result[j] ^= u[j];
            }
        }
        
        return result;
    }
    
    private String generateTOTP(byte[] secret, long counter) throws NoSuchAlgorithmException, InvalidKeyException {
        ByteBuffer buffer = ByteBuffer.allocate(8);
        buffer.putLong(counter);
        byte[] timeBytes = buffer.array();
        
        Mac mac = Mac.getInstance("HmacSHA256");
        SecretKeySpec keySpec = new SecretKeySpec(secret, "HmacSHA256");
        mac.init(keySpec);
        byte[] hash = mac.doFinal(timeBytes);
        
        int offset = hash[hash.length - 1] & 0x0F;
        int binary = ((hash[offset] & 0x7F) << 24) |
                     ((hash[offset + 1] & 0xFF) << 16) |
                     ((hash[offset + 2] & 0xFF) << 8) |
                     (hash[offset + 3] & 0xFF);
        
        int otp = binary % 1000000;
        return String.format("%06d", otp);
    }
    
    private boolean verifyTOTP(byte[] secret, String otp) {
        try {
            long currentTime = Instant.now().getEpochSecond() / TIME_STEP;
            
            for (long i = -1; i <= 1; i++) {
                String generatedOtp = generateTOTP(secret, currentTime + i);
                if (constantTimeEquals(otp.getBytes(StandardCharsets.UTF_8), 
                                      generatedOtp.getBytes(StandardCharsets.UTF_8))) {
                    return true;
                }
            }
            return false;
        } catch (Exception e) {
            return false;
        }
    }
    
    private boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) {
            return false;
        }
        if (a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    private void performDummyWork() {
        try {
            byte[] dummy = new byte[SALT_LENGTH];
            secureRandom.nextBytes(dummy);
            hashPassword("dummy", dummy);
        } catch (Exception e) {
            // Intentionally ignored
        }
    }
    
    public static void main(String[] args) {
        Task54 auth = new Task54();
        
        System.out.println("Test Case 1: Register new user");
        String result1 = auth.registerUser("alice", "SecureP@ss123");
        System.out.println(result1);
        
        System.out.println("\\nTest Case 2: Generate OTP for alice");
        String otp = auth.generateOTP("alice");
        System.out.println("Generated OTP: " + otp);
        
        System.out.println("\\nTest Case 3: Login with correct credentials and OTP");
        String result3 = auth.login("alice", "SecureP@ss123", otp);
        System.out.println(result3);
        
        System.out.println("\\nTest Case 4: Login with incorrect OTP");
        String result4 = auth.login("alice", "SecureP@ss123", "000000");
        System.out.println(result4);
        
        System.out.println("\\nTest Case 5: Login with incorrect password");
        String result5 = auth.login("alice", "WrongPassword", otp);
        System.out.println(result5);
    }
}
