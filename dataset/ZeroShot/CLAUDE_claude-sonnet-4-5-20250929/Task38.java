
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Task38 {
    
    public static String sanitizeInput(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    public static String getFormHTML() {
        return "<!DOCTYPE html><html><head><title>User Input Form</title>" +
               "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:50px auto;padding:20px;}" +
               "input,textarea{width:100%;padding:10px;margin:10px 0;box-sizing:border-box;}" +
               "button{background:#4CAF50;color:white;padding:10px 20px;border:none;cursor:pointer;}" +
               "button:hover{background:#45a049;}</style></head><body>" +
               "<h1>User Input Form</h1>" +
               "<form method='POST' action='/submit'>" +
               "<label>Name:</label><input type='text' name='name' required><br>" +
               "<label>Message:</label><textarea name='message' rows='4' required></textarea><br>" +
               "<button type='submit'>Submit</button></form></body></html>";
    }
    
    public static String getResponseHTML(String name, String message) {
        String safeName = sanitizeInput(name);
        String safeMessage = sanitizeInput(message);
        return "<!DOCTYPE html><html><head><title>Your Response</title>" +
               "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:50px auto;padding:20px;}" +
               ".result{background:#f0f0f0;padding:20px;border-radius:5px;margin:20px 0;}" +
               "a{color:#4CAF50;text-decoration:none;}</style></head><body>" +
               "<h1>Your Submission</h1><div class='result'>" +
               "<p><strong>Name:</strong> " + safeName + "</p>" +
               "<p><strong>Message:</strong> " + safeMessage + "</p></div>" +
               "<a href='/'>← Back to Form</a></body></html>";
    }
    
    public static Map<String, String> parseFormData(String formData) throws UnsupportedEncodingException {
        Map<String, String> params = new HashMap<>();
        if (formData == null || formData.isEmpty()) return params;
        
        String[] pairs = formData.split("&");
        for (String pair : pairs) {
            String[] keyValue = pair.split("=", 2);
            if (keyValue.length == 2) {
                String key = URLDecoder.decode(keyValue[0], "UTF-8");
                String value = URLDecoder.decode(keyValue[1], "UTF-8");
                params.put(key, value);
            }
        }
        return params;
    }
    
    static class FormHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException {
            String response = getFormHTML();
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes(StandardCharsets.UTF_8));
            os.close();
        }
    }
    
    static class SubmitHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            if ("POST".equals(exchange.getRequestMethod())) {
                InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
                BufferedReader br = new BufferedReader(isr);
                String formData = br.readLine();
                
                Map<String, String> params = parseFormData(formData);
                String name = params.getOrDefault("name", "");
                String message = params.getOrDefault("message", "");
                
                response = getResponseHTML(name, message);
            } else {
                response = "<html><body><h1>Method Not Allowed</h1><a href='/'>Go Back</a></body></html>";
            }
            
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes(StandardCharsets.UTF_8));
            os.close();
        }
    }
    
    public static void main(String[] args) throws IOException {
        System.out.println("Testing sanitizeInput function with 5 test cases:\\n");
        
        String[] testCases = {
            "Hello World",
            "<script>alert('XSS')</script>",
            "Name with <b>HTML</b> tags",
            "Special chars: & < > \\" '",
            "Normal text without special characters"
        };
        
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: " + testCases[i]);
            System.out.println("Sanitized: " + sanitizeInput(testCases[i]));
            System.out.println();
        }
        
        System.out.println("\\nStarting web server on http://localhost:8000");
        System.out.println("Press Ctrl+C to stop the server\\n");
        
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/", new FormHandler());
        server.createContext("/submit", new SubmitHandler());
        server.setExecutor(null);
        server.start();
    }
}
