
import java.io.*;
import java.nio.file.*;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;
import java.util.regex.Pattern;

public class Lse68 {
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB limit
    private static final Pattern ALLOWED_EXTENSIONS = Pattern.compile(".*\\\\.(jpg|jpeg|png|gif)$", Pattern.CASE_INSENSITIVE);
    private static final String ALLOWED_MIME_TYPES = "image/jpeg|image/png|image/gif";
    
    // Secure database connection with proper configuration
    private static Connection getSecureConnection() throws SQLException {
        String url = System.getenv("DB_URL");
        String user = System.getenv("DB_USER");
        String password = System.getenv("DB_PASSWORD");
        
        if (url == null || user == null || password == null) {
            throw new SQLException("Database credentials not configured in environment");
        }
        
        return DriverManager.getConnection(url, user, password);
    }
    
    // Validates file is actually an image by checking magic bytes
    private static boolean isValidImageFile(byte[] data) {
        if (data == null || data.length < 4) return false;
        
        // Check JPEG magic bytes
        if (data.length >= 2 && data[0] == (byte)0xFF && data[1] == (byte)0xD8) return true;
        // Check PNG magic bytes
        if (data.length >= 8 && data[0] == (byte)0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) return true;
        // Check GIF magic bytes
        if (data.length >= 6 && data[0] == 0x47 && data[1] == 0x49 && data[2] == 0x46) return true;
        
        return false;
    }
    
    public static String uploadImage(String filename, byte[] imageData) {
        // Input validation: check filename is not null or empty
        if (filename == null || filename.trim().isEmpty()) {
            return "Error: Invalid filename";
        }
        
        // Input validation: check image data is not null
        if (imageData == null || imageData.length == 0) {
            return "Error: No image data provided";
        }
        
        // Validate file size to prevent DOS attacks
        if (imageData.length > MAX_FILE_SIZE) {
            return "Error: File size exceeds limit";
        }
        
        // Sanitize filename: remove path traversal attempts and validate extension
        String sanitizedFilename = Paths.get(filename).getFileName().toString();
        if (!ALLOWED_EXTENSIONS.matcher(sanitizedFilename).matches()) {
            return "Error: Invalid file extension";
        }
        
        // Validate actual file content by magic bytes
        if (!isValidImageFile(imageData)) {
            return "Error: Invalid image format";
        }
        
        // Convert to base64 using secure encoding
        String base64Image;
        try {
            base64Image = Base64.getEncoder().encodeToString(imageData);
        } catch (Exception e) {
            return "Error: Failed to encode image";
        }
        
        // Insert into database using parameterized query to prevent SQL injection
        try (Connection conn = getSecureConnection();
             PreparedStatement pstmt = conn.prepareStatement(
                 "INSERT INTO images (name, base64_data) VALUES (?, ?)")) {
            
            // Use parameterized queries - never concatenate user input
            pstmt.setString(1, sanitizedFilename);
            pstmt.setString(2, base64Image);
            
            int rowsAffected = pstmt.executeUpdate();
            
            if (rowsAffected > 0) {
                return "Success: Image uploaded";
            } else {
                return "Error: Failed to insert image";
            }
            
        } catch (SQLException e) {
            // Generic error message to user, detailed logging would go to secure logs
            return "Error: Database operation failed";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid small JPEG image (simplified magic bytes)
        byte[] validJpeg = new byte[100];
        validJpeg[0] = (byte)0xFF;
        validJpeg[1] = (byte)0xD8;
        System.out.println("Test 1: " + uploadImage("test1.jpg", validJpeg));
        
        // Test case 2: Valid PNG image
        byte[] validPng = new byte[100];
        validPng[0] = (byte)0x89;
        validPng[1] = 0x50;
        validPng[2] = 0x4E;
        validPng[3] = 0x47;
        System.out.println("Test 2: " + uploadImage("test2.png", validPng));
        
        // Test case 3: Invalid extension
        System.out.println("Test 3: " + uploadImage("test3.exe", validJpeg));
        
        // Test case 4: Path traversal attempt
        System.out.println("Test 4: " + uploadImage("../../etc/passwd.jpg", validJpeg));
        
        // Test case 5: Null input
        System.out.println("Test 5: " + uploadImage(null, null));
    }
}
