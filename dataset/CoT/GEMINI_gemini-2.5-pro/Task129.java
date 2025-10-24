import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task129 {

    /**
     * Retrieves a file from a server and saves it locally.
     *
     * @param baseUrl  The base URL of the server directory (e.g., "https://example.com/files/").
     * @param fileName The name of the file to retrieve. This must be a simple filename without any path elements.
     * @param saveDir  The local directory where the file will be saved.
     * @return true if the file was downloaded successfully, false otherwise.
     */
    public static boolean retrieveFile(String baseUrl, String fileName, String saveDir) {
        // 1. Security: Input Validation to prevent Path Traversal
        if (fileName == null || fileName.isEmpty() || !new File(fileName).getName().equals(fileName)) {
            System.err.println("Error: Invalid file name. Must be a simple name without path separators.");
            return false;
        }

        HttpURLConnection connection = null;
        try {
            // 2. Security: Construct URL from a fixed base and validated filename to prevent SSRF
            String urlString = baseUrl + (baseUrl.endsWith("/") ? "" : "/") + fileName;
            URL url = new URL(urlString);

            connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            
            // 3. Security: Set timeouts to prevent Denial of Service from slow servers
            connection.setConnectTimeout(5000); // 5 seconds
            connection.setReadTimeout(5000);    // 5 seconds

            int responseCode = connection.getResponseCode();

            if (responseCode == HttpURLConnection.HTTP_OK) {
                // Securely construct the save path
                Path savePath = Paths.get(saveDir, fileName);

                // Ensure parent directory exists
                Files.createDirectories(savePath.getParent());

                // Use try-with-resources for automatic stream closing
                try (InputStream inputStream = connection.getInputStream();
                     FileOutputStream outputStream = new FileOutputStream(savePath.toFile())) {
                    
                    byte[] buffer = new byte[4096];
                    int bytesRead;
                    while ((bytesRead = inputStream.read(buffer)) != -1) {
                        outputStream.write(buffer, 0, bytesRead);
                    }
                }
                System.out.println("File downloaded successfully to: " + savePath);
                return true;
            } else {
                System.err.println("Server responded with code: " + responseCode + " for URL: " + urlString);
                return false;
            }
        } catch (Exception e) {
            System.err.println("An error occurred during download: " + e.getMessage());
            return false;
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    public static void main(String[] args) {
        // Base URL of a public repository for testing purposes
        String baseUrl = "https://raw.githubusercontent.com/gittar-und-fuchtel/md5-python-java-c/master/";
        // Local directory to save downloaded files
        String saveDir = "downloads_java";

        // Create the directory if it doesn't exist
        new File(saveDir).mkdirs();

        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Valid file
        System.out.println("\n[Test 1: Valid File]");
        retrieveFile(baseUrl, "README.md", saveDir);

        // Test Case 2: File not found
        System.out.println("\n[Test 2: Non-existent File]");
        retrieveFile(baseUrl, "non_existent_file.txt", saveDir);

        // Test Case 3: Path Traversal attempt (should be blocked)
        System.out.println("\n[Test 3: Path Traversal Attempt]");
        retrieveFile(baseUrl, "../some_other_file.txt", saveDir);

        // Test Case 4: Absolute path attempt (should be blocked)
        System.out.println("\n[Test 4: Absolute Path Attempt]");
        retrieveFile(baseUrl, "/etc/passwd", saveDir);
        
        // Test Case 5: Empty filename (should be blocked)
        System.out.println("\n[Test 5: Empty Filename]");
        retrieveFile(baseUrl, "", saveDir);
    }
}