
import java.io.*;
import java.nio.file.*;
import java.util.Base64;
import java.util.UUID;

public class Lse69 {
    private static final long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
    private static final String UPLOAD_DIR = "uploads";
    private static final String[] ALLOWED_TYPES = {".jpg", ".jpeg", ".png", ".gif"};
    
    public static String uploadImage(byte[] imageData, String originalFilename) {
        try {
            // Validate file size
            if (imageData == null || imageData.length == 0 || imageData.length > MAX_FILE_SIZE) {
                throw new IllegalArgumentException("Invalid file size");
            }
            
            // Validate file extension
            String extension = getFileExtension(originalFilename);
            if (!isAllowedType(extension)) {
                throw new IllegalArgumentException("File type not allowed");
            }
            
            // Create upload directory if not exists
            Path uploadPath = Paths.get(UPLOAD_DIR);
            if (!Files.exists(uploadPath)) {
                Files.createDirectories(uploadPath);
            }
            
            // Generate safe filename using UUID
            String safeFilename = UUID.randomUUID().toString() + "_encoded.txt";
            Path filePath = uploadPath.resolve(safeFilename);
            
            // Encode image to base64
            String encodedImage = Base64.getEncoder().encodeToString(imageData);
            
            // Write encoded image to file
            Files.write(filePath, encodedImage.getBytes(), StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            
            return filePath.toString();
        } catch (IOException e) {
            System.err.println("Error writing file: " + e.getMessage());
            return null;
        }
    }
    
    private static String getFileExtension(String filename) {
        if (filename == null || filename.isEmpty()) {
            return "";
        }
        int lastDot = filename.lastIndexOf('.');
        return lastDot > 0 ? filename.substring(lastDot).toLowerCase() : "";
    }
    
    private static boolean isAllowedType(String extension) {
        for (String allowed : ALLOWED_TYPES) {
            if (allowed.equals(extension)) {
                return true;
            }
        }
        return false;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid small image
        byte[] testImage1 = "test image data 1".getBytes();
        System.out.println("Test 1: " + uploadImage(testImage1, "test1.jpg"));
        
        // Test case 2: Valid PNG image
        byte[] testImage2 = "test image data 2".getBytes();
        System.out.println("Test 2: " + uploadImage(testImage2, "test2.png"));
        
        // Test case 3: Invalid file type
        byte[] testImage3 = "test image data 3".getBytes();
        System.out.println("Test 3: " + uploadImage(testImage3, "test3.exe"));
        
        // Test case 4: Null data
        System.out.println("Test 4: " + uploadImage(null, "test4.jpg"));
        
        // Test case 5: Empty array
        byte[] testImage5 = new byte[0];
        System.out.println("Test 5: " + uploadImage(testImage5, "test5.jpg"));
    }
}
