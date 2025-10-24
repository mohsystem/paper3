import java.io.InputStream;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;

// WARNING: Downloading and executing files from the internet can be extremely dangerous.
// Only run this with URLs from trusted sources.
public class Task86 {

    /**
     * Downloads an executable file from a URL and runs it.
     * @param urlStr The URL of the EXE file to download.
     * @param fileName The local filename to save the EXE as.
     */
    public static void downloadAndExecute(String urlStr, String fileName) {
        try {
            // Step 1: Download the file
            System.out.println("Downloading " + urlStr + " to " + fileName + "...");
            URL url = new URL(urlStr);
            Path targetPath = Paths.get(fileName).toAbsolutePath();

            // Use try-with-resources to ensure the stream is closed
            try (InputStream in = url.openStream()) {
                Files.copy(in, targetPath, StandardCopyOption.REPLACE_EXISTING);
            }
            System.out.println("Download complete.");

            // On some systems, we might need to make the file executable
            if (!System.getProperty("os.name").toLowerCase().startsWith("windows")) {
                targetPath.toFile().setExecutable(true);
            }

            // Step 2: Execute the downloaded file
            System.out.println("Executing " + targetPath + "...");
            ProcessBuilder processBuilder = new ProcessBuilder(targetPath.toString());
            Process process = processBuilder.start();
            
            // Wait for the process to complete and print its exit code
            int exitCode = process.waitFor();
            System.out.println("'" + fileName + "' finished with exit code: " + exitCode);

        } catch (Exception e) {
            System.err.println("An error occurred: " + e.getMessage());
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        // Using a URL for a safe, well-known executable (PuTTY)
        String fileUrl = "https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe";
        
        System.out.println("--- Starting 5 Test Cases ---");

        System.out.println("\n--- Test Case 1 ---");
        downloadAndExecute(fileUrl, "putty_java_1.exe");

        System.out.println("\n--- Test Case 2 ---");
        downloadAndExecute(fileUrl, "putty_java_2.exe");

        System.out.println("\n--- Test Case 3 ---");
        downloadAndExecute(fileUrl, "putty_java_3.exe");

        System.out.println("\n--- Test Case 4 ---");
        downloadAndExecute(fileUrl, "putty_java_4.exe");

        System.out.println("\n--- Test Case 5 ---");
        downloadAndExecute(fileUrl, "putty_java_5.exe");
        
        System.out.println("\n--- All tests completed ---");
    }
}