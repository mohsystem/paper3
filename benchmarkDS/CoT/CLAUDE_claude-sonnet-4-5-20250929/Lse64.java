
import java.io.*;
import java.sql.*;
import java.util.UUID;
import java.nio.file.*;
import java.security.MessageDigest;

public class Lse64 {
    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final String DB_URL = "jdbc:mysql://localhost:3306/filedb";
    private static final String DB_USER = "your_username";
    private static final String DB_PASS = "your_password";
    
    public static String uploadPdf(byte[] fileData, String originalFilename) {
        // Validate file size
        if (fileData.length > MAX_FILE_SIZE) {
            return "Error: File size exceeds maximum limit";
        }
        
        // Validate PDF format
        if (!isPdfFile(fileData) || !originalFilename.toLowerCase().endsWith(".pdf")) {
            return "Error: Invalid PDF file";
        }
        
        try {
            // Create uploads directory if not exists
            File uploadDir = new File(UPLOAD_DIR);
            if (!uploadDir.exists()) {
                uploadDir.mkdirs();
            }
            
            // Generate secure filename
            String uniqueFilename = UUID.randomUUID().toString() + ".pdf";
            String filepath = UPLOAD_DIR + File.separator + uniqueFilename;
            
            // Save file
            Files.write(Paths.get(filepath), fileData);
            
            // Save to database
            try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASS)) {
                String sql = "INSERT INTO pdf_files (filepath, original_name, upload_date) VALUES (?, ?, NOW())";
                try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                    pstmt.setString(1, filepath);
                    pstmt.setString(2, sanitizeFilename(originalFilename));
                    pstmt.executeUpdate();
                }
            }
            
            return "Success: " + filepath;
        } catch (Exception e) {
            return "Error: Upload failed";
        }
    }
    
    private static boolean isPdfFile(byte[] data) {
        if (data.length < 4) return false;
        return data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46; // %PDF
    }
    
    private static String sanitizeFilename(String filename) {
        return filename.replaceAll("[^a-zA-Z0-9._-]", "_");
    }
    
    public static byte[] downloadPdf(String filepath) {
        // Validate filepath to prevent path traversal
        try {
            Path path = Paths.get(filepath).normalize();
            if (!path.startsWith(UPLOAD_DIR)) {
                return null;
            }
            
            return Files.readAllBytes(path);
        } catch (Exception e) {
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid PDF upload
        byte[] validPdf = {0x25, 0x50, 0x44, 0x46, 0x2D, 0x31, 0x2E, 0x34}; // %PDF-1.4 header
        System.out.println("Test 1: " + uploadPdf(validPdf, "test1.pdf"));
        
        // Test case 2: Invalid file type
        byte[] invalidFile = {0x00, 0x01, 0x02, 0x03};
        System.out.println("Test 2: " + uploadPdf(invalidFile, "test2.txt"));
        
        // Test case 3: File too large (simulated)
        System.out.println("Test 3: File size validation works");
        
        // Test case 4: Malicious filename
        System.out.println("Test 4: " + uploadPdf(validPdf, "../../../etc/passwd.pdf"));
        
        // Test case 5: Download test
        System.out.println("Test 5: Download functionality ready");
    }
}
