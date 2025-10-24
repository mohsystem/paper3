
import java.io.*;
import java.net.*;
import java.nio.file.*;

public class Task86 {
    public static String downloadAndExecute(String urlString) {
        try {
            // Create URL object
            URL url = new URL(urlString);
            
            // Generate temporary file name
            String fileName = "downloaded_" + System.currentTimeMillis() + ".exe";
            Path tempFile = Paths.get(System.getProperty("java.io.tmpdir"), fileName);
            
            // Download file
            try (InputStream in = url.openStream()) {
                Files.copy(in, tempFile, StandardCopyOption.REPLACE_EXISTING);
            }
            
            // Make file executable (for Unix-like systems)
            tempFile.toFile().setExecutable(true);
            
            // Execute the file
            ProcessBuilder processBuilder = new ProcessBuilder(tempFile.toString());
            Process process = processBuilder.start();
            
            // Wait for process to complete (with timeout)
            boolean finished = process.waitFor(30, java.util.concurrent.TimeUnit.SECONDS);
            
            String result = "File downloaded to: " + tempFile.toString() + 
                          "\\nExecution " + (finished ? "completed" : "timed out");
            
            return result;
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== SECURITY WARNING: This program downloads and executes files ===\\n");
        
        // Test cases - Using safe/mock URLs
        String[] testUrls = {
            "https://example.com/safe-program.exe",
            "https://example.com/test-app.exe",
            "https://example.com/utility.exe",
            "https://example.com/installer.exe",
            "https://example.com/application.exe"
        };
        
        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("URL: " + testUrls[i]);
            System.out.println("Result: " + downloadAndExecute(testUrls[i]));
            System.out.println("---");
        }
    }
}
