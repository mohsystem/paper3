
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse54 {
    private static Map<String, String> userDatabase = new HashMap<>();
    private static Map<String, String> salts = new HashMap<>();
    
    static {
        // Initialize with secure password hashing (example only)
        try {
            String salt = generateSalt();
            salts.put("admin", salt);
            userDatabase.put("admin", hashPassword("admin123", salt));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    // GET request - returns login form HTML
    public static String login() {
        return "<!DOCTYPE html>\\n" +
               "<html>\\n" +
               "<head><title>Login</title></head>\\n" +
               "<body>\\n" +
               "<h2>Login Form</h2>\\n" +
               "<form method='POST' action='/do_login'>\\n" +
               "  <label>Username: <input type='text' name='username' required pattern='[a-zA-Z0-9_]{3,20}'></label><br>\\n" +
               "  <label>Password: <input type='password' name='password' required minlength='6'></label><br>\\n" +
               "  <input type='submit' value='Login'>\\n" +
               "</form>\\n" +
               "</body>\\n" +
               "</html>";
    }
    
    // POST request - processes login
    public static String doLogin(String username, String password) {
        // Input validation
        if (username == null || password == null || 
            username.trim().isEmpty() || password.trim().isEmpty()) {
            return "Invalid credentials";
        }
        
        // Sanitize username (alphanumeric and underscore only)
        if (!username.matches("^[a-zA-Z0-9_]{3,20}$")) {
            return "Invalid username format";
        }
        
        // Check if user exists and verify password
        if (userDatabase.containsKey(username)) {
            String salt = salts.get(username);
            String hashedPassword = hashPassword(password, salt);
            
            if (hashedPassword.equals(userDatabase.get(username))) {
                if ("admin".equals(username)) {
                    return "REDIRECT:/admin_page";
                }
                return "Login successful";
            }
        }
        
        return "Invalid credentials";
    }
    
    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    private static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1 - GET login form:");
        System.out.println(login().substring(0, 50) + "...\\n");
        
        System.out.println("Test Case 2 - Valid admin login:");
        System.out.println(doLogin("admin", "admin123") + "\\n");
        
        System.out.println("Test Case 3 - Invalid password:");
        System.out.println(doLogin("admin", "wrongpass") + "\\n");
        
        System.out.println("Test Case 4 - Non-existent user:");
        System.out.println(doLogin("user123", "password") + "\\n");
        
        System.out.println("Test Case 5 - Empty credentials:");
        System.out.println(doLogin("", "") + "\\n");
    }
}
