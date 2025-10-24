
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Map;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

public class Task64 {
    public static JsonObject fetchAndParseJSON(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            throw new IllegalArgumentException("URL cannot be null or empty");
        }
        
        HttpURLConnection connection = null;
        BufferedReader reader = null;
        
        try {
            // Validate URL format
            URL url = new URL(urlString);
            
            // Only allow HTTP and HTTPS protocols
            String protocol = url.getProtocol().toLowerCase();
            if (!protocol.equals("http") && !protocol.equals("https")) {
                throw new IllegalArgumentException("Only HTTP and HTTPS protocols are allowed");
            }
            
            connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(5000);
            connection.setReadTimeout(5000);
            connection.setRequestProperty("User-Agent", "Mozilla/5.0");
            
            int responseCode = connection.getResponseCode();
            if (responseCode != 200) {
                throw new RuntimeException("HTTP Error: " + responseCode);
            }
            
            reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            StringBuilder response = new StringBuilder();
            String line;
            
            while ((line = reader.readLine()) != null) {
                response.append(line);
            }
            
            JsonObject jsonObject = JsonParser.parseString(response.toString()).getAsJsonObject();
            return jsonObject;
            
        } catch (Exception e) {
            throw new RuntimeException("Error fetching or parsing JSON: " + e.getMessage(), e);
        } finally {
            try {
                if (reader != null) reader.close();
                if (connection != null) connection.disconnect();
            } catch (Exception e) {
                // Ignore cleanup errors
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases with mock JSON responses
        System.out.println("Test Case 1: Valid JSON URL");
        try {
            JsonObject result1 = fetchAndParseJSON("https://jsonplaceholder.typicode.com/todos/1");
            System.out.println("Result: " + result1);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 2: Another valid JSON URL");
        try {
            JsonObject result2 = fetchAndParseJSON("https://jsonplaceholder.typicode.com/users/1");
            System.out.println("Result: " + result2);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 3: Invalid URL format");
        try {
            JsonObject result3 = fetchAndParseJSON("not-a-valid-url");
            System.out.println("Result: " + result3);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 4: Empty URL");
        try {
            JsonObject result4 = fetchAndParseJSON("");
            System.out.println("Result: " + result4);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        System.out.println("\\nTest Case 5: Null URL");
        try {
            JsonObject result5 = fetchAndParseJSON(null);
            System.out.println("Result: " + result5);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Command line argument support
        if (args.length > 0) {
            System.out.println("\\nCommand line URL: " + args[0]);
            try {
                JsonObject result = fetchAndParseJSON(args[0]);
                System.out.println("Result: " + result);
            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
            }
        }
    }
}
