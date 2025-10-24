
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Task120 {
    
    public static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        
        if (input.length() > 1000) {
            input = input.substring(0, 1000);
        }
        
        StringBuilder sanitized = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '<':
                    sanitized.append("&lt;");
                    break;
                case '>':
                    sanitized.append("&gt;");
                    break;
                case '&':
                    sanitized.append("&amp;");
                    break;
                case '"':
                    sanitized.append("&quot;");
                    break;
                case '\\'':
                    sanitized.append("&#x27;");
                    break;
                case '/':
                    sanitized.append("&#x2F;");
                    break;
                default:
                    if (c >= 32 && c <= 126 || c == '\\n' || c == '\\r' || c == '\\t') {
                        sanitized.append(c);
                    } else {
                        sanitized.append("&#").append((int) c).append(";");
                    }
                    break;
            }
        }
        return sanitized.toString();
    }
    
    public static String generateHtmlPage(String userInput) {
        String sanitized = sanitizeInput(userInput);
        
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html>\\n");
        html.append("<html lang=\\"en\\">\\n");
        html.append("<head>\\n");
        html.append("<meta charset=\\"UTF-8\\">\\n");
        html.append("<meta name=\\"viewport\\" content=\\"width=device-width, initial-scale=1.0\\">\\n");
        html.append("<meta http-equiv=\\"X-Content-Type-Options\\" content=\\"nosniff\\">\\n");
        html.append("<meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'none'; style-src 'unsafe-inline';\\">\\n");
        html.append("<title>User Input Display</title>\\n");
        html.append("<style>body{font-family:Arial,sans-serif;margin:20px;}</style>\\n");
        html.append("</head>\\n");
        html.append("<body>\\n");
        html.append("<h1>User Input Display</h1>\\n");
        html.append("<div id=\\"user-content\\">\\n");
        html.append("<p>Your input:</p>\\n");
        html.append("<pre>").append(sanitized).append("</pre>\\n");
        html.append("</div>\\n");
        html.append("</body>\\n");
        html.append("</html>");
        
        return html.toString();
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Simple text");
        String test1 = generateHtmlPage("Hello World");
        System.out.println(test1.contains("Hello World"));
        
        System.out.println("\\nTest Case 2: XSS attempt with script tag");
        String test2 = generateHtmlPage("<script>alert('XSS')</script>");
        System.out.println(test2.contains("&lt;script&gt;"));
        System.out.println(!test2.contains("<script>alert"));
        
        System.out.println("\\nTest Case 3: HTML injection attempt");
        String test3 = generateHtmlPage("<img src=x onerror=alert(1)>");
        System.out.println(test3.contains("&lt;img"));
        System.out.println(!test3.contains("<img src="));
        
        System.out.println("\\nTest Case 4: Special characters");
        String test4 = generateHtmlPage("Test & \\"quotes\\" and 'apostrophes'");
        System.out.println(test4.contains("&amp;"));
        System.out.println(test4.contains("&quot;"));
        System.out.println(test4.contains("&#x27;"));
        
        System.out.println("\\nTest Case 5: Empty input");
        String test5 = generateHtmlPage("");
        System.out.println(test5.contains("<pre></pre>"));
    }
}
