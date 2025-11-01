
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import java.sql.*;

public class Lse86 extends HttpServlet {
    
    private static final String DB_URL = "jdbc:mysql://localhost:3306/weather_db";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        String latitude = request.getParameter("latitude");
        String longitude = request.getParameter("longitude");
        String year = request.getParameter("year");
        String month = request.getParameter("month");
        String day = request.getParameter("day");
        String gribFile = request.getParameter("grib_file");
        
        Double maxTemperature = getMaxTemperature(latitude, longitude, year, month, day, gribFile);
        
        response.setContentType("application/json");
        PrintWriter out = response.getWriter();
        
        if (maxTemperature != null) {
            out.println("{\\"max_temperature\\": " + maxTemperature + "}");
        } else {
            out.println("{\\"error\\": \\"No data found\\"}");
        }
    }
    
    private Double getMaxTemperature(String latitude, String longitude, String year, 
                                     String month, String day, String gribFile) {
        Double maxTemp = null;
        Connection conn = null;
        PreparedStatement stmt = null;
        ResultSet rs = null;
        
        try {
            Class.forName("com.mysql.cj.jdbc.Driver");
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String query = "SELECT MAX(temperature) as max_temp FROM temperature_data " +
                          "WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? " +
                          "AND day = ? AND grib_file = ?";
            
            stmt = conn.prepareStatement(query);
            stmt.setString(1, latitude);
            stmt.setString(2, longitude);
            stmt.setString(3, year);
            stmt.setString(4, month);
            stmt.setString(5, day);
            stmt.setString(6, gribFile);
            
            rs = stmt.executeQuery();
            
            if (rs.next()) {
                maxTemp = rs.getDouble("max_temp");
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
        System.out.println("Servlet-based application. Deploy to a web server to test.");
        System.out.println("Test cases would be HTTP requests to /api/temperature_for_location");
    }
}
