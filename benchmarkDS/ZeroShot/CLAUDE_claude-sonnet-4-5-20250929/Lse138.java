
import java.sql.*;
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet("/checkModerator")
public class Lse138 extends HttpServlet {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/webapp";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String username = request.getParameter("username");
        response.setContentType("application/json");
        PrintWriter out = response.getWriter();
        
        try {
            boolean isModerator = checkIfModerator(username);
            out.println("{\\"isModerator\\": " + isModerator + "}");
        } catch (SQLException e) {
            response.setStatus(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
            out.println("{\\"error\\": \\"Database error\\"}");
        }
    }
    
    private boolean checkIfModerator(String username) throws SQLException {
        if (username == null || username.isEmpty()) {
            return false;
        }
        
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD)) {
            String query = "SELECT is_moderator FROM users WHERE username = ?";
            try (PreparedStatement stmt = conn.prepareStatement(query)) {
                stmt.setString(1, username);
                try (ResultSet rs = stmt.executeQuery()) {
                    if (rs.next()) {
                        return rs.getBoolean("is_moderator");
                    }
                }
            }
        }
        return false;
    }
    
    public static void main(String[] args) {
        // Test cases would require a running servlet container
        // Simulating direct method calls for testing
        Lse138 app = new Lse138();
        String[] testUsers = {"admin", "moderator1", "user1", "testmod", "normaluser"};
        
        System.out.println("Test Cases:");
        for (String user : testUsers) {
            try {
                boolean result = app.checkIfModerator(user);
                System.out.println("User: " + user + " - Is Moderator: " + result);
            } catch (SQLException e) {
                System.out.println("User: " + user + " - Error: " + e.getMessage());
            }
        }
    }
}
