import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URL;
import java.nio.charset.StandardCharsets;

import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.servlet.ServletContextHandler;
import org.eclipse.jetty.servlet.ServletHolder;


public class Lse55 extends HttpServlet {

    @Override
    protected void doGet(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        String path = req.getServletPath();
        String responseContent = "";
        int statusCode = HttpServletResponse.SC_OK;

        switch (path) {
            case "/":
            case "/index":
                responseContent = index();
                break;
            case "/login":
                responseContent = login();
                break;
            case "/user":
                responseContent = user_page();
                break;
            case "/admin":
                responseContent = admin_page();
                break;
            default:
                responseContent = "<h1>404 Not Found</h1>";
                statusCode = HttpServletResponse.SC_NOT_FOUND;
                break;
        }

        resp.setStatus(statusCode);
        resp.setContentType("text/html; charset=utf-8");
        try (PrintWriter writer = resp.getWriter()) {
            writer.write(responseContent);
        }
    }

    @Override
    protected void doPost(HttpServletRequest req, HttpServletResponse resp) throws ServletException, IOException {
        String path = req.getServletPath();
        if ("/do_login".equals(path)) {
            do_login(req, resp);
        } else {
            resp.setStatus(HttpServletResponse.SC_NOT_FOUND);
            resp.getWriter().write("<h1>404 Not Found</h1>");
        }
    }

    // GET /login
    private String login() {
        return "<h1>Login Page</h1>"
                + "<form action='/do_login' method='post'>"
                + "Username: <input type='text' name='username'><br>"
                + "Password: <input type='password' name='password'><br>"
                + "<input type='submit' value='Login'>"
                + "</form>";
    }

    // POST /do_login
    private void do_login(HttpServletRequest req, HttpServletResponse resp) throws IOException {
        // Rules#6: Ensure all input is validated
        String username = req.getParameter("username");
        String password = req.getParameter("password");

        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            resp.setStatus(HttpServletResponse.SC_BAD_REQUEST);
            resp.getWriter().write("<h1>Username and password cannot be empty.</h1>");
            return;
        }

        // In a real application, you would validate credentials against a database.
        // Rules#1, #3, #4, #5: NEVER store passwords in plaintext.
        // Use a strong, salted, and peppered hashing algorithm like Argon2, scrypt, or PBKDF2.
        // The following is for demonstration purposes ONLY and is highly insecure.
        if ("admin".equals(username) && "password123".equals(password)) {
            resp.getWriter().write("<h1>Admin Login Successful!</h1>");
        } else if ("user".equals(username) && "password123".equals(password)) {
            resp.getWriter().write("<h1>User Login Successful!</h1>");
        } else {
            resp.setStatus(HttpServletResponse.SC_UNAUTHORIZED);
            resp.getWriter().write("<h1>Login Failed: Invalid credentials.</h1>");
        }
    }

    // GET /
    private String index() {
        return "<h1>Welcome to the Index Page!</h1>";
    }

    // GET /user
    private String user_page() {
        return "<h1>Welcome to the User Page!</h1>";
    }

    // GET /admin
    private String admin_page() {
        return "<h1>Welcome to the Admin Page!</h1>";
    }
    
    public static void main(String[] args) throws Exception {
        int port = 8080;
        Server server = new Server(new InetSocketAddress("localhost", port));
        ServletContextHandler context = new ServletContextHandler(ServletContextHandler.SESSIONS);
        context.setContextPath("/");
        server.setHandler(context);
        context.addServlet(new ServletHolder(new Lse55()), "/*");

        System.out.println("Starting server on port " + port);
        server.start();

        System.out.println("\n--- Running 5 Test Cases ---");
        runTest("GET", "http://localhost:8080/index", null); // Test Case 1: index function
        runTest("GET", "http://localhost:8080/login", null); // Test Case 2: login function
        runTest("POST", "http://localhost:8080/do_login", "username=admin&password=password123"); // Test Case 3: do_login function
        runTest("GET", "http://localhost:8080/user", null);  // Test Case 4: user_page function
        runTest("GET", "http://localhost:8080/admin", null); // Test Case 5: admin_page function
        System.out.println("--- Test Cases Finished ---\n");

        System.out.println("Stopping server...");
        server.stop();
        server.join();
        System.out.println("Server stopped.");
    }
    
    private static void runTest(String method, String urlString, String postData) {
        System.out.println("Testing " + method + " " + urlString);
        HttpURLConnection conn = null;
        try {
            URL url = new URL(urlString);
            conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod(method);
            conn.setConnectTimeout(5000);
            conn.setReadTimeout(5000);

            if ("POST".equals(method) && postData != null) {
                conn.setDoOutput(true);
                conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
                byte[] postDataBytes = postData.getBytes(StandardCharsets.UTF_8);
                conn.setRequestProperty("Content-Length", String.valueOf(postDataBytes.length));
                try (OutputStream os = conn.getOutputStream()) {
                    os.write(postDataBytes);
                }
            }

            int responseCode = conn.getResponseCode();
            System.out.println("Response Code: " + responseCode);

            java.io.InputStream is = (responseCode >= 200 && responseCode < 400) ? conn.getInputStream() : conn.getErrorStream();
            if (is != null) {
                try (java.util.Scanner s = new java.util.Scanner(is)) {
                    s.useDelimiter("\\A");
                    String responseBody = s.hasNext() ? s.next() : "";
                    System.out.println("Response Body: " + responseBody.substring(0, Math.min(100, responseBody.length())).replace('\n', ' ') + "...");
                }
            }
        } catch (IOException e) {
            System.err.println("Error during test: " + e.getMessage());
        } finally {
            if (conn != null) {
                conn.disconnect();
            }
        }
        System.out.println("----------------------------");
    }
}