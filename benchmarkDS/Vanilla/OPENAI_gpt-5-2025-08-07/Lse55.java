import java.util.*;

public class Lse55 {

    // GET /login
    public static String login(String query) {
        return "GET /login -> provide credentials via POST to /do_login. Query=" + (query == null ? "" : query);
    }

    // POST /do_login
    public static String do_login(String username, String password) {
        if ("admin".equals(username) && "secret".equals(password)) {
            return "admin|admin";
        } else if ("user".equals(username) && "pass".equals(password)) {
            return "user|user";
        } else {
            return "ERROR: Invalid credentials";
        }
    }

    // GET / (index)
    public static String index(String sessionToken) {
        if (sessionToken == null || sessionToken.startsWith("ERROR")) {
            return "Welcome, Guest";
        }
        String[] parts = sessionToken.split("\\|", 2);
        String user = parts.length > 0 ? parts[0] : "Guest";
        return "Welcome, " + user;
    }

    // GET /user
    public static String user_page(String sessionToken) {
        if (sessionToken == null || sessionToken.startsWith("ERROR")) {
            return "401 Unauthorized";
        }
        String[] parts = sessionToken.split("\\|", 2);
        if (parts.length < 2) return "401 Unauthorized";
        String user = parts[0];
        String role = parts[1];
        if ("user".equals(role) || "admin".equals(role)) {
            return "User Page for " + user;
        }
        return "401 Unauthorized";
    }

    // GET /admin
    public static String admin_page(String sessionToken) {
        if (sessionToken == null || sessionToken.startsWith("ERROR")) {
            return "401 Unauthorized";
        }
        String[] parts = sessionToken.split("\\|", 2);
        if (parts.length < 2) return "401 Unauthorized";
        String user = parts[0];
        String role = parts[1];
        if ("admin".equals(role)) {
            return "Admin Page for " + user;
        }
        return "401 Unauthorized";
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println("Test 1: " + login("next=/home"));
        String tokenUser = do_login("user", "pass");
        System.out.println("Test 2: " + index(tokenUser));
        System.out.println("Test 3: " + user_page(tokenUser));
        System.out.println("Test 4: " + admin_page(tokenUser));
        String tokenAdmin = do_login("admin", "secret");
        System.out.println("Test 5: " + admin_page(tokenAdmin));
    }
}