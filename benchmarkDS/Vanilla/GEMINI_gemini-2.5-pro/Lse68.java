import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.nio.charset.StandardCharsets;

public class Lse68 {

    // Simulating a database table with a Map
    private static final Map<String, String> imageDatabase = new HashMap<>();

    /**
     * Simulates reading image data, converting it to base64, and inserting it into a database.
     * @param imageName The name of the image file.
     * @param imageData The raw byte data of the image.
     */
    public static void upload_image(String imageName, byte[] imageData) {
        if (imageName == null || imageName.isEmpty() || imageData == null) {
            System.out.println("Error: Image name and data cannot be null or empty.");
            return;
        }

        // Convert image byte data to Base64 string
        String base64ImageString = Base64.getEncoder().encodeToString(imageData);

        // Insert image name and base64 string into the database (simulated)
        // If the name already exists, its value is updated.
        imageDatabase.put(imageName, base64ImageString);

        System.out.println("Successfully uploaded and stored '" + imageName + "'.");
    }

    /**
     * Displays the current contents of the simulated database.
     */
    public static void displayDatabaseContents() {
        System.out.println("\n--- Simulated Database Contents ---");
        if (imageDatabase.isEmpty()) {
            System.out.println("Database is empty.");
        } else {
            for (Map.Entry<String, String> entry : imageDatabase.entrySet()) {
                System.out.println("Image Name: " + entry.getKey());
                // Printing a snippet of the base64 string for brevity
                String value = entry.getValue();
                String snippet = value.length() > 30 ? value.substring(0, 30) + "..." : value;
                System.out.println("Base64 Data (Snippet): " + snippet);
                System.out.println("---------------------------------");
            }
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: A simple text as image data
        upload_image("profile_pic.jpg", "This is my profile picture content.".getBytes(StandardCharsets.UTF_8));

        // Test Case 2: Another text file
        upload_image("document.png", "Important document data here.".getBytes(StandardCharsets.UTF_8));

        // Test Case 3: Empty image data
        upload_image("empty_image.gif", new byte[0]);

        // Test Case 4: Image with special characters
        upload_image("art-@#$.svg", "SVG content with <tags> & symbols!".getBytes(StandardCharsets.UTF_8));
        
        // Test Case 5: Overwriting an existing image
        upload_image("profile_pic.jpg", "This is my NEW profile picture.".getBytes(StandardCharsets.UTF_8));

        // Display the final state of the database
        displayDatabaseContents();
    }
}