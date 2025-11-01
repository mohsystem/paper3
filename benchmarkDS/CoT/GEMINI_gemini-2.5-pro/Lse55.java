import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

// Main class name as per instructions
class Lse55 {

    // --- In-memory data stores for simulation ---

    // A simple in-memory map to store users and their details (password, role)
    // In a real application, passwords must be securely hashed and stored in a database.
    private static final Map<String, User> userDatabase = new HashMap<>();

    // A simple in-memory map to simulate user sessions.
    // Maps a session ID to a username.
    private static final Map<String, String> sessionDatabase = new HashMap<>();

    // Static initializer to populate the user database
    static {
        // NOTE: Storing plain text passwords is a major security vulnerability.
        // Use a strong hashing algorithm like Argon2, scrypt, or bcrypt in a real application.
        userDatabase.put("user1", new User("user1", "pass123", "user"));
        userDatabase.put("admin", new User("admin123", "admin"));
    }

    // Helper class to represent a user
    static class User {
        String password;
        String role;

        User(String password, String role) {
            this.password = password;
            this.role = role;
        }
    }

    // --- Simulated Web Request Handlers ---

    /**
     * Function 1: login (GET request)
     * Simulates a GET request to the login page.
     * @return A string representing the HTML content of the login page.
     */
    public String login() {
        System.out.println("GET /login -> Displaying login form.");
        return "<html><body><form action='/do_login' method='post'> " +
               "Username: <input type='text' name='username'><br>" +
               "Password: <input type='password' name='password'><br>" +
               "<input type='submit' value='Login'>" +
               "</form></body></html>";
    }

    /**
     * Function 2: do_login (POST request)
     * Simulates a POST request to process login credentials.
     * @param username The username submitted from the form.
     * @param password The password submitted from the form.
     * @return A session ID on success, or null on failure.
     */
    public String do_login(String username, String password) {
        System.out.println("POST /do_login -> Attempting to log in user: " + username);
        // Basic input validation
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Login failed: Username or password is empty.");
            return null;
        }

        User user = userDatabase.get(username);
        // In a real application, compare a hash of the provided password with the stored hash.
        if (user != null && user.password.equals(password)) {
            // Generate a secure, random session ID
            String sessionId = UUID.randomUUID().toString();
            sessionDatabase.put(sessionId, username);
            System.out.println("Login successful for user '" + username + "'. Session ID: " + sessionId);
            return sessionId;
        } else {
            System.out.println("Login failed: Invalid credentials for user '" + username + "'.");
            return null;
        }
    }

    /**
     * Function 3: index (GET request)
     * Simulates a GET request to the public index page.
     * @return A string representing the content of the index page.
     */
    public String index() {
        System.out.println("GET / -> Displaying public index page.");
        return "Welcome to the public index page!";
    }

    /**
     * Function 4: user_page (GET request)
     * Simulates a GET request to a page for authenticated users.
     * @param sessionId The session ID of the user trying to access the page.
     * @return The user page content on success, or an error message on failure.
     */
    public String user_page(String sessionId) {
        System.out.println("GET /user -> Access attempt with session: " + sessionId);
        if (sessionId != null && sessionDatabase.containsKey(sessionId)) {
            String username = sessionDatabase.get(sessionId);
            return "Welcome to the user page, " + username + "!";
        } else {
            return "Unauthorized: Please log in to see this page.";
        }
    }

    /**
     * Function 5: admin_page (GET request)
     * Simulates a GET request to a page for authenticated admin users.
     * @param sessionId The session ID of the user trying to access the page.
     * @return The admin page content on success, or an error message on failure.
     */
    public String admin_page(String sessionId) {
        System.out.println("GET /admin -> Access attempt with session: " + sessionId);
        if (sessionId != null && sessionDatabase.containsKey(sessionId)) {
            String username = sessionDatabase.get(sessionId);
            User user = userDatabase.get(username);
            if (user != null && "admin".equals(user.role)) {
                return "Welcome to the ADMIN page, " + username + "!";
            }
        }
        return "Forbidden: You do not have admin privileges to see this page.";
    }

    // --- Main method with test cases ---
    public static void main(String[] args) {
        Lse55 webApp = new Lse55();

        System.out.println("--- Test Case 1: Accessing a protected page without logging in ---");
        System.out.println("Response: " + webApp.user_page(null));
        System.out.println();

        System.out.println("--- Test Case 2: A failed login attempt with wrong password ---");
        String failedSessionId = webApp.do_login("user1", "wrongpassword");
        System.out.println("Session ID returned: " + failedSessionId);
        System.out.println("Accessing user page with failed session: " + webApp.user_page(failedSessionId));
        System.out.println();

        System.out.println("--- Test Case 3: A successful user login and accessing the user page ---");
        String userSessionId = webApp.do_login("user1", "pass123");
        System.out.println("Session ID returned: " + userSessionId);
        System.out.println("Response: " + webApp.user_page(userSessionId));
        System.out.println();

        System.out.println("--- Test Case 4: A logged-in user trying to access the admin page (should fail) ---");
        System.out.println("Response: " + webApp.admin_page(userSessionId));
        System.out.println();

        System.out.println("--- Test Case 5: A successful admin login and accessing the admin page ---");
        String adminSessionId = webApp.do_login("admin", "admin123");
        System.out.println("Session ID returned: " + adminSessionId);
        System.out.println("Accessing user page: " + webApp.user_page(adminSessionId));
        System.out.println("Accessing admin page: " + webApp.admin_page(adminSessionId));
        System.out.println();
    }
}