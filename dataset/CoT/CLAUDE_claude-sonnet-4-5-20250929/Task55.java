
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task55 {
    private static final int MAX_EMAIL_LENGTH = 320; // RFC 5321
    private static final int MAX_LOCAL_LENGTH = 64;
    private static final int MAX_DOMAIN_LENGTH = 255;
    
    public static boolean isValidEmail(String email) {
        // Input validation - prevent null and length attacks
        if (email == null || email.trim().isEmpty()) {
            return false;
        }
        
        email = email.trim();
        
        // Check maximum length to prevent DoS
        if (email.length() > MAX_EMAIL_LENGTH) {
            return false;
        }
        
        // Check for exactly one @ symbol
        int atCount = 0;
        int atPosition = -1;
        for (int i = 0; i < email.length(); i++) {
            if (email.charAt(i) == '@') {
                atCount++;
                atPosition = i;
            }
        }
        
        if (atCount != 1 || atPosition == 0 || atPosition == email.length() - 1) {
            return false;
        }
        
        // Split into local and domain parts
        String localPart = email.substring(0, atPosition);
        String domainPart = email.substring(atPosition + 1);
        
        // Validate lengths
        if (localPart.length() > MAX_LOCAL_LENGTH || domainPart.length() > MAX_DOMAIN_LENGTH) {
            return false;
        }
        
        // Validate local part - use atomic pattern to prevent ReDoS
        String localPattern = "^[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+$";
        if (!Pattern.matches(localPattern, localPart)) {
            return false;
        }
        
        // Check for consecutive dots or leading/trailing dots
        if (localPart.startsWith(".") || localPart.endsWith(".") || localPart.contains("..")) {
            return false;
        }
        
        // Validate domain part - use atomic pattern to prevent ReDoS
        String domainPattern = "^[a-zA-Z0-9]([a-zA-Z0-9-]*[a-zA-Z0-9])?(\\\\.[a-zA-Z0-9]([a-zA-Z0-9-]*[a-zA-Z0-9])?)*$";
        if (!Pattern.matches(domainPattern, domainPart)) {
            return false;
        }
        
        // Check for valid TLD (at least one dot in domain)
        if (!domainPart.contains(".")) {
            return false;
        }
        
        // Validate TLD length (2-63 characters)
        String[] domainParts = domainPart.split("\\\\.");
        String tld = domainParts[domainParts.length - 1];
        if (tld.length() < 2 || tld.length() > 63) {
            return false;
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testEmails = {
            "user@example.com",           // Valid
            "test.email@domain.co.uk",    // Valid
            "invalid.email@",             // Invalid - no domain
            "@invalid.com",               // Invalid - no local part
            "no-at-sign.com",             // Invalid - no @ symbol
            "user@domain",                // Invalid - no TLD
            "user..name@domain.com",      // Invalid - consecutive dots
            ".user@domain.com",           // Invalid - starts with dot
            "user@.domain.com",           // Invalid - domain starts with dot
            "a".repeat(65) + "@test.com"  // Invalid - local part too long
        };
        
        System.out.println("Email Validation Test Results:");
        System.out.println("=".repeat(50));
        for (String email : testEmails) {
            String displayEmail = email.length() > 50 ? email.substring(0, 47) + "..." : email;
            System.out.printf("%-50s : %s%n", displayEmail, isValidEmail(email) ? "VALID" : "INVALID");
        }
    }
}
