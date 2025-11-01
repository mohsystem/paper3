
import java.sql.*;
import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

@WebServlet("/checkModerator")
public class Lse138 extends HttpServlet {
    
    private static final String DB_URL = "jdbc:mysql://localhost:3306/webapp_db";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String userId = request.getParameter("userId");
        String result = checkIfModerator(userId);
        
        response.setContentType("application/json");
        response.getWriter().write("{\\"isModerator\\": " + result + "}");
    }
    
    public static String checkIfModerator(String userId) {
        Connection conn = null;
        PreparedStatement stmt = null;
        ResultSet rs = null;
        String result = "false";
        
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String query = "SELECT is_moderator FROM users WHERE user_id = ?";
            stmt = conn.prepareStatement(query);
            stmt.setString(1, userId);
            
            rs = stmt.executeQuery();
            
            if (rs.next()) {
                result = String.valueOf(rs.getBoolean("is_moderator"));
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
        
        return result;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: " + checkIfModerator("1"));
        System.out.println("Test Case 2: " + checkIfModerator("2"));
        System.out.println("Test Case 3: " + checkIfModerator("3"));
        System.out.println("Test Case 4: " + checkIfModerator("4"));
        System.out.println("Test Case 5: " + checkIfModerator("5"));
    }
}
