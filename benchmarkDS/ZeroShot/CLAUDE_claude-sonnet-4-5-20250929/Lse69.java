
import java.io.*;
import java.util.Base64;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Lse69 {
    public static String upload_image(String inputImagePath, String outputFilePath) {
        try {
            // Read the image file
            byte[] imageBytes = Files.readAllBytes(Paths.get(inputImagePath));
            
            // Encode the image using base64
            String encodedImage = Base64.getEncoder().encodeToString(imageBytes);
            
            // Write the encoded image to a file
            try (FileWriter writer = new FileWriter(outputFilePath)) {
                writer.write(encodedImage);
            }
            
            return "Image uploaded and encoded successfully";
        } catch (IOException e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        System.out.println(upload_image("test1.jpg", "encoded1.txt"));
        
        // Test case 2
        System.out.println(upload_image("test2.png", "encoded2.txt"));
        
        // Test case 3
        System.out.println(upload_image("test3.gif", "encoded3.txt"));
        
        // Test case 4
        System.out.println(upload_image("test4.bmp", "encoded4.txt"));
        
        // Test case 5
        System.out.println(upload_image("test5.jpeg", "encoded5.txt"));
    }
}
