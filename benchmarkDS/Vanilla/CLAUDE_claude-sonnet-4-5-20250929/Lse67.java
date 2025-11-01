
import java.io.*;
import java.sql.*;
import java.nio.file.*;

public class Lse67 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/imagedb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static boolean storeImageInDatabase(String imagePath) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            // Load MySQL JDBC Driver
            Class.forName("com.mysql.cj.jdbc.Driver");
            
            // Establish connection
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Read image file
            File imageFile = new File(imagePath);
            FileInputStream fis = new FileInputStream(imageFile);
            
            // Prepare SQL statement
            String sql = "INSERT INTO images (image_name, image_data) VALUES (?, ?)";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, imageFile.getName());
            pstmt.setBinaryStream(2, fis, (int) imageFile.length());
            
            // Execute update
            int result = pstmt.executeUpdate();
            fis.close();
            
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
    
    public static void main(String[] args) {
        // Test cases - Note: These require actual image files and MySQL setup
        System.out.println("Test Case 1: " + storeImageInDatabase("test1.jpg"));
        System.out.println("Test Case 2: " + storeImageInDatabase("test2.png"));
        System.out.println("Test Case 3: " + storeImageInDatabase("test3.gif"));
        System.out.println("Test Case 4: " + storeImageInDatabase("test4.bmp"));
        System.out.println("Test Case 5: " + storeImageInDatabase("test5.jpeg"));
    }
}
