
import java.io.*;
import java.sql.*;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;

public class Lse149 extends HttpServlet {
    private static final String DB_URL = "jdbc:sqlite:messages.db";
    
    static {
        try {
            Class.forName("org.sqlite.JDBC");
            initializeDatabase();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS messages " +
                        "(id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                        " username TEXT NOT NULL, " +
                        " message TEXT NOT NULL, " +
                        " timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    public static void insertUserMessageInDb(String username, String message) {
        String sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, username);
            pstmt.setString(2, message);
            pstmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    public static List<Map<String, String>> getMessages() {
        List<Map<String, String>> messages = new ArrayList<>();
        String sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            while (rs.next()) {
                Map<String, String> msg = new HashMap<>();
                msg.put("username", rs.getString("username"));
                msg.put("message", rs.getString("message"));
                msg.put("timestamp", rs.getString("timestamp"));
                messages.add(msg);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return messages;
    }
    
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String path = request.getPathInfo() != null ? request.getPathInfo() : "/";
        
        if (path.equals("/") || path.equals("")) {
            displayMessages(response);
        } else {
            response.sendError(HttpServletResponse.SC_NOT_FOUND);
        }
    }
    
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String path = request.getPathInfo() != null ? request.getPathInfo() : "/";
        
        if (path.equals("/post")) {
            String username = request.getParameter("username");
            String message = request.getParameter("message");
            
            if (username != null && message != null && !username.isEmpty() && !message.isEmpty()) {
                insertUserMessageInDb(username, message);
            }
            response.sendRedirect(request.getContextPath() + "/");
        } else {
            response.sendError(HttpServletResponse.SC_NOT_FOUND);
        }
    }
    
    private void displayMessages(HttpServletResponse response) throws IOException {
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        out.println("<html><body>");
        out.println("<h1>Message Board</h1>");
        out.println("<form method='post' action='/post'>");
        out.println("Username: <input type='text' name='username'><br>");
        out.println("Message: <textarea name='message'></textarea><br>");
        out.println("<input type='submit' value='Post'>");
        out.println("</form><hr>");
        
        List<Map<String, String>> messages = getMessages();
        for (Map<String, String> msg : messages) {
            out.println("<p><strong>" + msg.get("username") + "</strong>: " + 
                       msg.get("message") + " <em>(" + msg.get("timestamp") + ")</em></p>");
        }
        out.println("</body></html>");
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Insert message from user1");
        insertUserMessageInDb("user1", "Hello World!");
        
        System.out.println("Test Case 2: Insert message from user2");
        insertUserMessageInDb("user2", "This is a test message");
        
        System.out.println("Test Case 3: Insert message from user3");
        insertUserMessageInDb("user3", "Java servlet example");
        
        System.out.println("Test Case 4: Insert message from user4");
        insertUserMessageInDb("user4", "Another test message");
        
        System.out.println("Test Case 5: Display all messages");
        List<Map<String, String>> messages = getMessages();
        for (Map<String, String> msg : messages) {
            System.out.println(msg.get("username") + ": " + msg.get("message"));
        }
    }
}
