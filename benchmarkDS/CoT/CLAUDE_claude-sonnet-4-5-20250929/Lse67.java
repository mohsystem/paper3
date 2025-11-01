
import java.io.*;
import java.sql.*;
import java.nio.file.*;

public class Lse67 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final String[] ALLOWED_EXTENSIONS = {".jpg", ".jpeg", ".png", ".gif", ".bmp"};
    
    public static boolean storeImageInDatabase(String imagePath, String dbUrl, String dbUser, String dbPassword) {
        // Validate input
        if (imagePath == null || imagePath.trim().isEmpty()) {
            System.out.println("Invalid image path");
            return false;
        }
        
        File imageFile = new File(imagePath);
        
        // Validate file existence
        if (!imageFile.exists() || !imageFile.isFile()) {
            System.out.println("File does not exist or is not a valid file");
            return false;
        }
        
        // Validate file size
        if (imageFile.length() > MAX_FILE_SIZE) {
            System.out.println("File size exceeds maximum allowed size");
            return false;
        }
        
        // Validate file extension
        String fileName = imageFile.getName().toLowerCase();
        boolean validExtension = false;
        for (String ext : ALLOWED_EXTENSIONS) {
            if (fileName.endsWith(ext)) {
                validExtension = true;
                break;
            }
        }
        if (!validExtension) {
            System.out.println("Invalid file extension");
            return false;
        }
        
        // Use try-with-resources for automatic resource management
        try (Connection conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
             FileInputStream fis = new FileInputStream(imageFile);
             PreparedStatement pstmt = conn.prepareStatement(
                 "INSERT INTO images (filename, image_data, upload_date) VALUES (?, ?, ?)")) {
            
            // Use parameterized query to prevent SQL injection
            pstmt.setString(1, imageFile.getName());
            pstmt.setBinaryStream(2, fis, (int) imageFile.length());
            pstmt.setTimestamp(3, new Timestamp(System.currentTimeMillis()));
            
            int result = pstmt.executeUpdate();
            return result > 0;
            
        } catch (SQLException e) {
            System.out.println("Database error: " + e.getMessage());
            return false;
        } catch (IOException e) {
            System.out.println("File reading error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        String dbUrl = "jdbc:mysql://localhost:3306/imagedb";
        String dbUser = "root";
        String dbPassword = "password";
        
        // Test cases
        System.out.println("Test 1 (valid image): " + 
            storeImageInDatabase("test1.jpg", dbUrl, dbUser, dbPassword));
        
        System.out.println("Test 2 (valid image): " + 
            storeImageInDatabase("test2.png", dbUrl, dbUser, dbPassword));
        
        System.out.println("Test 3 (null path): " + 
            storeImageInDatabase(null, dbUrl, dbUser, dbPassword));
        
        System.out.println("Test 4 (non-existent file): " + 
            storeImageInDatabase("nonexistent.jpg", dbUrl, dbUser, dbPassword));
        
        System.out.println("Test 5 (invalid extension): " + 
            storeImageInDatabase("test.txt", dbUrl, dbUser, dbPassword));
    }
}
