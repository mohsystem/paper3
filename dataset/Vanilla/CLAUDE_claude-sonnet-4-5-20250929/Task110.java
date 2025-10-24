
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task110 {
    public static boolean validateURL(String url) {
        if (url == null || url.trim().isEmpty()) {
            return false;
        }
        
        // Regular expression pattern for URL validation
        String urlPattern = "^(https?|ftp)://"  // Protocol
                + "("
                + "([a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,}" // Domain name
                + "|"
                + "localhost" // OR localhost
                + "|"
                + "((\\\\d{1,3}\\\\.){3}\\\\d{1,3})" // OR IP address
                + ")"
                + "(:\\\\d+)?" // Optional port
                + "(/[a-zA-Z0-9._~:/?#\\\\[\\\\]@!$&'()*+,;=-]*)?" // Optional path
                + "$";
        
        Pattern pattern = Pattern.compile(urlPattern);
        Matcher matcher = pattern.matcher(url);
        
        return matcher.matches();
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testUrls = {
            "https://www.example.com",
            "http://localhost:8080/path/to/resource",
            "ftp://192.168.1.1/files",
            "invalid-url-without-protocol",
            "https://subdomain.example.co.uk:3000/api?param=value"
        };
        
        System.out.println("URL Validation Results:");
        for (String url : testUrls) {
            boolean isValid = validateURL(url);
            System.out.println(url + " -> " + (isValid ? "Valid" : "Invalid"));
        }
    }
}
