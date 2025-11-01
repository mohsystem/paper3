
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

public class Lse39 {
    private static final Map<String, String> userDatabase = new HashMap<>();
    
    static {
        // Initialize with some test users
        userDatabase.put("admin", "admin123");
        userDatabase.put("user1", "password1");
        userDatabase.put("user2", "password2");
        userDatabase.put("testuser", "test123");
        userDatabase.put("john", "doe456");
    }
    
    public static String login(String username, String password) {
        if (username == null || password == null) {
            return "Username or password is incorrect";
        }
        
        if (userDatabase.containsKey(username) && userDatabase.get(username).equals(password)) {
            return "Login successful! Redirecting to session page...";
        } else {
            return "Username or password is incorrect";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(login("admin", "admin123"));
        System.out.println();
        
        System.out.println("Test Case 2:");
        System.out.println(login("user1", "wrongpassword"));
        System.out.println();
        
        System.out.println("Test Case 3:");
        System.out.println(login("nonexistent", "password"));
        System.out.println();
        
        System.out.println("Test Case 4:");
        System.out.println(login("testuser", "test123"));
        System.out.println();
        
        System.out.println("Test Case 5:");
        System.out.println(login("john", "doe456"));
    }
}
