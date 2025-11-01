
import com.sun.net.httpserver.*;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.*;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

class Lse47 {
    private static final String IMAGES_DIR = "images";
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList(".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp")
    );

    public static byte[] serveImage(String filename) {
        try {
            // Sanitize filename - remove any path traversal attempts
            String sanitizedFilename = new File(filename).getName();
            
            // Validate file extension
            String lowerFilename = sanitizedFilename.toLowerCase();
            boolean validExtension = ALLOWED_EXTENSIONS.stream()
                .anyMatch(ext -> lowerFilename.endsWith(ext));
            
            if (!validExtension) {
                return null;
            }
            
            // Construct safe path
            Path imagePath = Paths.get(IMAGES_DIR, sanitizedFilename).normalize();
            Path baseDir = Paths.get(IMAGES_DIR).toAbsolutePath().normalize();
            
            // Ensure the resolved path is within images directory
            if (!imagePath.toAbsolutePath().normalize().startsWith(baseDir)) {
                return null;
            }
            
            // Check if file exists and is a regular file
            if (Files.exists(imagePath) && Files.isRegularFile(imagePath)) {
                return Files.readAllBytes(imagePath);
            }
            
            return null;
        } catch (Exception e) {
            return null;
        }
    }

    public static void main(String[] args) throws IOException {
        // Create images directory if not exists
        new File(IMAGES_DIR).mkdirs();
        
        // Create test images
        String[] testFiles = {"test1.jpg", "test2.png", "test3.gif", "test4.webp", "test5.bmp"};
        for (String testFile : testFiles) {
            Path testPath = Paths.get(IMAGES_DIR, testFile);
            if (!Files.exists(testPath)) {
                Files.write(testPath, ("Test image content for " + testFile).getBytes());
            }
        }
        
        // Test cases
        System.out.println("Test 1 - Valid image (test1.jpg): " + 
            (serveImage("test1.jpg") != null ? "SUCCESS" : "FAILED"));
        
        System.out.println("Test 2 - Valid image (test2.png): " + 
            (serveImage("test2.png") != null ? "SUCCESS" : "FAILED"));
        
        System.out.println("Test 3 - Path traversal attempt (../test.jpg): " + 
            (serveImage("../test.jpg") == null ? "SUCCESS (Blocked)" : "FAILED"));
        
        System.out.println("Test 4 - Invalid extension (test.txt): " + 
            (serveImage("test.txt") == null ? "SUCCESS (Blocked)" : "FAILED"));
        
        System.out.println("Test 5 - Non-existent file (nonexistent.jpg): " + 
            (serveImage("nonexistent.jpg") == null ? "SUCCESS (Not Found)" : "FAILED"));
    }
}
