
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.regex.Pattern;

public class Task73 {
    private static final int MIN_PASSWORD_LENGTH = 12;
    private static final int MAX_PASSWORD_LENGTH = 128;
    
    // Simulated stored hash (in production, retrieve from secure storage)
    // This represents SHA-256 hash of "AdminPass123!@#$"
    private static final byte[] STORED_HASH = hexStringToByteArray(
        "8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918"
    );
    
    public static boolean isValidAdminPassword(String password) {
        if (password == null) {
            return false;
        }
        
        // Check length constraints
        if (password.length() < MIN_PASSWORD_LENGTH || password.length() > MAX_PASSWORD_LENGTH) {
            return false;
        }
        
        // Check complexity requirements
        if (!hasUpperCase(password)) {
            return false;
        }
        
        if (!hasLowerCase(password)) {
            return false;
        }
        
        if (!hasDigit(password)) {
            return false;
        }
        
        if (!hasSpecialCharacter(password)) {
            return false;
        }
        
        // Verify against stored hash using constant-time comparison
        byte[] inputHash = hashPassword(password);
        if (inputHash == null) {
            return false;
        }
        
        return constantTimeEquals(inputHash, STORED_HASH);
    }
    
    private static boolean hasUpperCase(String str) {
        for (int i = 0; i < str.length(); i++) {
            if (Character.isUpperCase(str.charAt(i))) {
                return true;
            }
        }
        return false;
    }
    
    private static boolean hasLowerCase(String str) {
        for (int i = 0; i < str.length(); i++) {
            if (Character.isLowerCase(str.charAt(i))) {
                return true;
            }
        }
        return false;
    }
    
    private static boolean hasDigit(String str) {
        for (int i = 0; i < str.length(); i++) {
            if (Character.isDigit(str.charAt(i))) {
                return true;
            }
        }
        return false;
    }
    
    private static boolean hasSpecialCharacter(String str) {
        Pattern specialCharPattern = Pattern.compile("[!@#$%^&*()_+\\\\-=\\\\[\\\\]{};':\\"\\\\\\\\|,.<>/?]");
        return specialCharPattern.matcher(str).find();
    }
    
    private static byte[] hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            return digest.digest(password.getBytes(StandardCharsets.UTF_8));
        } catch (NoSuchAlgorithmException e) {
            return null;
        }
    }
    
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }
        
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    private static byte[] hexStringToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                                 + Character.digit(s.charAt(i+1), 16));
        }
        return data;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid password matching stored hash
        String password1 = "AdminPass123!@#$";
        System.out.println("Test 1 - Valid admin password: " + isValidAdminPassword(password1));
        
        // Test case 2: Password too short
        String password2 = "Short1!";
        System.out.println("Test 2 - Too short password: " + isValidAdminPassword(password2));
        
        // Test case 3: Missing special character
        String password3 = "AdminPassword123";
        System.out.println("Test 3 - No special character: " + isValidAdminPassword(password3));
        
        // Test case 4: Missing uppercase
        String password4 = "adminpass123!@#";
        System.out.println("Test 4 - No uppercase: " + isValidAdminPassword(password4));
        
        // Test case 5: Null password
        String password5 = null;
        System.out.println("Test 5 - Null password: " + isValidAdminPassword(password5));
    }
}
