import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.util.HashMap;
import java.util.Map;
import java.util.Scanner;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse54 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        // Context for the login form (GET)
        server.createContext("/login", new LoginHandler());
        
        // Context for processing the login (POST)
        server.createContext("/do_login", new DoLoginHandler());
        
        // Context for the protected admin page
        server.createContext("/admin_page", new AdminPageHandler());
        
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Java server started on port " + port);
        System.out.println("--- 5 Test Cases ---");
        System.out.println("1. View login form: Open http://localhost:8080/login in a browser or run: curl http://localhost:8080/login");
        System.out.println("2. Successful login: curl -X POST -d \"username=admin&password=admin\" -L http://localhost:8080/do_login");
        System.out.println("3. Failed login (wrong user): curl -X POST -d \"username=user&password=admin\" http://localhost:8080/do_login");
        System.out.println("4. Failed login (wrong pass): curl -X POST -d \"username=admin&password=wrong\" http://localhost:8080/do_login");
        System.out.println("5. View admin page directly: curl http://localhost:8080/admin_page");
    }

    // Handler for GET /login
    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            if ("GET".equals(t.getRequestMethod())) {
                String response = "<html><body>" +
                                  "<h2>Login</h2>" +
                                  "<form method='POST' action='/do_login'>" +
                                  "Username: <input type='text' name='username'><br>" +
                                  "Password: <input type='password' name='password'><br>" +
                                  "<input type='submit' value='Login'>" +
                                  "</form>" +
                                  "</body></html>";
                sendResponse(t, 200, response);
            } else {
                sendResponse(t, 405, "Method Not Allowed");
            }
        }
    }

    // Handler for POST /do_login
    static class DoLoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            if ("POST".equals(t.getRequestMethod())) {
                Map<String, String> params = parseFormData(t.getRequestBody());
                String username = params.getOrDefault("username", "");
                String password = params.getOrDefault("password", "");

                if ("admin".equals(username) && "admin".equals(password)) {
                    // Redirect to admin_page on success
                    t.getResponseHeaders().set("Location", "/admin_page");
                    t.sendResponseHeaders(302, -1); // 302 Found for redirection
                } else {
                    sendResponse(t, 401, "Login Failed: Invalid credentials.");
                }
            } else {
                sendResponse(t, 405, "Method Not Allowed");
            }
        }
    }
    
    // Handler for GET /admin_page
    static class AdminPageHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
             if ("GET".equals(t.getRequestMethod())) {
                sendResponse(t, 200, "<html><body><h1>Welcome, Admin!</h1></body></html>");
             } else {
                sendResponse(t, 405, "Method Not Allowed");
             }
        }
    }
    
    // Utility to send a response
    private static void sendResponse(HttpExchange t, int statusCode, String response) throws IOException {
        t.sendResponseHeaders(statusCode, response.getBytes().length);
        OutputStream os = t.getResponseBody();
        os.write(response.getBytes());
        os.close();
    }

    // Utility to parse form data from request body
    private static Map<String, String> parseFormData(InputStream is) throws UnsupportedEncodingException {
        Scanner scanner = new Scanner(is).useDelimiter("\\A");
        String body = scanner.hasNext() ? scanner.next() : "";
        Map<String, String> params = new HashMap<>();
        for (String param : body.split("&")) {
            String[] pair = param.split("=");
            if (pair.length > 1) {
                params.put(URLDecoder.decode(pair[0], "UTF-8"), URLDecoder.decode(pair[1], "UTF-8"));
            } else if (pair.length == 1) {
                params.put(URLDecoder.decode(pair[0], "UTF-8"), "");
            }
        }
        return params;
    }
}