
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

public class Lse30 {
    private static final Pattern SAFE_REDIRECT_PATTERN = Pattern.compile("^[a-zA-Z0-9-]+\\\\.");
    private static final String ROOT_REDIRECT = "/";
    
    public static String getRedirectUrl(String target) {
        if (target == null || target.isEmpty()) {
            return ROOT_REDIRECT;
        }
        
        try {
            // Decode URL to prevent encoding bypasses
            String decodedTarget = URLDecoder.decode(target, StandardCharsets.UTF_8.name());
            
            // Additional security checks
            // 1. Prevent open redirect by ensuring it's a relative path\n            if (decodedTarget.startsWith("http://") || decodedTarget.startsWith("https://") || \n                decodedTarget.startsWith("//") || decodedTarget.contains("@")) {\n                return ROOT_REDIRECT;\n            }\n            \n            // 2. Check against the regular expression\n            Matcher matcher = SAFE_REDIRECT_PATTERN.matcher(decodedTarget);\n            if (matcher.find() && matcher.start() == 0) {\n                // Ensure the target doesn't contain path traversal attempts
                if (decodedTarget.contains("..") || decodedTarget.contains("./")) {
                    return ROOT_REDIRECT;
                }
                return decodedTarget;
            }
            
        } catch (Exception e) {
            // If any error occurs during decoding, redirect to root
            return ROOT_REDIRECT;
        }
        
        return ROOT_REDIRECT;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + getRedirectUrl("page1.html"));
        System.out.println("Test 2: " + getRedirectUrl("my-page.html"));
        System.out.println("Test 3: " + getRedirectUrl("http://evil.com"));
        System.out.println("Test 4: " + getRedirectUrl("../etc/passwd"));
        System.out.println("Test 5: " + getRedirectUrl("invalid"));
    }
}
