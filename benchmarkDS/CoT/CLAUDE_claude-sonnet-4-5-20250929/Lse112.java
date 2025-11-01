
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.util.regex.Pattern;

public class Lse112 {
    private static final Pattern VALID_HOSTNAME = Pattern.compile(
        "^(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)*[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?$"
    );
    
    public static String securePing(String url) {
        if (url == null || url.trim().isEmpty()) {
            return "Error: URL parameter is required";
        }
        
        url = url.trim();
        
        // Validate hostname format (no special characters that could enable injection)
        if (!VALID_HOSTNAME.matcher(url).matches()) {
            return "Error: Invalid hostname format";
        }
        
        try {
            InetAddress address = InetAddress.getByName(url);
            boolean reachable = address.isReachable(5000);
            return "Ping result for " + url + ": " + (reachable ? "Host is reachable" : "Host is not reachable");
        } catch (UnknownHostException e) {
            return "Error: Unknown host - " + url;
        } catch (IOException e) {
            return "Error: Unable to ping - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test 1: " + securePing("google.com"));
        System.out.println("Test 2: " + securePing("localhost"));
        System.out.println("Test 3: " + securePing("127.0.0.1"));
        System.out.println("Test 4: " + securePing("invalid..host"));
        System.out.println("Test 5: " + securePing("; rm -rf /"));
        
        // Web server setup
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/ping", new PingHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000. Access: http://localhost:8000/ping?url=google.com");
    }
    
    static class PingHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            String url = "";
            if (query != null && query.startsWith("url=")) {
                url = query.substring(4);
            }
            
            String response = securePing(url);
            exchange.sendResponseHeaders(200, response.length());
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }
}
