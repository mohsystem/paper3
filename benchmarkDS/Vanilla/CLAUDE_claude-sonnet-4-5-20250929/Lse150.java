
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.sql.*;
import java.util.*;

public class Lse150 extends HttpServlet {
    
    // Database connection details
    private static final String DB_URL = "jdbc:mysql://localhost:3306/messages_db";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    // Function to insert user message into database
    private void insert_user_message_in_db(String username, String message) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String sql = "INSERT INTO messages (username, message, timestamp) VALUES (?, ?, NOW())";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, message);
            pstmt.executeUpdate();
            
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
    
    // POST method to handle form submission
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        post(request, response);
    }
    
    public void post(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        // Get username and message from request
        String username = request.getParameter("username");
        String message = request.getParameter("message");
        
        // Insert message into database
        if (username != null && message != null && !username.isEmpty() && !message.isEmpty()) {
            insert_user_message_in_db(username, message);
        }
        
        // Redirect to main page
        response.sendRedirect(request.getContextPath() + "/");
    }
    
    // GET method to display the main page with form
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        out.println("<!DOCTYPE html>");
        out.println("<html>");
        out.println("<head><title>Message Board</title></head>");
        out.println("<body>");
        out.println("<h1>Message Board</h1>");
        out.println("<form method='post' action='" + request.getContextPath() + "/post'>");
        out.println("<label>Username: <input type='text' name='username' required></label><br><br>");
        out.println("<label>Message: <textarea name='message' required></textarea></label><br><br>");
        out.println("<input type='submit' value='Submit Message'>");
        out.println("</form>");
        out.println("</body>");
        out.println("</html>");
    }
    
    // Main method for testing (not typical for servlets)
    public static void main(String[] args) {
        System.out.println("This is a servlet application. Deploy to a web server to test.");
        System.out.println("Test cases would be:");
        System.out.println("1. Submit with username='John' and message='Hello World'");
        System.out.println("2. Submit with username='Alice' and message='Test message'");
        System.out.println("3. Submit with username='Bob' and message='Another test'");
        System.out.println("4. Submit with username='Charlie' and message='Fourth message'");
        System.out.println("5. Submit with username='David' and message='Last test message'");
    }
}
