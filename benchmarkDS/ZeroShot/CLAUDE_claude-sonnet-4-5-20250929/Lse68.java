
import java.io.*;
import java.nio.file.Files;
import java.sql.*;
import java.util.Base64;

public class Lse68 {
    private static final String DB_URL = "jdbc:sqlite:images.db";
    
    // Initialize database
    private static void initDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS images (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "name TEXT NOT NULL," +
                        "image_data TEXT NOT NULL)";
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database initialization error: " + e.getMessage());
        }
    }
    
    // Secure upload_image function
    public static boolean upload_image(String imagePath) {
        // Input validation
        if (imagePath == null || imagePath.trim().isEmpty()) {
            System.err.println("Invalid image path");
            return false;
        }
        
        File imageFile = new File(imagePath);
        
        // Validate file exists and is readable
        if (!imageFile.exists() || !imageFile.isFile() || !imageFile.canRead()) {
            System.err.println("File does not exist or is not readable");
            return false;
        }
        
        // Validate file size (max 5MB)
        if (imageFile.length() > 5 * 1024 * 1024) {
            System.err.println("File size exceeds 5MB limit");
            return false;
        }
        
        // Validate file extension
        String fileName = imageFile.getName().toLowerCase();
        if (!fileName.endsWith(".jpg") && !fileName.endsWith(".jpeg") && 
            !fileName.endsWith(".png") && !fileName.endsWith(".gif")) {
            System.err.println("Invalid file type. Only JPG, PNG, and GIF allowed");
            return false;
        }
        
        try {
            // Read image file
            byte[] imageBytes = Files.readAllBytes(imageFile.toPath());
            
            // Convert to base64
            String base64Image = Base64.getEncoder().encodeToString(imageBytes);
            
            // Insert into database using prepared statement (prevents SQL injection)
            try (Connection conn = DriverManager.getConnection(DB_URL);
                 PreparedStatement pstmt = conn.prepareStatement(
                     "INSERT INTO images (name, image_data) VALUES (?, ?)")) {
                
                pstmt.setString(1, imageFile.getName());
                pstmt.setString(2, base64Image);
                
                int rowsAffected = pstmt.executeUpdate();
                
                if (rowsAffected > 0) {
                    System.out.println("Image uploaded successfully: " + imageFile.getName());
                    return true;
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading image file: " + e.getMessage());
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
        
        return false;
    }
    
    public static void main(String[] args) {
        // Initialize database
        initDatabase();
        
        // Test cases
        System.out.println("Test Case 1: Valid image (create a test.jpg file first)");
        upload_image("test.jpg");
        
        System.out.println("\\nTest Case 2: Null path");
        upload_image(null);
        
        System.out.println("\\nTest Case 3: Empty path");
        upload_image("");
        
        System.out.println("\\nTest Case 4: Non-existent file");
        upload_image("nonexistent.jpg");
        
        System.out.println("\\nTest Case 5: Invalid file type");
        upload_image("test.txt");
    }
}
