import static spark.Spark.*;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

// Note: To run this, you need the SparkJava library.
// For Maven, add this to your pom.xml:
// <dependency>
//     <groupId>com.sparkjava</groupId>
//     <artifactId>spark-core</artifactId>
//     <version>2.9.4</version>
// </dependency>
public class Lse55 {

    // In-memory store for user data. In a real application, use a database with hashed passwords.
    private static final Map<String, User> users = new HashMap<>();
    // In-memory store for sessions. In a real application, use a distributed cache like Redis.
    private static final Map<String, Session> sessions = new ConcurrentHashMap<>();

    // User model
    static class User {
        String username;
        String passwordHash; // Store HASHED passwords, not plain text. This is just an example.
        String role;

        User(String username, String password, String role) {
            this.username = username;
            // In a real app, use a strong hashing algorithm like BCrypt or Argon2
            // e.g., this.passwordHash = BCrypt.hashpw(password, BCrypt.gensalt());
            this.passwordHash = password; // For demonstration only!
            this.role = role;
        }

        boolean checkPassword(String password) {
            // In a real app, use the hashing library's verify function
            // e.g., return BCrypt.checkpw(password, this.passwordHash);
            return this.passwordHash.equals(password);
        }
    }

    // Session model
    static class Session {
        String username;
        String role;
        String csrfToken;

        Session(String username, String role, String csrfToken) {
            this.username = username;
            this.role = role;
            this.csrfToken = csrfToken;
        }
    }

    public static void main(String[] args) {
        // Initialize user data
        users.put("user", new User("user", "password123", "USER"));
        users.put("admin", new User("admin", "adminpass", "ADMIN"));
        
        port(8080);

        // --- Routes ---

        // 1. Index route (GET)
        get("/", (req, res) -> {
            String sessionId = req.cookie("sessionId");
            if (sessionId != null && sessions.containsKey(sessionId)) {
                Session session = sessions.get(sessionId);
                return String.format("<h1>Welcome, %s!</h1><p><a href='/user'>User Page</a></p><p><a href='/logout'>Logout</a></p>", session.username);
            }
            return "<h1>Welcome!</h1><p>Please <a href='/login'>login</a>.</p>";
        });

        // 2. Login form route (GET)
        get("/login", (req, res) -> {
            String sessionId = req.cookie("sessionId");
            if (sessionId != null && sessions.containsKey(sessionId)) {
                res.redirect("/");
                return "";
            }

            // Create a temporary session for CSRF token
            String tempSessionId = UUID.randomUUID().toString();
            String csrfToken = UUID.randomUUID().toString();
            sessions.put(tempSessionId, new Session(null, null, csrfToken));
            res.cookie("/", "sessionId", tempSessionId, 300, false, true); // 5 min expiry, httpOnly

            return "<form action='/do_login' method='post'>"
                 + "<h2>Login</h2>"
                 + "<label for='username'>Username:</label><br>"
                 + "<input type='text' id='username' name='username'><br>"
                 + "<label for='password'>Password:</label><br>"
                 + "<input type='password' id='password' name='password'><br>"
                 + "<input type='hidden' name='csrf_token' value='" + csrfToken + "'>"
                 + "<br><input type='submit' value='Login'>"
                 + "</form>";
        });

        // 3. Login action route (POST)
        post("/do_login", (req, res) -> {
            String username = req.queryParams("username");
            String password = req.queryParams("password");
            String formCsrfToken = req.queryParams("csrf_token");
            
            String sessionId = req.cookie("sessionId");
            Session session = sessions.get(sessionId);

            // CSRF Check
            if (session == null || session.csrfToken == null || !session.csrfToken.equals(formCsrfToken)) {
                res.status(403);
                return "Forbidden: Invalid CSRF Token.";
            }

            // Authentication
            User user = users.get(username);
            if (user != null && user.checkPassword(password)) {
                // Invalidate old temp session
                sessions.remove(sessionId);

                // Create a new authenticated session
                String newSessionId = UUID.randomUUID().toString();
                String newCsrfToken = UUID.randomUUID().toString(); // Regenerate CSRF token after login
                sessions.put(newSessionId, new Session(user.username, user.role, newCsrfToken));
                res.cookie("/", "sessionId", newSessionId, 3600, false, true); // 1 hour expiry
                res.redirect("/");
                return "";
            } else {
                res.redirect("/login?error=1");
                return "Invalid credentials";
            }
        });

        // 4. User page (GET)
        get("/user", (req, res) -> {
            String sessionId = req.cookie("sessionId");
            Session session = sessions.get(sessionId);
            if (session != null && session.username != null) { // Authenticated session
                return String.format("<h1>User Page</h1><p>Hello, %s. Your role is %s.</p><p><a href='/'>Home</a></p>", session.username, session.role);
            }
            res.redirect("/login");
            return "";
        });

        // 5. Admin page (GET)
        get("/admin", (req, res) -> {
            String sessionId = req.cookie("sessionId");
            Session session = sessions.get(sessionId);
            if (session != null && "ADMIN".equals(session.role)) {
                return "<h1>Admin Page</h1><p>Welcome, administrator!</p><p><a href='/'>Home</a></p>";
            } else if (session != null) {
                res.status(403);
                return "<h1>403 Forbidden</h1><p>You do not have access to this page.</p>";
            } else {
                res.redirect("/login");
                return "";
            }
        });

        get("/logout", (req, res) -> {
            String sessionId = req.cookie("sessionId");
            if (sessionId != null) {
                sessions.remove(sessionId);
            }
            res.removeCookie("sessionId");
            res.redirect("/");
            return "";
        });

        System.out.println("Server started on http://localhost:8080");
        System.out.println("--- Test Cases ---");
        System.out.println("1. Visit the index page: http://localhost:8080/");
        System.out.println("2. Visit the login page: http://localhost:8080/login");
        System.out.println("3. Login as 'user' with password 'password123' via the form.");
        System.out.println("4. After logging in, visit the user page: http://localhost:8080/user");
        System.out.println("5. Try to visit the admin page (will be forbidden): http://localhost:8080/admin");
        System.out.println("   Then, logout and log in as 'admin' with password 'adminpass' to access it.");

    }
}