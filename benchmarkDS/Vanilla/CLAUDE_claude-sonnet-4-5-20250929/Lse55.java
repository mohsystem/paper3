
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.util.*;

public class Lse55 extends HttpServlet {
    private Map<String, User> users = new HashMap<>();
    private Map<String, String> sessions = new HashMap<>();
    
    static class User {
        String username;
        String password;
        String role;
        
        User(String username, String password, String role) {
            this.username = username;
            this.password = password;
            this.role = role;
        }
    }
    
    public void init() {
        // Initialize with some test users
        users.put("admin", new User("admin", "admin123", "admin"));
        users.put("user1", new User("user1", "pass123", "user"));
        users.put("user2", new User("user2", "pass456", "user"));
    }
    
    // GET: Login page
    public String login(HttpServletRequest request, HttpServletResponse response) {
        return "<html><body><h2>Login Page</h2>" +
               "<form method='POST' action='/do_login'>" +
               "Username: <input type='text' name='username'><br>" +
               "Password: <input type='password' name='password'><br>" +
               "<input type='submit' value='Login'>" +
               "</form></body></html>";
    }
    
    // POST: Process login
    public String do_login(HttpServletRequest request, HttpServletResponse response) {
        String username = request.getParameter("username");
        String password = request.getParameter("password");
        
        if (users.containsKey(username)) {
            User user = users.get(username);
            if (user.password.equals(password)) {
                String sessionId = UUID.randomUUID().toString();
                sessions.put(sessionId, username);
                HttpSession session = request.getSession();
                session.setAttribute("sessionId", sessionId);
                session.setAttribute("username", username);
                session.setAttribute("role", user.role);
                return "Login successful! Redirecting...";
            }
        }
        return "Login failed! Invalid credentials.";
    }
    
    // GET: Index/Home page
    public String index(HttpServletRequest request, HttpServletResponse response) {
        HttpSession session = request.getSession(false);
        String username = session != null ? (String) session.getAttribute("username") : null;
        
        if (username != null) {
            return "<html><body><h2>Welcome to the Index Page</h2>" +
                   "<p>Hello, " + username + "!</p>" +
                   "<a href='/user_page'>User Page</a><br>" +
                   "<a href='/admin_page'>Admin Page</a>" +
                   "</body></html>";
        }
        return "<html><body><h2>Index Page</h2>" +
               "<p>Please <a href='/login'>login</a> to continue.</p>" +
               "</body></html>";
    }
    
    // GET: User page
    public String user_page(HttpServletRequest request, HttpServletResponse response) {
        HttpSession session = request.getSession(false);
        String username = session != null ? (String) session.getAttribute("username") : null;
        
        if (username == null) {
            return "<html><body><h2>Access Denied</h2>" +
                   "<p>Please <a href='/login'>login</a> first.</p>" +
                   "</body></html>";
        }
        
        return "<html><body><h2>User Page</h2>" +
               "<p>Welcome, " + username + "!</p>" +
               "<p>This is your user dashboard.</p>" +
               "<a href='/index'>Back to Index</a>" +
               "</body></html>";
    }
    
    // GET: Admin page
    public String admin_page(HttpServletRequest request, HttpServletResponse response) {
        HttpSession session = request.getSession(false);
        String username = session != null ? (String) session.getAttribute("username") : null;
        String role = session != null ? (String) session.getAttribute("role") : null;
        
        if (username == null) {
            return "<html><body><h2>Access Denied</h2>" +
                   "<p>Please <a href='/login'>login</a> first.</p>" +
                   "</body></html>";
        }
        
        if (!"admin".equals(role)) {
            return "<html><body><h2>Access Denied</h2>" +
                   "<p>You don't have admin privileges.</p>" +
                   "<a href='/index'>Back to Index</a>" +
                   "</body></html>";
        }
        
        return "<html><body><h2>Admin Page</h2>" +
               "<p>Welcome, Administrator " + username + "!</p>" +
               "<p>This is the admin control panel.</p>" +
               "<a href='/index'>Back to Index</a>" +
               "</body></html>";
    }
    
    public static void main(String[] args) {
        System.out.println("Web Application Functions:");
        System.out.println("1. GET /login - Display login form");
        System.out.println("2. POST /do_login - Process login credentials");
        System.out.println("3. GET /index - Display home page");
        System.out.println("4. GET /user_page - Display user dashboard");
        System.out.println("5. GET /admin_page - Display admin panel");
        
        // Test cases simulation
        Lse55 app = new Lse55();
        app.init();
        
        System.out.println("\\nTest Case 1: Access login page");
        System.out.println("Expected: Login form HTML");
        
        System.out.println("\\nTest Case 2: Login with valid user credentials");
        System.out.println("Username: user1, Password: pass123");
        System.out.println("Expected: Login successful");
        
        System.out.println("\\nTest Case 3: Access index page without login");
        System.out.println("Expected: Please login message");
        
        System.out.println("\\nTest Case 4: Access user_page with valid session");
        System.out.println("Expected: User dashboard");
        
        System.out.println("\\nTest Case 5: Access admin_page with non-admin user");
        System.out.println("Expected: Access denied - no admin privileges");
    }
}
