
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.sql.*;
import java.util.*;

public class Lse148 extends HttpServlet {
    
    // Database connection parameters
    private static final String DB_URL = "jdbc:mysql://localhost:3306/messagedb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    // Function to insert user message in database
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
    
    // Function to get all messages from database
    private List<Map<String, String>> getMessages() {
        List<Map<String, String>> messages = new ArrayList<>();
        Connection conn = null;
        Statement stmt = null;
        ResultSet rs = null;
        
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC LIMIT 50";
            stmt = conn.createStatement();
            rs = stmt.executeQuery(sql);
            
            while (rs.next()) {
                Map<String, String> msgMap = new HashMap<>();
                msgMap.put("username", rs.getString("username"));
                msgMap.put("message", rs.getString("message"));
                msgMap.put("timestamp", rs.getString("timestamp"));
                messages.add(msgMap);
            }
            
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (rs != null) rs.close();
                if (stmt != null) stmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        
        return messages;
    }
    
    // POST method handler
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        post(request, response);
    }
    
    public void post(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        // Get username and message from request
        String username = request.getParameter("username");
        String message = request.getParameter("message");
        
        // Validate input
        if (username != null && !username.trim().isEmpty() && 
            message != null && !message.trim().isEmpty()) {
            
            // Sanitize input to prevent XSS
            username = sanitizeInput(username);
            message = sanitizeInput(message);
            
            // Insert message in database
            insert_user_message_in_db(username, message);
        }
        
        // Redirect to main page
        response.sendRedirect("/");
    }
    
    // GET method handler to display messages
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        List<Map<String, String>> messages = getMessages();
        
        out.println("<!DOCTYPE html>");
        out.println("<html><head><title>Message Board</title></head><body>");
        out.println("<h1>Message Board</h1>");
        
        // Display form
        out.println("<form method='post' action='/post'>");
        out.println("Username: <input type='text' name='username' required><br>");
        out.println("Message: <textarea name='message' required></textarea><br>");
        out.println("<input type='submit' value='Submit'>");
        out.println("</form>");
        
        out.println("<hr>");
        out.println("<h2>Messages:</h2>");
        
        // Display messages
        for (Map<String, String> msg : messages) {
            out.println("<div style='border:1px solid #ccc; margin:10px; padding:10px;'>");
            out.println("<strong>" + escapeHtml(msg.get("username")) + "</strong> ");
            out.println("<small>(" + escapeHtml(msg.get("timestamp")) + ")</small><br>");
            out.println("<p>" + escapeHtml(msg.get("message")) + "</p>");
            out.println("</div>");
        }
        
        out.println("</body></html>");
    }
    
    // Sanitize input
    private String sanitizeInput(String input) {
        if (input == null) return "";
        return input.trim().replaceAll("[<>\\"']", "");
    }
    
    // Escape HTML for output
    private String escapeHtml(String text) {
        if (text == null) return "";
        return text.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    public static void main(String[] args) {
        System.out.println("Servlet-based application. Deploy to a servlet container like Tomcat.");
        System.out.println("Test cases should be performed via HTTP requests.");
    }
}
