
import java.util.*;
import java.util.regex.Pattern;

public class Task126 {
    private static final Map<String, String> ALLOWED_PAGES = new HashMap<>();
    
    static {
        ALLOWED_PAGES.put("home", "/home");
        ALLOWED_PAGES.put("about", "/about");
        ALLOWED_PAGES.put("contact", "/contact");
        ALLOWED_PAGES.put("services", "/services");
        ALLOWED_PAGES.put("profile", "/profile");
    }
    
    public static String redirectUser(String userInput) {
        if (userInput == null || userInput.trim().isEmpty()) {
            return "Error: Invalid input. Redirecting to default page: /home";
        }
        
        // Sanitize input: convert to lowercase and remove special characters
        String sanitizedInput = userInput.trim().toLowerCase();
        sanitizedInput = sanitizedInput.replaceAll("[^a-z0-9]", "");
        
        // Check against whitelist
        if (ALLOWED_PAGES.containsKey(sanitizedInput)) {
            return "Redirecting to: " + ALLOWED_PAGES.get(sanitizedInput);
        } else {
            return "Error: Page not found. Redirecting to default page: /home";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== User Redirection System (Java) ===\\n");
        
        // Test cases
        String[] testCases = {
            "home",
            "about",
            "contact",
            "invalid_page",
            "../../../etc/passwd"
        };
        
        for (String testCase : testCases) {
            System.out.println("Input: \\"" + testCase + "\\"");
            System.out.println("Result: " + redirectUser(testCase));
            System.out.println();
        }
    }
}
