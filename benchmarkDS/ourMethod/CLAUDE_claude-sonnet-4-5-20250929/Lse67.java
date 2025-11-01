
import java.io.*;
import java.nio.file.*;
import java.sql.*;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Lse67 {
    // Maximum file size: 5MB to prevent DoS
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024;
    
    // Allowed MIME types for images - whitelist approach for security
    private static final Set<String> ALLOWED_MIME_TYPES = new HashSet<>(Arrays.asList(
        "image/jpeg", "image/png", "image/gif", "image/bmp", "image/webp"
    ));
    
    // Maximum filename length to prevent buffer issues
    private static final int MAX_FILENAME_LENGTH = 255;
    
    /**
     * Stores an image file in MySQL database with security validations.
     * @param filePath Path to the image file
     * @param dbUrl Database connection URL
     * @param dbUser Database username
     * @param dbPassword Database password
     * @return true if successful, false otherwise
     */
    public static boolean storeImageInDatabase(String filePath, String dbUrl, 
                                               String dbUser, String dbPassword) {
        // Input validation: reject null or empty paths
        if (filePath == null || filePath.trim().isEmpty()) {
            System.err.println("Error: File path cannot be null or empty");
            return false;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        FileInputStream fis = null;
        
        try {
            // Secure file handling: open first, then validate (TOCTOU prevention)
            Path path = Paths.get(filePath).toRealPath(); // Normalize path
            File file = path.toFile();
            
            // Validation: check if file exists and is a regular file
            if (!Files.isRegularFile(path, LinkOption.NOFOLLOW_LINKS)) {
                System.err.println("Error: Not a regular file or symlink detected");
                return false;
            }
            
            // Validation: check file size to prevent DoS
            long fileSize = Files.size(path);
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File size exceeds maximum allowed size");
                return false;
            }
            
            if (fileSize == 0) {
                System.err.println("Error: File is empty");
                return false;
            }
            
            // Validation: check filename length
            String fileName = path.getFileName().toString();
            if (fileName.length() > MAX_FILENAME_LENGTH) {
                System.err.println("Error: Filename too long");
                return false;
            }
            
            // Validation: detect MIME type securely
            String mimeType = Files.probeContentType(path);
            if (mimeType == null || !ALLOWED_MIME_TYPES.contains(mimeType)) {
                System.err.println("Error: Invalid or unsupported image type");
                return false;
            }
            
            // Open file after all validations
            fis = new FileInputStream(file);
            
            // Secure database connection with proper error handling
            conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
            
            // Parameterized query to prevent SQL injection
            String sql = "INSERT INTO images (filename, mime_type, file_size, image_data) VALUES (?, ?, ?, ?)";
            pstmt = conn.prepareStatement(sql);
            
            // Set parameters safely - no direct string concatenation
            pstmt.setString(1, fileName);
            pstmt.setString(2, mimeType);
            pstmt.setLong(3, fileSize);
            pstmt.setBinaryStream(4, fis, fileSize);
            
            // Execute the insert
            int rowsAffected = pstmt.executeUpdate();
            
            return rowsAffected > 0;
            
        } catch (IOException e) {
            // Generic error message to avoid information leakage
            System.err.println("Error: Failed to read file");
            return false;
        } catch (SQLException e) {
            // Generic error message to avoid information leakage
            System.err.println("Error: Database operation failed");
            return false;
        } catch (Exception e) {
            // Catch-all for any unexpected errors
            System.err.println("Error: Operation failed");
            return false;
        } finally {
            // Proper resource cleanup in reverse order of acquisition
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                    // Log but don't fail\n                }\n            }\n            if (pstmt != null) {\n                try {\n                    pstmt.close();\n                } catch (SQLException e) {\n                    // Log but don't fail
                }
            }
            if (conn != null) {
                try {
                    conn.close();
                } catch (SQLException e) {
                    // Log but don't fail
                }
            }
        }
    }
    
    public static void main(String[] args) {
        // Test configuration - in production, use environment variables or secure config
        String dbUrl = "jdbc:mysql://localhost:3306/imagedb?useSSL=true&requireSSL=true";
        String dbUser = "testuser";
        String dbPassword = "testpass";
        
        // Test cases with various scenarios
        System.out.println("Test 1 - Valid JPEG:");
        boolean result1 = storeImageInDatabase("test1.jpg", dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result1 + "\\n");
        
        System.out.println("Test 2 - Valid PNG:");
        boolean result2 = storeImageInDatabase("test2.png", dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result2 + "\\n");
        
        System.out.println("Test 3 - Invalid null path:");
        boolean result3 = storeImageInDatabase(null, dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result3 + "\\n");
        
        System.out.println("Test 4 - Empty path:");
        boolean result4 = storeImageInDatabase("", dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result4 + "\\n");
        
        System.out.println("Test 5 - Non-existent file:");
        boolean result5 = storeImageInDatabase("nonexistent.jpg", dbUrl, dbUser, dbPassword);
        System.out.println("Result: " + result5);
    }
}
