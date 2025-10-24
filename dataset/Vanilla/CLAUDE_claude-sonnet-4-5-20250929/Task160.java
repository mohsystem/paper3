
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Scanner;

public class Task160 {
    public static String makeHttpRequest(String urlString) {
        StringBuilder response = new StringBuilder();
        HttpURLConnection connection = null;
        
        try {
            URL url = new URL(urlString);
            connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(5000);
            connection.setReadTimeout(5000);
            
            int responseCode = connection.getResponseCode();
            
            BufferedReader reader;
            if (responseCode >= 200 && responseCode < 300) {
                reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            } else {
                reader = new BufferedReader(new InputStreamReader(connection.getErrorStream()));
            }
            
            String line;
            while ((line = reader.readLine()) != null) {
                response.append(line).append("\\n");
            }
            reader.close();
            
            return "Response Code: " + responseCode + "\\n" + response.toString();
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUrls = {
            "https://jsonplaceholder.typicode.com/posts/1",
            "https://api.github.com/users/github",
            "https://httpbin.org/get",
            "https://www.google.com",
            "https://example.com"
        };
        
        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": " + testUrls[i]);
            String result = makeHttpRequest(testUrls[i]);
            System.out.println(result.substring(0, Math.min(200, result.length())) + "...\\n");
        }
    }
}
