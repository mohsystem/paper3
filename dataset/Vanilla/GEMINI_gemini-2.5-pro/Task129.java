import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;

public class Task129 {

    /**
     * Retrieves the content of a file from a server.
     * @param baseUrl The base URL of the server (e.g., "https://example.com/files/").
     * @param fileName The name of the file to retrieve (e.g., "data.txt").
     * @return The content of the file as a String, or an error message if it fails.
     */
    public static String retrieveFileFromServer(String baseUrl, String fileName) {
        if (fileName == null || fileName.trim().isEmpty()) {
            return "Error: File name cannot be empty.";
        }
        
        StringBuilder content = new StringBuilder();
        HttpURLConnection connection = null;
        
        try {
            // Ensure the base URL ends with a slash if it doesn't already
            if (!baseUrl.endsWith("/")) {
                baseUrl += "/";
            }
            URL url = new URL(baseUrl + fileName);
            connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(5000); // 5 seconds
            connection.setReadTimeout(5000);    // 5 seconds

            int status = connection.getResponseCode();

            if (status >= 200 && status < 300) {
                try (BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream()))) {
                    String inputLine;
                    while ((inputLine = in.readLine()) != null) {
                        content.append(inputLine);
                        content.append(System.lineSeparator());
                    }
                }
            } else {
                return "Error: Failed to retrieve file. HTTP status code: " + status;
            }

        } catch (IOException e) {
            return "Error: " + e.getMessage();
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
        return content.toString();
    }

    public static void main(String[] args) {
        String baseUrl = "https://jsonplaceholder.typicode.com";
        String[] testFiles = {
            "todos/1",
            "posts/10",
            "users/5",
            "nonexistent/path", // This will cause a 404
            "" // Empty file name
        };

        for (int i = 0; i < testFiles.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": Retrieving '" + testFiles[i] + "' ---");
            String result = retrieveFileFromServer(baseUrl, testFiles[i]);
            System.out.println("Result:\n" + result);
            System.out.println("--- End Test Case " + (i + 1) + " ---\n");
        }
    }
}