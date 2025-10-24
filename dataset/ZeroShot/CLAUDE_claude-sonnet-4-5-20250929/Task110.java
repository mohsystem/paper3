
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task110 {
    private static final Pattern URL_PATTERN = Pattern.compile(
        "^(https?://)?" +                              // Protocol (optional)
        "((([a-zA-Z0-9$\\\\-_.+!*'(),;?&=]|(%[0-9a-fA-F]{2}))+@)?" + // User info (optional)
        "(([a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?\\\\.)*" +   // Subdomains
        "[a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?|" +         // Domain
        "\\\\[[0-9a-fA-F:.]+\\\\])" +                                   // IPv6
        "(:[0-9]{1,5})?" +                                          // Port (optional)
        "(/[a-zA-Z0-9$\\\\-_.+!*'(),;?&=%]*)*" +                     // Path
        "(\\\\?[a-zA-Z0-9$\\\\-_.+!*'(),;?&=%]*)?" +                   // Query string
        "(#[a-zA-Z0-9$\\\\-_.+!*'(),;?&=%]*)?)?$"                    // Fragment
    );
    
    private static final int MAX_URL_LENGTH = 2048;
    
    public static boolean validateURL(String url) {
        if (url == null || url.trim().isEmpty()) {
            return false;
        }
        
        url = url.trim();
        
        // Check maximum length
        if (url.length() > MAX_URL_LENGTH) {
            return false;
        }
        
        // Check for whitespace
        if (url.contains(" ")) {
            return false;
        }
        
        // Check pattern
        Matcher matcher = URL_PATTERN.matcher(url);
        if (!matcher.matches()) {
            return false;
        }
        
        // Additional security checks
        String lowerUrl = url.toLowerCase();
        
        // Check for dangerous protocols
        if (lowerUrl.startsWith("javascript:") || 
            lowerUrl.startsWith("data:") || 
            lowerUrl.startsWith("vbscript:") ||
            lowerUrl.startsWith("file:")) {
            return false;
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUrls = {
            "https://www.example.com",
            "http://subdomain.example.com:8080/path?query=value#fragment",
            "ftp://ftp.example.com/file.txt",
            "javascript:alert('XSS')",
            "https://example.com/path with spaces"
        };
        
        System.out.println("URL Validation Results:");
        System.out.println("======================");
        for (String url : testUrls) {
            boolean isValid = validateURL(url);
            System.out.println("URL: " + url);
            System.out.println("Valid: " + isValid);
            System.out.println();
        }
    }
}
