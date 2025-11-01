
import java.sql.*;
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet("/api/temperature_for_location")
public class Lse86 extends HttpServlet {
    
    private static final String DB_URL = "jdbc:mysql://localhost:3306/weatherdb";
    private static final String DB_USER = "your_username";
    private static final String DB_PASSWORD = "your_password";
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        String latitude = request.getParameter("latitude");
        String longitude = request.getParameter("longitude");
        String year = request.getParameter("year");
        String month = request.getParameter("month");
        String day = request.getParameter("day");
        String gribFile = request.getParameter("grib_file");
        
        response.setContentType("application/json");
        PrintWriter out = response.getWriter();
        
        if (latitude == null || longitude == null || year == null || 
            month == null || day == null || gribFile == null) {
            out.println("{\\"error\\": \\"Missing required parameters\\"}");
            return;
        }
        
        Double maxTemp = getMaxTemperature(latitude, longitude, year, month, day, gribFile);
        
        if (maxTemp != null) {
            out.println("{\\"max_temperature\\": " + maxTemp + "}");
        } else {
            out.println("{\\"error\\": \\"No data found\\"}");
        }
    }
    
    private Double getMaxTemperature(String latitude, String longitude, 
                                     String year, String month, String day, String gribFile) {
        Connection conn = null;
        PreparedStatement stmt = null;
        ResultSet rs = null;
        Double maxTemp = null;
        
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String sql = "SELECT MAX(temperature) as max_temp FROM temperature_data " +
                        "WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? " +
                        "AND day = ? AND grib_file = ?";
            
            stmt = conn.prepareStatement(sql);
            stmt.setDouble(1, Double.parseDouble(latitude));
            stmt.setDouble(2, Double.parseDouble(longitude));
            stmt.setInt(3, Integer.parseInt(year));
            stmt.setInt(4, Integer.parseInt(month));
            stmt.setInt(5, Integer.parseInt(day));
            stmt.setString(6, gribFile);
            
            rs = stmt.executeQuery();
            
            if (rs.next()) {
                maxTemp = rs.getDouble("max_temp");
                if (rs.wasNull()) {
                    maxTemp = null;
                }
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
        
        return maxTemp;
    }
    
    public static void main(String[] args) {
        System.out.println("Web servlet - requires servlet container to run");
        System.out.println("Test cases would be executed via HTTP requests");
    }
}
