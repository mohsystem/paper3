import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse149 {
    private static final List<Message> DB = Collections.synchronizedList(new ArrayList<>());
    private static final int MAX_USERNAME = 50;
    private static final int MAX_MESSAGE = 500;
    private static final int MAX_BODY = 4096;

    private static class Message {
        final String username;
        final String message;
        final long timestamp;

        Message(String u, String m) {
            this.username = u;
            this.message = m;
            this.timestamp = System.currentTimeMillis();
        }
    }

    public static boolean insert_user_message_in_db(String username, String message) {
        if (username == null || message == null) return false;
        String u = username.trim();
        String m = message.trim();
        if (u.isEmpty() || m.isEmpty()) return false;
        if (u.length() > MAX_USERNAME || m.length() > MAX_MESSAGE) return false;
        synchronized (DB) {
            DB.add(new Message(u, m));
        }
        return true;
    }

    private static String htmlEscape(String s) {
        StringBuilder out = new StringBuilder((int)(s.length() * 1.2) + 16);
        for (char c : s.toCharArray()) {
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#x27;"); break;
                case '/': out.append("&#x2F;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    private static Map<String, String> parseForm(String body) {
        Map<String, String> map = new HashMap<>();
        String[] pairs = body.split("&");
        for (String p : pairs) {
            if (p.isEmpty()) continue;
            String[] kv = p.split("=", 2);
            try {
                String k = URLDecoder.decode(kv[0], StandardCharsets.UTF_8);
                String v = kv.length > 1 ? URLDecoder.decode(kv[1], StandardCharsets.UTF_8) : "";
                map.put(k, v);
            } catch (Exception ignored) {}
        }
        return map;
    }

    private static byte[] renderMessagesHtml() {
        StringBuilder sb = new StringBuilder();
        sb.append("<!doctype html><html><head><meta charset='utf-8'>")
          .append("<meta http-equiv='Content-Security-Policy' content=\"default-src 'self'; style-src 'self' 'unsafe-inline'\">")
          .append("<title>Messages</title></head><body>")
          .append("<h1>Messages</h1>")
          .append("<form method='POST' action='/post'>")
          .append("<label>Username: <input name='username' maxlength='").append(MAX_USERNAME).append("'></label><br>")
          .append("<label>Message: <input name='message' maxlength='").append(MAX_MESSAGE).append("'></label><br>")
          .append("<button type='submit'>Post</button></form><hr>");
        List<Message> snapshot;
        synchronized (DB) {
            snapshot = new ArrayList<>(DB);
        }
        snapshot.sort(Comparator.comparingLong(m -> m.timestamp));
        sb.append("<ul>");
        for (Message m : snapshot) {
            sb.append("<li><strong>")
              .append(htmlEscape(m.username))
              .append("</strong>: ")
              .append(htmlEscape(m.message))
              .append(" <small>(").append(Instant.ofEpochMilli(m.timestamp)).append(")</small>")
              .append("</li>");
        }
        sb.append("</ul></body></html>");
        return sb.toString().getBytes(StandardCharsets.UTF_8);
    }

    private static byte[] readBody(HttpExchange ex, int max) throws IOException {
        InputStream is = ex.getRequestBody();
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] buf = new byte[1024];
        int read;
        int total = 0;
        while ((read = is.read(buf)) != -1) {
            total += read;
            if (total > max) {
                return null;
            }
            bos.write(buf, 0, read);
        }
        return bos.toByteArray();
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases: pre-populate with messages
        insert_user_message_in_db("alice", "Hello world!");
        insert_user_message_in_db("bob", "Secure message <3");
        insert_user_message_in_db("charlie", "Stay safe & sound");
        insert_user_message_in_db("dora", "Use limits and escaping.");
        insert_user_message_in_db("eve", "I will not XSS you :)");

        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    exchange.sendResponseHeaders(405, -1);
                    exchange.close();
                    return;
                }
                Headers h = exchange.getResponseHeaders();
                h.add("Content-Type", "text/html; charset=utf-8");
                byte[] body = renderMessagesHtml();
                exchange.sendResponseHeaders(200, body.length);
                OutputStream os = exchange.getResponseBody();
                os.write(body);
                os.close();
            }
        });
        server.createContext("/post", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    exchange.sendResponseHeaders(405, -1);
                    exchange.close();
                    return;
                }
                String contentType = Optional.ofNullable(exchange.getRequestHeaders().getFirst("Content-Type")).orElse("");
                if (!contentType.toLowerCase(Locale.ROOT).startsWith("application/x-www-form-urlencoded")) {
                    exchange.sendResponseHeaders(415, -1);
                    exchange.close();
                    return;
                }
                byte[] bodyBytes = readBody(exchange, MAX_BODY);
                if (bodyBytes == null) {
                    exchange.sendResponseHeaders(413, -1);
                    exchange.close();
                    return;
                }
                String body = new String(bodyBytes, StandardCharsets.UTF_8);
                Map<String, String> form = parseForm(body);
                String username = form.getOrDefault("username", "");
                String message = form.getOrDefault("message", "");
                insert_user_message_in_db(username, message);
                Headers h = exchange.getResponseHeaders();
                h.add("Location", "/");
                exchange.sendResponseHeaders(303, -1);
                exchange.close();
            }
        });
        server.setExecutor(Executors.newFixedThreadPool(8));
        server.start();
    }
}