import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.Executors;

public class Lse32 {

    static class Response {
        int status;
        String location; // for redirect
        String body;     // for error or informational messages

        static Response redirect(String location) {
            Response r = new Response();
            r.status = 302;
            r.location = location;
            r.body = "";
            return r;
        }

        static Response error(int status, String message) {
            Response r = new Response();
            r.status = status;
            r.body = message;
            return r;
        }
    }

    // Core logic function: accepts a URL string and returns a structured response
    public static Response redirect_invalid_url(String urlStr) {
        if (urlStr == null || urlStr.isEmpty()) {
            return Response.error(400, "Missing URL");
        }
        try {
            URI uri = new URI(urlStr);
            String scheme = uri.getScheme();
            String host = uri.getHost();
            if (scheme == null || host == null) {
                return Response.error(400, "Invalid URL");
            }
            if ("example.com".equalsIgnoreCase(host)) {
                return Response.redirect(urlStr);
            } else {
                return Response.error(400, "URL must be from example.com");
            }
        } catch (URISyntaxException e) {
            return Response.error(400, "Invalid URL");
        }
    }

    static class RedirectHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    send(exchange, 405, "Method Not Allowed");
                    return;
                }
                String rawPath = exchange.getRequestURI().getRawPath();
                String prefix = "/redirect/";
                if (!rawPath.startsWith(prefix)) {
                    send(exchange, 404, "Not Found");
                    return;
                }
                String encodedParam = rawPath.substring(prefix.length());
                if (encodedParam.isEmpty()) {
                    send(exchange, 400, "Missing URL");
                    return;
                }
                String decodedUrl = URLDecoder.decode(encodedParam, StandardCharsets.UTF_8);
                Response resp = redirect_invalid_url(decodedUrl);
                if (resp.status == 302 && resp.location != null) {
                    Headers headers = exchange.getResponseHeaders();
                    headers.add("Location", resp.location);
                    exchange.sendResponseHeaders(302, -1);
                } else {
                    send(exchange, resp.status, resp.body == null ? "" : resp.body);
                }
            } catch (Exception ex) {
                send(exchange, 500, "Internal Server Error");
            }
        }

        private void send(HttpExchange ex, int status, String body) throws IOException {
            byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
            ex.sendResponseHeaders(status, bytes.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    private static void startServer() throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        server.createContext("/redirect/", new RedirectHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
        System.out.println("Java server started at http://127.0.0.1:8080");
    }

    private static void runTest(String label, String encodedUrlParam) throws IOException {
        URL url = new URL("http://127.0.0.1:8080/redirect/" + encodedUrlParam);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setInstanceFollowRedirects(false);
        conn.setRequestMethod("GET");
        int code = conn.getResponseCode();
        String location = conn.getHeaderField("Location");
        System.out.println(label + ": status=" + code + (location != null ? " location=" + location : ""));
        conn.disconnect();
    }

    public static void main(String[] args) throws Exception {
        startServer();
        Thread.sleep(500);

        String[] tests = new String[] {
                URLEncoder.encode("https://example.com/", StandardCharsets.UTF_8),
                URLEncoder.encode("http://example.com/blog", StandardCharsets.UTF_8),
                URLEncoder.encode("https://evil.com/hack", StandardCharsets.UTF_8),
                URLEncoder.encode("notaurl", StandardCharsets.UTF_8),
                URLEncoder.encode("https://sub.example.com/", StandardCharsets.UTF_8)
        };
        runTest("Test 1 (valid https example.com)", tests[0]);
        runTest("Test 2 (valid http example.com)", tests[1]);
        runTest("Test 3 (invalid other domain)", tests[2]);
        runTest("Test 4 (invalid URL)", tests[3]);
        runTest("Test 5 (subdomain not allowed)", tests[4]);

        System.out.println("Java tests complete. Press Ctrl+C to stop the server.");
    }
}