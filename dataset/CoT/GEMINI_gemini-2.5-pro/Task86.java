import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;

public class Task86 {

    /**
     * WARNING: This function downloads and executes a file from the internet.
     * This is an extremely dangerous operation and can expose your system to
     * malware and other security risks. Only use this with URLs from sources
     * you completely trust. The downloaded executable will have the same
     * permissions as the user running this Java program.
     *
     * @param url The URL of the EXE file to download and execute.
     * @return true if the process was started successfully, false otherwise.
     */
    public static boolean downloadAndExecute(String url) {
        // Use a modern HttpClient
        HttpClient client = HttpClient.newHttpClient();
        Path tempFile = null;

        try {
            // 1. Create a secure temporary file.
            // The file will be created in the system's temporary directory
            // with a secure, random name. We add a prefix and suffix for clarity.
            tempFile = Files.createTempFile("downloaded-", ".exe");

            // 2. Download the file.
            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(url))
                    .build();

            System.out.println("Downloading from " + url + " to " + tempFile);
            HttpResponse<InputStream> response = client.send(request, HttpResponse.BodyHandlers.ofInputStream());

            // Check if the download was successful (HTTP 2xx status code).
            if (response.statusCode() < 200 || response.statusCode() >= 300) {
                System.err.println("Download failed with HTTP status code: " + response.statusCode());
                return false;
            }

            // Save the downloaded content to the temporary file.
            try (InputStream inputStream = response.body()) {
                Files.copy(inputStream, tempFile, StandardCopyOption.REPLACE_EXISTING);
            }

            // Make the file executable (important on non-Windows systems, good practice anyway).
            if (!System.getProperty("os.name").toLowerCase().startsWith("windows")) {
                tempFile.toFile().setExecutable(true);
            }

            // 3. Execute the downloaded file.
            System.out.println("Executing " + tempFile);
            ProcessBuilder processBuilder = new ProcessBuilder(tempFile.toAbsolutePath().toString());
            Process process = processBuilder.start();

            // The program will run asynchronously.
            System.out.println("Process started successfully. PID: " + process.pid());

            return true;

        } catch (IOException | InterruptedException | IllegalArgumentException e) {
            // Catch various exceptions: network errors, file errors, invalid URLs, etc.
            System.err.println("An error occurred: " + e.getMessage());
            return false;
        } finally {
            // 4. Cleanup: Delete the temporary file.
            if (tempFile != null) {
                try {
                    Files.delete(tempFile);
                    System.out.println("Cleaned up temporary file: " + tempFile);
                } catch (IOException e) {
                    System.err.println("Failed to delete temporary file: " + tempFile);
                }
            }
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: A well-known, safe 64-bit executable (PuTTY)
        System.out.println("\n--- Test Case 1: Valid 64-bit EXE (PuTTY) ---");
        downloadAndExecute("https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe");

        // Test Case 2: A well-known, safe portable executable (Process Explorer)
        System.out.println("\n--- Test Case 2: Valid portable EXE (Process Explorer) ---");
        downloadAndExecute("https://live.sysinternals.com/procexp.exe");

        // Test Case 3: An invalid URL that will fail to download
        System.out.println("\n--- Test Case 3: Invalid URL ---");
        downloadAndExecute("https://this-is-a-fake-url-12345.com/nonexistent.exe");
        
        // Test Case 4: A valid URL to a non-executable file
        System.out.println("\n--- Test Case 4: Valid URL, but not an EXE ---");
        downloadAndExecute("https://www.google.com/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png");

        // Test Case 5: A well-known, safe 32-bit executable (PuTTY)
        System.out.println("\n--- Test Case 5: Valid 32-bit EXE (PuTTY) ---");
        downloadAndExecute("https://the.earth.li/~sgtatham/putty/latest/w32/putty.exe");
    }
}