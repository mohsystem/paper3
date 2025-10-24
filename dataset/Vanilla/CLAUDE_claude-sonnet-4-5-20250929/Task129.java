
import java.io.*;
import java.net.*;
import java.nio.file.*;
import java.util.*;

public class Task129 {
    
    public static String retrieveFileFromServer(String serverUrl, String fileName) {
        try {
            // Sanitize filename to prevent path traversal attacks
            String sanitizedFileName = sanitizeFileName(fileName);
            
            // Construct full URL
            String fullUrl = serverUrl + "/" + sanitizedFileName;
            
            URL url = new URL(fullUrl);
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(5000);
            connection.setReadTimeout(5000);
            
            int responseCode = connection.getResponseCode();
            
            if (responseCode == HttpURLConnection.HTTP_OK) {
                BufferedReader reader = new BufferedReader(
                    new InputStreamReader(connection.getInputStream())
                );
                StringBuilder content = new StringBuilder();
                String line;
                
                while ((line = reader.readLine()) != null) {
                    content.append(line).append("\\n");
                }
                reader.close();
                
                return content.toString();
            } else {
                return "Error: HTTP " + responseCode;
            }
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    private static String sanitizeFileName(String fileName) {
        // Remove path traversal sequences and dangerous characters
        if (fileName == null || fileName.isEmpty()) {
            return "default.txt";
        }
        
        // Remove directory traversal attempts
        fileName = fileName.replace("..", "");
        fileName = fileName.replace("/", "");
        fileName = fileName.replace("\\\\", "");\n        \n        // Keep only alphanumeric, dots, hyphens, and underscores\n        fileName = fileName.replaceAll("[^a-zA-Z0-9._-]", "");\n        \n        return fileName;\n    }\n    \n    public static void main(String[] args) {\n        // Test cases\n        String serverUrl = "http://example.com/files";\n        \n        System.out.println("Test Case 1: Valid file name");\n        System.out.println(retrieveFileFromServer(serverUrl, "document.txt"));\n        System.out.println();\n        \n        System.out.println("Test Case 2: File with numbers");\n        System.out.println(retrieveFileFromServer(serverUrl, "report_2024.pdf"));\n        System.out.println();\n        \n        System.out.println("Test Case 3: Attempting path traversal (sanitized)");\n        System.out.println(retrieveFileFromServer(serverUrl, "../../../etc/passwd"));\n        System.out.println();\n        \n        System.out.println("Test Case 4: File with special characters");\n        System.out.println(retrieveFileFromServer(serverUrl, "data-file_01.json"));\n        System.out.println();\n        \n        System.out.println("Test Case 5: Empty filename");\n        System.out.println(retrieveFileFromServer(serverUrl, ""));
        System.out.println();
    }
}
