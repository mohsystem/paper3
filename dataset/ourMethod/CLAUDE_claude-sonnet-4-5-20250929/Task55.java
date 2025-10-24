
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.nio.charset.StandardCharsets;

public class Task55 {
    private static final int MAX_EMAIL_LENGTH = 320; // RFC 5321
    private static final int MAX_LOCAL_PART = 64;
    private static final int MAX_DOMAIN_PART = 255;
    
    // Simplified email pattern that avoids catastrophic backtracking
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$"
    );
    
    public static boolean isValidEmail(String email) {
        // Validate input is not null
        if (email == null) {
            return false;
        }
        
        // Validate length constraints
        if (email.length() == 0 || email.length() > MAX_EMAIL_LENGTH) {
            return false;
        }
        
        // Validate UTF-8 encoding
        byte[] bytes = email.getBytes(StandardCharsets.UTF_8);
        String decoded = new String(bytes, StandardCharsets.UTF_8);
        if (!email.equals(decoded)) {
            return false;
        }
        
        // Check for whitespace
        if (email.trim().length() != email.length()) {
            return false;
        }
        
        // Must contain exactly one @ symbol
        int atIndex = email.indexOf('@');
        int lastAtIndex = email.lastIndexOf('@');
        if (atIndex == -1 || atIndex != lastAtIndex) {
            return false;
        }
        
        // Split and validate parts
        String localPart = email.substring(0, atIndex);
        String domainPart = email.substring(atIndex + 1);
        
        // Validate local part
        if (localPart.length() == 0 || localPart.length() > MAX_LOCAL_PART) {
            return false;
        }
        if (localPart.startsWith(".") || localPart.endsWith(".")) {
            return false;
        }
        if (localPart.contains("..")) {
            return false;
        }
        
        // Validate domain part
        if (domainPart.length() == 0 || domainPart.length() > MAX_DOMAIN_PART) {
            return false;
        }
        if (domainPart.startsWith(".") || domainPart.endsWith(".") || 
            domainPart.startsWith("-") || domainPart.endsWith("-")) {
            return false;
        }
        if (domainPart.contains("..") || domainPart.contains("--")) {
            return false;
        }
        
        // Must have at least one dot in domain
        if (!domainPart.contains(".")) {
            return false;
        }
        
        // Apply regex pattern
        Matcher matcher = EMAIL_PATTERN.matcher(email);
        return matcher.matches();
    }
    
    public static void main(String[] args) {
        String[] testCases = {
            "user@example.com",
            "test.user+tag@sub.example.co.uk",
            "invalid.email@",
            "@invalid.com",
            "no-at-sign.com"
        };
        
        for (String testCase : testCases) {
            boolean result = isValidEmail(testCase);
            System.out.println("Email: '" + testCase + "' - Valid: " + result);
        }
    }
}
