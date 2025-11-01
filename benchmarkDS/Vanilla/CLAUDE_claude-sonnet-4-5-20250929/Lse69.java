
import java.io.*;
import java.util.Base64;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Lse69 {
    public static void uploadImage(String inputImagePath, String outputFilePath) {
        try {
            // Read the image file
            byte[] imageBytes = Files.readAllBytes(Paths.get(inputImagePath));
            
            // Encode the image using base64
            String encodedImage = Base64.getEncoder().encodeToString(imageBytes);
            
            // Write the encoded image to a file
            FileWriter writer = new FileWriter(outputFilePath);
            writer.write(encodedImage);
            writer.close();
            
            System.out.println("Image uploaded and encoded successfully!");
        } catch (IOException e) {
            System.err.println("Error uploading image: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        uploadImage("test_image1.jpg", "encoded_image1.txt");
        
        System.out.println("\\nTest Case 2:");
        uploadImage("test_image2.png", "encoded_image2.txt");
        
        System.out.println("\\nTest Case 3:");
        uploadImage("test_image3.gif", "encoded_image3.txt");
        
        System.out.println("\\nTest Case 4:");
        uploadImage("test_image4.bmp", "encoded_image4.txt");
        
        System.out.println("\\nTest Case 5:");
        uploadImage("test_image5.jpeg", "encoded_image5.txt");
    }
}
