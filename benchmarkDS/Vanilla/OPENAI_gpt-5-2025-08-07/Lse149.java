import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse149 {

    private static final List<Message> MESSAGES = Collections.synchronizedList(new ArrayList<>());

    static class Message {
        final String username;
        final String text;
        final long timestamp;

        Message(String username, String text) {
            this.username = username;
            this.text = text;
            this.timestamp = System.currentTimeMillis();
        }
    }

    // Inserts a user message into the "database" (in-memory list)
    public static synchronized boolean insert_user_message_in_db(String username, String message) {
        if (username == null || username.trim().isEmpty()) return false;
        if (message == null || message.trim().isEmpty()) return false;
        MESSAGES.add(new Message(username.trim(), message.trim()));
        return true;
    }

    // Renders the messages into an HTML fragment
    public static String render_messages() {
        StringBuilder sb = new StringBuilder();
        sb.append("<ul>");
        synchronized (MESSAGES) {
            for (Message m : MESSAGES) {
                sb.append("<li><strong>")
                        .append(escapeHtml(m.username))
                        .append(":</strong> ")
                        .append(escapeHtml(m.text))
                        .append(" <em>(")
                        .append(new Date(m.timestamp))
                        .append(")</em></li>");
            }
        }
        sb.append("</ul>");
        return sb.toString();
    }

    // Full page rendering
    public static String render_page() {
        return "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Messages</title></head><body>" +
                "<h1>Messages</h1>" +
                render_messages() +
                "<h2>Post a message</h2>" +
                "<form method='POST' action='/post'>" +
                "Username: <input name='username' required/> <br/>" +
                "Message: <input name='message' required/> <br/>" +
                "<button type='submit'>Post</button>" +
                "</form>" +
                "</body></html>";
    }

    private static String escapeHtml(String s) {
        if (s == null) return "";
        return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
                .replace("\"", "&quot;").replace("'", "&#x27;");
    }

    private static Map<String, String> parseForm(String form) throws UnsupportedEncodingException {
        Map<String, String> map = new HashMap<>();
        if (form == null || form.isEmpty()) return map;
        String[] pairs = form.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf('=');
            if (idx >= 0) {
                String key = URLDecoder.decode(pair.substring(0, idx), "UTF-8");
                String val = URLDecoder.decode(pair.substring(idx + 1), "UTF-8");
                map.put(key, val);
            } else {
                map.put(URLDecoder.decode(pair, "UTF-8"), "");
            }
        }
        return map;
    }

    private static void writeResponse(HttpExchange exchange, int status, String contentType, byte[] body) throws IOException {
        Headers headers = exchange.getResponseHeaders();
        headers.set("Content-Type", contentType);
        exchange.sendResponseHeaders(status, body.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(body);
        }
    }

    private static void redirect(HttpExchange exchange, String location) throws IOException {
        Headers headers = exchange.getResponseHeaders();
        headers.set("Location", location);
        exchange.sendResponseHeaders(302, -1);
        exchange.close();
    }

    // Start server
    public static HttpServer startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", exchange -> {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    writeResponse(exchange, 405, "text/plain; charset=utf-8", "Method Not Allowed".getBytes(StandardCharsets.UTF_8));
                    return;
                }
                String html = render_page();
                writeResponse(exchange, 200, "text/html; charset=utf-8", html.getBytes(StandardCharsets.UTF_8));
            } catch (Exception e) {
                String err = "Internal Server Error";
                writeResponse(exchange, 500, "text/plain; charset=utf-8", err.getBytes(StandardCharsets.UTF_8));
            }
        });
        server.createContext("/post", exchange -> {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    writeResponse(exchange, 405, "text/plain; charset=utf-8", "Method Not Allowed".getBytes(StandardCharsets.UTF_8));
                    return;
                }
                String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
                if (contentType == null || !contentType.toLowerCase(Locale.ROOT).startsWith("application/x-www-form-urlencoded")) {
                    writeResponse(exchange, 400, "text/plain; charset=utf-8", "Bad Request".getBytes(StandardCharsets.UTF_8));
                    return;
                }
                byte[] bodyBytes = readAll(exchange.getRequestBody());
                String body = new String(bodyBytes, StandardCharsets.UTF_8);
                Map<String, String> form = parseForm(body);
                String username = form.getOrDefault("username", "");
                String message = form.getOrDefault("message", "");
                insert_user_message_in_db(username, message);
                redirect(exchange, "/");
            } catch (Exception e) {
                String err = "Internal Server Error";
                writeResponse(exchange, 500, "text/plain; charset=utf-8", err.getBytes(StandardCharsets.UTF_8));
            }
        });
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        return server;
    }

    private static byte[] readAll(InputStream in) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] buf = new byte[4096];
        int r;
        while ((r = in.read(buf)) != -1) bos.write(buf, 0, r);
        return bos.toByteArray();
    }

    // Simple HTTP helper for tests
    private static int postForm(String urlStr, Map<String, String> data) throws IOException {
        URL url = new URL(urlStr);
        HttpURLConnection con = (HttpURLConnection) url.openConnection();
        con.setInstanceFollowRedirects(false);
        con.setRequestMethod("POST");
        con.setDoOutput(true);
        con.setRequestProperty("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
        StringBuilder sb = new StringBuilder();
        boolean first = true;
        for (Map.Entry<String, String> e : data.entrySet()) {
            if (!first) sb.append('&');
            first = false;
            sb.append(URLEncoder.encode(e.getKey(), "UTF-8")).append('=').append(URLEncoder.encode(e.getValue(), "UTF-8"));
        }
        byte[] out = sb.toString().getBytes(StandardCharsets.UTF_8);
        con.setFixedLengthStreamingMode(out.length);
        con.connect();
        try (OutputStream os = con.getOutputStream()) {
            os.write(out);
        }
        int code = con.getResponseCode();
        con.disconnect();
        return code;
    }

    private static String getUrl(String urlStr) throws IOException {
        URL url = new URL(urlStr);
        HttpURLConnection con = (HttpURLConnection) url.openConnection();
        con.setRequestMethod("GET");
        try (InputStream in = con.getInputStream()) {
            return new String(readAll(in), StandardCharsets.UTF_8);
        } finally {
            con.disconnect();
        }
    }

    public static void main(String[] args) throws Exception {
        int port = 8080;
        HttpServer server = startServer(port);
        Thread.sleep(200);

        // 5 test cases: post 5 messages
        for (int i = 1; i <= 5; i++) {
            Map<String, String> data = new LinkedHashMap<>();
            data.put("username", "user" + i);
            data.put("message", "hello world " + i);
            int code = postForm("http://localhost:" + port + "/post", data);
            System.out.println("POST " + i + " response: " + code);
        }

        // Fetch and print the page content
        String page = getUrl("http://localhost:" + port + "/");
        System.out.println(page);

        server.stop(0);
    }
}