
import java.io.IOException;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse30 {
    // Secure pattern: matches alphanumeric/hyphen characters followed by a literal period
    // Pattern is anchored to prevent partial matches and injection attacks
    private static final Pattern SAFE_TARGET_PATTERN = Pattern.compile("^[a-zA-Z0-9-]+\\\\.$");
    
    // Maximum length to prevent ReDoS and excessive resource consumption
    private static final int MAX_TARGET_LENGTH = 100;
    
    /**
     * Validates and processes a redirect target based on secure pattern matching.
     * 
     * @param targetQueryParam The target query parameter value (untrusted input)
     * @return The validated redirect URL or root path "/"
     */
    public static String processRedirect(String targetQueryParam) {
        // Rule#3: Input validation - check for null or empty input
        if (targetQueryParam == null || targetQueryParam.isEmpty()) {
            return "/";  // Fail closed - redirect to root
        }
        
        try {
            // Decode URL-encoded input to prevent bypass via encoding
            String target = URLDecoder.decode(targetQueryParam, StandardCharsets.UTF_8);
            
            // Rule#3: Validate length to prevent resource exhaustion
            if (target.length() > MAX_TARGET_LENGTH) {
                return "/";  // Fail closed
            }
            
            // Rule#3: Additional validation - reject directory traversal attempts
            // Reject paths containing .. or absolute paths
            if (target.contains("..") || target.startsWith("/") || target.startsWith("\\\\")) {\n                return "/";  // Fail closed\n            }\n            \n            // Rule#3: Reject URL schemes to prevent open redirect to external sites\n            if (target.contains("://") || target.startsWith("//")) {\n                return "/";  // Fail closed - no external redirects\n            }\n            \n            // Apply the regex pattern match\n            Matcher matcher = SAFE_TARGET_PATTERN.matcher(target);\n            \n            if (matcher.matches()) {\n                // Even if pattern matches, construct safe relative path\n                // Prepend with ./ to ensure relative redirect within site\n                return "./" + target;\n            } else {\n                // Pattern does not match - redirect to root\n                return "/";\n            }\n            \n        } catch (Exception e) {\n            // Rule#1: Fail closed on any error, log generically\n            // Do not expose internal details to caller\n            System.err.println("Redirect validation error occurred");\n            return "/";\n        }\n    }\n    \n    public static void main(String[] args) {\n        // Test case 1: Valid input matching pattern\n        String test1 = "test123.";\n        System.out.println("Test 1 input: '" + test1 + "' -> " + processRedirect(test1));\n        \n        // Test case 2: Valid input with hyphens\n        String test2 = "my-page.";\n        System.out.println("Test 2 input: '" + test2 + "' -> " + processRedirect(test2));\n        \n        // Test case 3: Invalid - missing period\n        String test3 = "test123";\n        System.out.println("Test 3 input: '" + test3 + "' -> " + processRedirect(test3));\n        \n        // Test case 4: Invalid - directory traversal attempt\n        String test4 = "../etc.";\n        System.out.println("Test 4 input: '" + test4 + "' -> " + processRedirect(test4));\n        \n        // Test case 5: Invalid - open redirect attempt\n        String test5 = "http://evil.com.";\n        System.out.println("Test 5 input: '" + test5 + "' -> " + processRedirect(test5));
    }
}
