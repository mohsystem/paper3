import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.time.Duration;

public class Task86 {

    /**
     * Downloads an executable file from a given URL and executes it.
     * WARNING: This function is inherently dangerous as it downloads and executes
     * code from the internet. Only use with URLs from trusted sources.
     *
     * @param urlString The URL of the .exe file to download.
     * @return 0 on success, non-zero on failure.
     */
    public static int downloadAndExecute(String urlString) {
        // Rule #3: Ensure all input is validated
        if (urlString == null || urlString.trim().isEmpty()) {
            System.err.println("Error: URL is null or empty.");
            return 1;
        }
        if (!urlString.toLowerCase().startsWith("https://")) {
            System.err.println("Error: URL must use HTTPS.");
            return 2;
        }
        if (!urlString.toLowerCase().endsWith(".exe")) {
            System.err.println("Error: URL must point to an .exe file.");
            return 3;
        }

        URI uri;
        try {
            uri = new URI(urlString);
        } catch (URISyntaxException e) {
            System.err.println("Error: Invalid URL syntax: " + e.getMessage());
            return 4;
        }

        Path tempFilePath = null;
        try {
            // Rule #7: Use a safe, temporary directory for the downloaded file.
            String tempDir = System.getProperty("java.io.tmpdir");
            tempFilePath = Paths.get(tempDir, "downloaded_program.exe");

            System.out.println("Downloading from " + urlString + " to " + tempFilePath);

            // Rules #1 & #2: HttpClient by default performs SSL/TLS validation and hostname verification.
            HttpClient client = HttpClient.newBuilder()
                    .connectTimeout(Duration.ofSeconds(20))
                    .followRedirects(HttpClient.Redirect.NORMAL)
                    .build();

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(uri)
                    .GET()
                    .build();

            // Rule #9: Handle exceptions properly.
            HttpResponse<InputStream> response = client.send(request, HttpResponse.BodyHandlers.ofInputStream());

            if (response.statusCode() != 200) {
                System.err.println("Error: Failed to download file. Status code: " + response.statusCode());
                return 5;
            }

            // Rule #5: Use try-with-resources to prevent resource leaks and manage buffers safely.
            try (InputStream in = response.body()) {
                Files.copy(in, tempFilePath, StandardCopyOption.REPLACE_EXISTING);
            }

            System.out.println("Download complete. Executing " + tempFilePath);

            // Rule #4: Use ProcessBuilder for safer process creation.
            ProcessBuilder pb = new ProcessBuilder(tempFilePath.toString());
            pb.inheritIO(); // Show output of the executed program
            Process process = pb.start();

            int exitCode = process.waitFor();
            System.out.println("Program finished with exit code: " + exitCode);

            return 0; // Success

        } catch (IOException | InterruptedException e) {
            System.err.println("An error occurred: " + e.getMessage());
            return 6;
        } finally {
            // Rule #8: Cleanup the temporary file to mitigate risks.
            if (tempFilePath != null) {
                try {
                    Files.deleteIfExists(tempFilePath);
                    System.out.println("Cleaned up temporary file: " + tempFilePath);
                } catch (IOException e) {
                    System.err.println("Error cleaning up temporary file: " + e.getMessage());
                }
            }
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");
        // NOTE: The "successful" test case uses a real URL to a known safe program (PuTTY).
        
        // Test Case 1: Valid URL
        System.out.println("\n[Test Case 1: Valid URL]");
        String validUrl = "https://www.chiark.greenend.org.uk/~sgtatham/putty/latest/w64/putty.exe";
        int result1 = downloadAndExecute(validUrl);
        System.out.println("Test Case 1 Result: " + (result1 == 0 ? "Success (as expected)" : "Failure (Code: " + result1 + ")"));

        // Test Case 2: Invalid scheme (HTTP)
        System.out.println("\n[Test Case 2: HTTP URL]");
        String httpUrl = "http://www.chiark.greenend.org.uk/~sgtatham/putty/latest/w64/putty.exe";
        int result2 = downloadAndExecute(httpUrl);
        System.out.println("Test Case 2 Result: " + (result2 != 0 ? "Failure (as expected)" : "Success (unexpected)"));

        // Test Case 3: Not an EXE file
        System.out.println("\n[Test Case 3: Non-EXE URL]");
        String nonExeUrl = "https://www.google.com/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png";
        int result3 = downloadAndExecute(nonExeUrl);
        System.out.println("Test Case 3 Result: " + (result3 != 0 ? "Failure (as expected)" : "Success (unexpected)"));

        // Test Case 4: Not Found URL
        System.out.println("\n[Test Case 4: Not Found URL]");
        String notFoundUrl = "https://example.com/nonexistent/program.exe";
        int result4 = downloadAndExecute(notFoundUrl);
        System.out.println("Test Case 4 Result: " + (result4 != 0 ? "Failure (as expected)" : "Success (unexpected)"));

        // Test Case 5: Null input
        System.out.println("\n[Test Case 5: Null URL]");
        int result5 = downloadAndExecute(null);
        System.out.println("Test Case 5 Result: " + (result5 != 0 ? "Failure (as expected)" : "Success (unexpected)"));

        System.out.println("\n--- Test Cases Finished ---");
    }
}