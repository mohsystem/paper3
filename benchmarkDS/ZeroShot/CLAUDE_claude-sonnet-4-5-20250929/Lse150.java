
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.sql.*;
import java.util.*;

public class Lse150 extends HttpServlet {
    
    // Database connection details
    private static final String DB_URL = "jdbc:mysql://localhost:3306/messagedb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    // Method to insert user message into database
    private boolean insert_user_message_in_db(String username, String message) {
        if (username == null || message == null || username.trim().isEmpty() || message.trim().isEmpty()) {
            return false;
        }
        
        // Sanitize inputs
        username = sanitizeInput(username);
        message = sanitizeInput(message);
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String sql = "INSERT INTO messages (username, message, timestamp) VALUES (?, ?, ?)";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, message);
            pstmt.setTimestamp(3, new Timestamp(System.currentTimeMillis()));
            
            int result = pstmt.executeUpdate();
            return result > 0;
            
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
    
    // Sanitize input to prevent XSS and SQL injection
    private String sanitizeInput(String input) {
        if (input == null) return "";
        return input.replaceAll("[<>\\"'&]", "")
                   .substring(0, Math.min(input.length(), 500));
    }
    
    // POST method handler
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        post(request, response);
    }
    
    // Main post function
    public void post(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        // Get username and message from request
        String username = request.getParameter("username");
        String message = request.getParameter("message");
        
        // Validate inputs
        if (username != null && message != null && 
            !username.trim().isEmpty() && !message.trim().isEmpty()) {
            
            // Insert into database
            insert_user_message_in_db(username, message);
        }
        
        // Redirect back to main page
        response.sendRedirect(request.getContextPath() + "/");
    }
    
    // GET method to display the form
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        out.println("<html><body>");
        out.println("<h2>Submit a Message</h2>");
        out.println("<form method='post' action='post'>");
        out.println("Username: <input type='text' name='username' required maxlength='100'><br><br>");
        out.println("Message: <textarea name='message' required maxlength='500'></textarea><br><br>");
        out.println("<input type='submit' value='Submit'>");
        out.println("</form>");
        out.println("</body></html>");
    }
    
    // Test cases in main method
    public static void main(String[] args) {
        System.out.println("Test cases for message submission:");
        
        Lse150 servlet = new Lse150();
        
        // Test case 1: Valid username and message
        System.out.println("Test 1: Valid input");
        System.out.println(servlet.insert_user_message_in_db("john_doe", "Hello World!"));
        
        // Test case 2: Empty username
        System.out.println("Test 2: Empty username");
        System.out.println(servlet.insert_user_message_in_db("", "Test message"));
        
        // Test case 3: Null message
        System.out.println("Test 3: Null message");
        System.out.println(servlet.insert_user_message_in_db("jane_doe", null));
        
        // Test case 4: XSS attempt
        System.out.println("Test 4: XSS attempt");
        System.out.println(servlet.insert_user_message_in_db("attacker", "<script>alert('XSS')</script>"));
        
        // Test case 5: SQL injection attempt
        System.out.println("Test 5: SQL injection attempt");
        System.out.println(servlet.insert_user_message_in_db("hacker", "'; DROP TABLE messages; --"));
    }
}
