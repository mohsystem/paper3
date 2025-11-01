
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse47 {
    private static final String IMAGES_FOLDER = "images";
    
    public static byte[] serveImage(String filename) {
        try {
            Path filePath = Paths.get(IMAGES_FOLDER, filename);
            File file = filePath.toFile();
            
            if (file.exists() && file.isFile()) {
                return Files.readAllBytes(filePath);
            } else {
                return null;
            }
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static boolean checkFileExists(String filename) {
        Path filePath = Paths.get(IMAGES_FOLDER, filename);
        File file = filePath.toFile();
        return file.exists() && file.isFile();
    }
    
    public static void main(String[] args) {
        // Create test images folder
        new File(IMAGES_FOLDER).mkdirs();
        
        // Test cases
        String[] testFiles = {
            "image1.jpg",
            "image2.png",
            "nonexistent.jpg",
            "test.gif",
            "photo.bmp"
        };
        
        System.out.println("Testing file existence and serving:");
        for (String filename : testFiles) {
            boolean exists = checkFileExists(filename);
            System.out.println("File: " + filename + " - Exists: " + exists);
            
            byte[] content = serveImage(filename);
            if (content != null) {
                System.out.println("  -> File served successfully, size: " + content.length + " bytes");
            } else {
                System.out.println("  -> File not found or error occurred");
            }
        }
    }
}
