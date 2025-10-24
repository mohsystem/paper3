
import java.util.Scanner;
import java.util.HashMap;
import java.util.Map;

public class Task126 {
    private static Map<String, String> pageRoutes = new HashMap<>();
    
    static {
        pageRoutes.put("home", "https://example.com/home");
        pageRoutes.put("about", "https://example.com/about");
        pageRoutes.put("contact", "https://example.com/contact");
        pageRoutes.put("products", "https://example.com/products");
        pageRoutes.put("services", "https://example.com/services");
    }
    
    public static String redirectUser(String userInput) {
        if (userInput == null || userInput.trim().isEmpty()) {
            return "Error: Invalid input";
        }
        
        String normalizedInput = userInput.toLowerCase().trim();
        
        if (pageRoutes.containsKey(normalizedInput)) {
            return "Redirecting to: " + pageRoutes.get(normalizedInput);
        } else {
            return "Error: Page not found. Redirecting to: " + pageRoutes.get("home");
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Page Redirect System - Test Cases ===\\n");
        
        // Test Case 1: Valid page - home
        String test1 = "home";
        System.out.println("Test 1 - Input: '" + test1 + "'");
        System.out.println("Result: " + redirectUser(test1));
        System.out.println();
        
        // Test Case 2: Valid page - about
        String test2 = "about";
        System.out.println("Test 2 - Input: '" + test2 + "'");
        System.out.println("Result: " + redirectUser(test2));
        System.out.println();
        
        // Test Case 3: Valid page with mixed case
        String test3 = "PRODUCTS";
        System.out.println("Test 3 - Input: '" + test3 + "'");
        System.out.println("Result: " + redirectUser(test3));
        System.out.println();
        
        // Test Case 4: Invalid page
        String test4 = "unknown";
        System.out.println("Test 4 - Input: '" + test4 + "'");
        System.out.println("Result: " + redirectUser(test4));
        System.out.println();
        
        // Test Case 5: Empty input
        String test5 = "";
        System.out.println("Test 5 - Input: '" + test5 + "'");
        System.out.println("Result: " + redirectUser(test5));
        System.out.println();
    }
}
