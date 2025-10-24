import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;

public class Task38 {

    /**
     * The main method starts the web server.
     * "5 test cases" for a web application involve running the server and testing in a browser.
     *
     * How to Test:
     * 1. Compile and run this file:
     *    javac Task38.java
     *    java Task38
     * 2. Open a web browser and go to http://localhost:8080
     * 3. You will see a form.
     * 4. Test Case 1: Enter "Hello World" and submit.
     * 5. Test Case 2: Enter "Java Web App" and submit.
     * 6. Test Case 3: Enter an empty string and submit.
     * 7. Test Case 4: Enter "12345" and submit.
     * 8. Test Case 5: Enter "Special Characters: &<>" and submit.
     */
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new FormHandler());
        server.createContext("/submit", new SubmitHandler());
        server.setExecutor(null); // creates a default executor
        System.out.println("Server started on port 8080. Open http://localhost:8080 in your browser.");
        server.start();
    }

    // Serves the initial HTML form
    static class FormHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String formHtml = "<html><head><title>Input Form</title></head>" +
                              "<body>" +
                              "<h1>Enter some text</h1>" +
                              "<form action=\"/submit\" method=\"post\">" +
                              "<label for=\"userInput\">Your Input:</label><br>" +
                              "<input type=\"text\" id=\"userInput\" name=\"userInput\" placeholder=\"Enter text here...\">" +
                              "<br><br>" +
                              "<input type=\"submit\" value=\"Submit\">" +
                              "</form>" +
                              "</body></html>";
            
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, formHtml.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(formHtml.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    // Handles the form submission and displays the user's input back
    static class SubmitHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String responseMessage = "";
            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
                BufferedReader br = new BufferedReader(isr);
                String formData = br.lines().collect(Collectors.joining());

                Map<String, String> params = parseFormData(formData);
                String userInput = params.getOrDefault("userInput", "No input provided");
                
                // Basic HTML escaping for security to prevent XSS
                String escapedInput = userInput.replace("&", "&amp;")
                                               .replace("<", "&lt;")
                                               .replace(">", "&gt;");

                responseMessage = "<html><head><title>Submission Result</title></head>" +
                                  "<body>" +
                                  "<h1>You submitted:</h1>" +
                                  "<p>" + escapedInput + "</p>" +
                                  "<a href=\"/\">Go back</a>" +
                                  "</body></html>";
            } else {
                exchange.sendResponseHeaders(405, -1); // Method Not Allowed
                return;
            }
            
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, responseMessage.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseMessage.getBytes(StandardCharsets.UTF_8));
            }
        }
        
        private Map<String, String> parseFormData(String formData) throws IOException {
            Map<String, String> map = new HashMap<>();
            String[] pairs = formData.split("&");
            for (String pair : pairs) {
                int idx = pair.indexOf("=");
                if (idx > 0) {
                     map.put(URLDecoder.decode(pair.substring(0, idx), "UTF-8"), 
                             URLDecoder.decode(pair.substring(idx + 1), "UTF-8"));
                } else if (pair.endsWith("=")){
                     map.put(URLDecoder.decode(pair.substring(0, pair.length() - 1), "UTF-8"), "");
                }
            }
            return map;
        }
    }
}