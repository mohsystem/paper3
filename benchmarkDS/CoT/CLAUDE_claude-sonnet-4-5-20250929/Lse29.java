
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse29 {
    private static final Pattern SAFE_URL_PATTERN = Pattern.compile(
        "^https?://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/.*)?$"
    );

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            String target = "/";
            
            if (query != null) {
                String[] params = query.split("&");
                for (String param : params) {
                    String[] keyValue = param.split("=", 2);
                    if (keyValue.length == 2 && keyValue[0].equals("target")) {
                        String decodedTarget = URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8);
                        if (isValidTarget(decodedTarget)) {
                            target = decodedTarget;
                        }
                        break;
                    }
                }
            }
            
            exchange.getResponseHeaders().set("Location", target);
            exchange.sendResponseHeaders(302, -1);
            exchange.close();
        }
        
        private boolean isValidTarget(String target) {
            if (target == null || target.isEmpty()) {
                return false;
            }
            return SAFE_URL_PATTERN.matcher(target).matches();
        }
    }

    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/redirect", new RedirectHandler());
        server.setExecutor(null);
        
        System.out.println("Test Cases:");
        System.out.println("1. Valid: http://example.com -> " + 
            new RedirectHandler().new TestHelper().test("http://example.com"));
        System.out.println("2. Valid: https://www.example.com/path -> " + 
            new RedirectHandler().new TestHelper().test("https://www.example.com/path"));
        System.out.println("3. Invalid: http://malicious.com -> " + 
            new RedirectHandler().new TestHelper().test("http://malicious.com"));
        System.out.println("4. Invalid: javascript:alert(1) -> " + 
            new RedirectHandler().new TestHelper().test("javascript:alert(1)"));
        System.out.println("5. Valid: http://subdomain.example.com -> " + 
            new RedirectHandler().new TestHelper().test("http://subdomain.example.com"));
        
        System.out.println("\\nServer started on port 8080");
        server.start();
    }
}
