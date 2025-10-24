
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

public class Task126 {
    private static final Map<String, String> validPages = new HashMap<>();
    
    static {
        validPages.put("home", "/home");
        validPages.put("about", "/about");
        validPages.put("contact", "/contact");
        validPages.put("services", "/services");
        validPages.put("products", "/products");
    }
    
    public static String redirectUser(String userInput) {
        if (userInput == null || userInput.trim().isEmpty()) {
            return "Invalid input. Redirecting to default page: /home";
        }
        
        String sanitizedInput = userInput.trim().toLowerCase();
        
        if (validPages.containsKey(sanitizedInput)) {
            return "Redirecting to: " + validPages.get(sanitizedInput);
        } else {
            return "Page not found. Redirecting to default page: /home";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(redirectUser("home"));
        System.out.println();
        
        System.out.println("Test Case 2:");
        System.out.println(redirectUser("about"));
        System.out.println();
        
        System.out.println("Test Case 3:");
        System.out.println(redirectUser("invalidpage"));
        System.out.println();
        
        System.out.println("Test Case 4:");
        System.out.println(redirectUser(""));
        System.out.println();
        
        System.out.println("Test Case 5:");
        System.out.println(redirectUser("PRODUCTS"));
        System.out.println();
    }
}
