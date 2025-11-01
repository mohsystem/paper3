import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.MalformedURLException;
import java.net.URL;

public class Lse32 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/redirect/", new RedirectHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        
        System.out.println("Java server started on port " + port);
        System.out.println("--- 5 Test Cases (run in a separate terminal) ---");
        System.out.println("1. Valid URL: curl -i -L http://localhost:8080/redirect/http://example.com/some/path");
        System.out.println("2. Valid HTTPS URL: curl -i -L http://localhost:8080/redirect/https://example.com/another/page?query=1");
        System.out.println("3. Invalid Domain: curl -i http://localhost:8080/redirect/http://google.com");
        System.out.println("4. Malformed URL: curl -i http://localhost:8080/redirect/not-a-valid-url");
        System.out.println("5. Invalid Domain (not example.com): curl -i http://localhost:8080/redirect/http://test.example.org");
    }

    static class RedirectHandler implements HttpHandler {
        
        public void redirect_invalid_url(HttpExchange exchange, String urlStr) throws IOException {
            try {
                URL targetUrl = new URL(urlStr);
                String host = targetUrl.getHost();

                if ("example.com".equalsIgnoreCase(host)) {
                    // Valid domain, perform redirect
                    exchange.getResponseHeaders().set("Location", targetUrl.toString());
                    exchange.sendResponseHeaders(302, -1); // 302 Found for temporary redirect
                } else {
                    // Invalid domain, return error
                    sendErrorResponse(exchange, 400, "Error: Redirects are only allowed to example.com.");
                }
            } catch (MalformedURLException e) {
                // The provided string is not a valid URL
                sendErrorResponse(exchange, 400, "Error: Invalid URL format provided.");
            }
        }
        
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            // The path is /redirect/<url>, so we extract the <url> part
            if (path.length() > "/redirect/".length()) {
                String targetUrlStr = path.substring("/redirect/".length());
                redirect_invalid_url(exchange, targetUrlStr);
            } else {
                sendErrorResponse(exchange, 400, "Error: URL to redirect is missing.");
            }
        }

        private void sendErrorResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
            byte[] responseBytes = message.getBytes("UTF-8");
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }
}