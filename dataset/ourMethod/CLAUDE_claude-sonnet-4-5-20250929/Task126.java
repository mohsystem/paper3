
import java.util.*;
import java.util.regex.Pattern;

public class Task126 {
    private static final Map<String, String> ALLOWED_PAGES = new HashMap<>();
    private static final Pattern SAFE_PAGE_PATTERN = Pattern.compile("^[a-z0-9_-]+$");
    
    static {
        ALLOWED_PAGES.put("home", "/pages/home.html");
        ALLOWED_PAGES.put("about", "/pages/about.html");
        ALLOWED_PAGES.put("contact", "/pages/contact.html");
        ALLOWED_PAGES.put("products", "/pages/products.html");
        ALLOWED_PAGES.put("services", "/pages/services.html");
    }
    
    public static String redirectToPage(String userInput) {
        if (userInput == null || userInput.isEmpty()) {
            return "Error: Invalid input";
        }
        
        String sanitizedInput = userInput.trim().toLowerCase();
        
        if (sanitizedInput.length() > 50) {
            return "Error: Input too long";
        }
        
        if (!SAFE_PAGE_PATTERN.matcher(sanitizedInput).matches()) {
            return "Error: Invalid page name format";
        }
        
        String targetPage = ALLOWED_PAGES.get(sanitizedInput);
        
        if (targetPage == null) {
            return "Error: Page not found";
        }
        
        return "Redirect to: " + targetPage;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: " + redirectToPage("home"));
        System.out.println("Test Case 2: " + redirectToPage("about"));
        System.out.println("Test Case 3: " + redirectToPage("invalid"));
        System.out.println("Test Case 4: " + redirectToPage("../../../etc/passwd"));
        System.out.println("Test Case 5: " + redirectToPage(null));
    }
}
