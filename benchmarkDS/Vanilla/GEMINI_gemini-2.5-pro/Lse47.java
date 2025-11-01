import java.io.File;
import java.io.IOException;

// To run an actual web server, you would need a library like SparkJava.
// This example simulates the logic without starting a server.
// Add this to pom.xml for a real server:
// <dependency>
//     <groupId>com.sparkjava</groupId>
//     <artifactId>spark-core</artifactId>
//     <version>2.9.4</version>
// </dependency>
// import static spark.Spark.*;

public class Lse47 {

    private static final String IMAGE_DIR = "images";

    /**
     * This function simulates the core logic of a web handler.
     * It checks if a requested file exists in the designated image directory.
     *
     * @param filename The name of the file requested from the URL.
     * @return A string message indicating the result.
     */
    public static String handleFileRequest(String filename) {
        // Basic security check to prevent directory traversal
        if (filename == null || filename.isEmpty() || filename.contains("/") || filename.contains("\\") || filename.contains("..")) {
            return "HTTP 400 Bad Request: Invalid filename.";
        }

        File file = new File(IMAGE_DIR, filename);

        if (file.exists() && !file.isDirectory()) {
            return "HTTP 200 OK: Sending file '" + file.getPath() + "' to the browser.";
        } else {
            return "HTTP 404 Not Found: File '" + file.getPath() + "' does not exist.";
        }
    }

    /**
     * Sets up a dummy 'images' directory with some files for testing.
     */
    private static void setupDummyFiles() {
        try {
            File dir = new File(IMAGE_DIR);
            if (!dir.exists()) {
                dir.mkdirs();
            }
            new File(dir, "cat.jpg").createNewFile();
            new File(dir, "dog.png").createNewFile();
            new File(dir, "bird.gif").createNewFile();
        } catch (IOException e) {
            System.err.println("Failed to create dummy files: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // 1. Setup the environment
        setupDummyFiles();

        // 2. Run test cases
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Requesting an existing file
        System.out.println("Test 1 (cat.jpg):");
        System.out.println("  " + handleFileRequest("cat.jpg"));

        // Test Case 2: Requesting another existing file
        System.out.println("Test 2 (dog.png):");
        System.out.println("  " + handleFileRequest("dog.png"));

        // Test Case 3: Requesting a non-existent file
        System.out.println("Test 3 (lion.jpg):");
        System.out.println("  " + handleFileRequest("lion.jpg"));

        // Test Case 4: Attempting directory traversal (should be blocked)
        System.out.println("Test 4 (../somefile.txt):");
        System.out.println("  " + handleFileRequest("../somefile.txt"));
        
        // Test Case 5: Requesting an empty/invalid filename
        System.out.println("Test 5 (empty string):");
        System.out.println("  " + handleFileRequest(""));
        
        System.out.println("\n--- Test Cases Finished ---");
        System.out.println("\nNote: The code for an actual web server using a library like");
        System.out.println("SparkJava would use a decorator-like syntax: get(\"/images/:filename\", (req, res) -> { ... });");
    }
}