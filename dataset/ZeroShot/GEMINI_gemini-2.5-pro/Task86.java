import java.io.File;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;

public class Task86 {

    /**
     * Downloads an EXE file from a URL and executes it.
     * WARNING: This is an inherently dangerous operation. Executing code from
     * untrusted sources can compromise your system. This function should only be
     * used with URLs from trusted and verified sources. A production system
     * should include additional security measures like digital signature verification
     * and sandboxing.
     *
     * @param urlString The URL of the EXE file to download.
     */
    public static void downloadAndExecute(String urlString) {
        Path tempFile = null;
        try {
            // 1. URL Validation (basic)
            URL url = new URL(urlString);
            String protocol = url.getProtocol();
            if (!"https".equals(protocol) && !"http".equals(protocol)) {
                System.err.println("Error: URL must use HTTP or HTTPS protocol.");
                return;
            }

            // 2. Download the file
            System.out.println("Downloading from: " + urlString);
            HttpURLConnection httpConn = (HttpURLConnection) url.openConnection();
            // Set a reasonable timeout
            httpConn.setConnectTimeout(15000);
            httpConn.setReadTimeout(15000);
            int responseCode = httpConn.getResponseCode();

            if (responseCode == HttpURLConnection.HTTP_OK) {
                // Create a secure temporary file
                tempFile = Files.createTempFile("downloaded_", ".exe");
                
                try (InputStream inputStream = httpConn.getInputStream()) {
                    Files.copy(inputStream, tempFile, StandardCopyOption.REPLACE_EXISTING);
                }
                
                System.out.println("Downloaded to: " + tempFile.toString());

                // On non-Windows systems, we might need to set execute permissions.
                // On Windows, this is not necessary for .exe files.
                if (!System.getProperty("os.name").toLowerCase().contains("win")) {
                    tempFile.toFile().setExecutable(true);
                }

                // 3. Execute the file
                System.out.println("Executing: " + tempFile.toString());
                ProcessBuilder pb = new ProcessBuilder(tempFile.toString());
                pb.inheritIO();
                Process process = pb.start();
                
                // For GUI apps, you typically don't wait for them to close.
                // For console apps, you might want to wait:
                // int exitCode = process.waitFor();
                // System.out.println("Execution finished with exit code: " + exitCode);

            } else {
                System.err.println("Error: No file to download. Server replied HTTP code: " + responseCode);
            }
            httpConn.disconnect();
        } catch (Exception e) {
            System.err.println("An error occurred: " + e.getMessage());
        } finally {
            // 4. Cleanup
            if (tempFile != null) {
                try {
                    // Note: On Windows, deletion might fail if the launched process is still running.
                    Files.delete(tempFile);
                    System.out.println("Cleaned up temporary file: " + tempFile);
                } catch (java.io.IOException e) {
                    // If deletion fails, register it for deletion on JVM exit as a fallback.
                    System.err.println("Could not delete temp file immediately. Scheduling for deletion on exit.");
                    tempFile.toFile().deleteOnExit();
                }
            }
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: Valid Harmless EXE (PuTTY) ---");
        // WARNING: This URL points to a legitimate third-party application.
        // Running it is at your own risk. This test is for demonstration only.
        downloadAndExecute("https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe");

        System.out.println("\n--- Test Case 2: URL with 404 Not Found ---");
        downloadAndExecute("https://example.com/nonexistent/program.exe");

        System.out.println("\n--- Test Case 3: URL to a non-EXE file (text file) ---");
        // This will download a text file and attempt to execute it. 
        // The operating system should handle this gracefully (e.g., show an error).
        downloadAndExecute("https://raw.githubusercontent.com/octocat/Spoon-Knife/main/README.md");

        System.out.println("\n--- Test Case 4: Invalid URL format ---");
        downloadAndExecute("ftp://invalid-url-format.com/file.exe");
        
        System.out.println("\n--- Test Case 5: Another valid EXE (PSCP) ---");
        // PSCP is a command-line tool, it will run and exit quickly.
        downloadAndExecute("https://the.earth.li/~sgtatham/putty/latest/w64/pscp.exe");
    }
}