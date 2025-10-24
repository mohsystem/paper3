
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task76 {
    public static String getPageTitle(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            return "Invalid URL";
        }
        
        // Validate HTTPS URL
        if (!urlString.toLowerCase().startsWith("https://")) {
            return "Only HTTPS URLs are allowed";
        }
        
        HttpURLConnection connection = null;
        BufferedReader reader = null;
        
        try {
            URL url = new URL(urlString);
            connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(5000);
            connection.setReadTimeout(5000);
            connection.setRequestProperty("User-Agent", "Mozilla/5.0");
            
            int responseCode = connection.getResponseCode();
            if (responseCode != 200) {
                return "Failed to retrieve page. Response code: " + responseCode;
            }
            
            reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            StringBuilder content = new StringBuilder();
            String line;
            
            while ((line = reader.readLine()) != null) {
                content.append(line).append("\\n");
                if (content.length() > 1000000) { // Limit content size
                    break;
                }
            }
            
            String html = content.toString();
            Pattern pattern = Pattern.compile("<title>(.*?)</title>", Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
            Matcher matcher = pattern.matcher(html);
            
            if (matcher.find()) {
                return matcher.group(1).trim();
            } else {
                return "No title found";
            }
            
        } catch (Exception e) {
            return "Error: " + e.getMessage();
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
        if (args.length > 0) {
            String result = getPageTitle(args[0]);
            System.out.println("Title: " + result);
        } else {
            // Test cases
            System.out.println("Test Case 1:");
            System.out.println(getPageTitle("https://www.example.com"));
            
            System.out.println("\\nTest Case 2:");
            System.out.println(getPageTitle("https://www.google.com"));
            
            System.out.println("\\nTest Case 3:");
            System.out.println(getPageTitle("http://www.example.com"));
            
            System.out.println("\\nTest Case 4:");
            System.out.println(getPageTitle(""));
            
            System.out.println("\\nTest Case 5:");
            System.out.println(getPageTitle(null));
        }
    }
}
