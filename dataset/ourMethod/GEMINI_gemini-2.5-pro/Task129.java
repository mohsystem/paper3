import java.io.IOException;
import java.net.URI;
import java.net.URLEncoder;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.time.Duration;
import java.util.UUID;
import java.util.regex.Pattern;

public class Task129 {

    // Regex to validate filename. Allows alphanumeric, underscore, dot, hyphen.
    private static final Pattern FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]+$");
    private static final String DOWNLOAD_DIR = "downloads_java";

    /**
     * Validates a filename against a secure pattern.
     * @param filename The filename to validate.
     * @return true if the filename is valid, false otherwise.
     */
    private static boolean isValidFilename(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            return false;
        }
        return FILENAME_PATTERN.matcher(filename).matches();
    }

    /**
     * Downloads a file from a base URL given a filename.
     * @param baseUrl The base URL of the server.
     * @param filename The name of the file to download.
     * @return A Path object to the downloaded file, or null on failure.
     */
    public static Path downloadFile(String baseUrl, String filename) {
        if (!isValidFilename(filename)) {
            System.err.println("Error: Invalid filename provided: " + filename);
            return null;
        }

        Path downloadDirPath = Paths.get(DOWNLOAD_DIR);
        try {
            Files.createDirectories(downloadDirPath);
        } catch (IOException e) {
            System.err.println("Error: Could not create download directory: " + e.getMessage());
            return null;
        }

        Path destinationPath = downloadDirPath.resolve(filename).normalize();
        if (!destinationPath.startsWith(downloadDirPath)) {
             System.err.println("Error: Directory traversal attempt detected.");
             return null;
        }

        Path tempFile = null;
        try {
            String encodedFilename = URLEncoder.encode(filename, StandardCharsets.UTF_8);
            URI fileUri = URI.create(baseUrl + encodedFilename);

            HttpClient client = HttpClient.newBuilder()
                    .version(HttpClient.Version.HTTP_2)
                    .followRedirects(HttpClient.Redirect.NORMAL)
                    .connectTimeout(Duration.ofSeconds(20))
                    .build();

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(fileUri)
                    .GET()
                    .build();

            // Create a temporary file to write to
            String tempSuffix = ".tmp";
            tempFile = Files.createTempFile(downloadDirPath, "download_", tempSuffix);

            HttpResponse<Path> response = client.send(request, HttpResponse.BodyHandlers.ofFile(tempFile));

            if (response.statusCode() == 200) {
                // Atomically move the temporary file to the final destination
                Files.move(tempFile, destinationPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
                System.out.println("Successfully downloaded: " + filename + " to " + destinationPath);
                return destinationPath;
            } else {
                System.err.println("Error: Failed to download file. Status code: " + response.statusCode());
                Files.delete(tempFile); // Clean up temp file
                return null;
            }
        } catch (IOException | InterruptedException e) {
            System.err.println("Error during download: " + e.getMessage());
            // Clean up temporary file on error
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException cleanupEx) {
                    System.err.println("Error cleaning up temporary file: " + cleanupEx.getMessage());
                }
            }
            return null;
        }
    }

    public static void main(String[] args) {
        String baseUrl = "https://raw.githubusercontent.com/git/git/master/";

        System.out.println("--- Test Case 1: Valid File ---");
        downloadFile(baseUrl, "README.md");

        System.out.println("\n--- Test Case 2: Non-existent File ---");
        downloadFile(baseUrl, "NON_EXISTENT_FILE.txt");

        System.out.println("\n--- Test Case 3: Path Traversal Attempt ---");
        downloadFile(baseUrl, "../../etc/passwd");
        
        System.out.println("\n--- Test Case 4: Invalid Characters ---");
        downloadFile(baseUrl, "file?name=value");

        System.out.println("\n--- Test Case 5: Empty Filename ---");
        downloadFile(baseUrl, "");
    }
}