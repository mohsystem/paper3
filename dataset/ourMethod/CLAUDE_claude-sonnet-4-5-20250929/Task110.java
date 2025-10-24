
import java.net.URI;
import java.net.URISyntaxException;
import java.util.regex.Pattern;

public class Task110 {
    private static final int MAX_URL_LENGTH = 2048;
    private static final Pattern VALID_SCHEME = Pattern.compile("^(https?|ftp)$");
    private static final Pattern VALID_HOST = Pattern.compile("^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\\\\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$");
    
    public static boolean validateURL(String url) {
        if (url == null || url.isEmpty()) {
            return false;
        }
        
        if (url.length() > MAX_URL_LENGTH) {
            return false;
        }
        
        String trimmedUrl = url.trim();
        if (trimmedUrl.isEmpty()) {
            return false;
        }
        
        try {
            URI uri = new URI(trimmedUrl);
            
            String scheme = uri.getScheme();
            if (scheme == null || !VALID_SCHEME.matcher(scheme.toLowerCase()).matches()) {
                return false;
            }
            
            String host = uri.getHost();
            if (host == null || host.isEmpty()) {
                return false;
            }
            
            if (!VALID_HOST.matcher(host).matches()) {
                try {
                    java.net.InetAddress.getByName(host);
                } catch (Exception e) {
                    return false;
                }
            }
            
            int port = uri.getPort();
            if (port < -1 || port > 65535) {
                return false;
            }
            
            String path = uri.getPath();
            if (path != null && (path.contains("..") || path.contains("//") || path.contains("\\\\"))) {\n                return false;\n            }\n            \n            return true;\n            \n        } catch (URISyntaxException e) {\n            return false;\n        } catch (Exception e) {\n            return false;\n        }\n    }\n    \n    public static void main(String[] args) {\n        String[] testCases = {\n            "https://www.example.com",\n            "http://example.com/path/to/resource",\n            "ftp://ftp.example.com:21/file.txt",\n            "javascript:alert('xss')",\n            "http://.invalid..domain/path"\n        };\n        \n        for (String testUrl : testCases) {\n            boolean result = validateURL(testUrl);\n            System.out.println("URL: " + testUrl + " -> Valid: " + result);
        }
    }
}
