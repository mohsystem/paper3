
import java.io.*;
import java.net.*;
import java.util.*;
import org.json.*;

public class Task64 {
    public static String readJsonFromUrl(String urlString) throws IOException {
        StringBuilder content = new StringBuilder();
        URL url = new URL(urlString);
        HttpURLConnection connection = (HttpURLConnection) url.openConnection();
        connection.setRequestMethod("GET");
        connection.setConnectTimeout(5000);
        connection.setReadTimeout(5000);
        
        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(connection.getInputStream()))) {
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line);
            }
        }
        return content.toString();
    }
    
    public static JSONObject parseJson(String url) {
        try {
            String jsonContent = readJsonFromUrl(url);
            return new JSONObject(jsonContent);
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test cases with publicly available JSON APIs
        String[] testUrls = {
            "https://jsonplaceholder.typicode.com/posts/1",
            "https://jsonplaceholder.typicode.com/users/1",
            "https://jsonplaceholder.typicode.com/comments/1",
            "https://jsonplaceholder.typicode.com/todos/1",
            "https://jsonplaceholder.typicode.com/albums/1"
        };
        
        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("URL: " + testUrls[i]);
            JSONObject result = parseJson(testUrls[i]);
            if (result != null) {
                System.out.println("JSON Object: " + result.toString(2));
            }
            System.out.println();
        }
    }
}
