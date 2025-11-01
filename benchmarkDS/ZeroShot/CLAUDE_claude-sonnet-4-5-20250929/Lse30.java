
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Lse30 {
    private static final Pattern REDIRECT_PATTERN = Pattern.compile("^[a-zA-Z0-9-]+\\\\.");
    
    public static String secureRedirect(String target) {
        if (target == null || target.isEmpty()) {
            return "/";
        }
        
        Matcher matcher = REDIRECT_PATTERN.matcher(target);
        
        if (matcher.find()) {
            return target;
        } else {
            return "/";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + secureRedirect("example.com")); // Should return "example.com"
        System.out.println("Test 2: " + secureRedirect("my-site.org")); // Should return "my-site.org"
        System.out.println("Test 3: " + secureRedirect("test123.net")); // Should return "test123.net"
        System.out.println("Test 4: " + secureRedirect(".invalid")); // Should return "/"
        System.out.println("Test 5: " + secureRedirect("no-period")); // Should return "/"
    }
}
