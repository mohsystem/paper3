import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task49 {

    // Simple User model
    static class User {
        final int id;
        final String name;
        final String email;
        final int age;

        User(int id, String name, String email, int age) {
            this.id = id;
            this.name = name;
            this.email = email;
            this.age = age;
        }

        String toJson() {
            return "{\"id\":" + id +
                    ",\"name\":\"" + escapeJson(name) + "\"" +
                    ",\"email\":\"" + escapeJson(email) + "\"" +
                    ",\"age\":" + age + "}";
        }
    }

    // In-memory "database"
    static class InMemoryDB {
        private final ConcurrentHashMap<Integer, User> users = new ConcurrentHashMap<>();
        private final AtomicInteger nextId = new AtomicInteger(1);

        public User addUser(String name, String email, int age) {
            int id = nextId.getAndIncrement();
            User u = new User(id, name, email, age);
            users.put(id, u);
            return u;
        }

        public String getAllUsersJson() {
            StringBuilder sb = new StringBuilder();
            sb.append("[");
            boolean first = true;
            for (User u : users.values()) {
                if (!first) sb.append(",");
                sb.append(u.toJson());
                first = false;
            }
            sb.append("]");
            return sb.toString();
        }
    }

    private static final InMemoryDB DB = new InMemoryDB();
    private static HttpServer server;

    // Public API functions (accept params and return outputs)
    public static int addUserToDb(String name, String email, int age) {
        return DB.addUser(name, email, age).id;
    }

    public static String getAllUsersJson() {
        return DB.getAllUsersJson();
    }

    public static void startServer(int port) throws IOException {
        server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/users", new UsersHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
    }

    public static void stopServer() {
        if (server != null) server.stop(0);
    }

    // HTTP request helper methods for tests
    public static String httpPost(String urlStr, String json) throws IOException {
        URL url = new URL(urlStr);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        try {
            conn.setRequestMethod("POST");
            conn.setDoOutput(true);
            conn.setRequestProperty("Content-Type", "application/json; charset=UTF-8");
            try (OutputStream os = conn.getOutputStream()) {
                os.write(json.getBytes(StandardCharsets.UTF_8));
            }
            return readStream(conn);
        } finally {
            conn.disconnect();
        }
    }

    public static String httpGet(String urlStr) throws IOException {
        URL url = new URL(urlStr);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        try {
            conn.setRequestMethod("GET");
            return readStream(conn);
        } finally {
            conn.disconnect();
        }
    }

    private static String readStream(HttpURLConnection conn) throws IOException {
        InputStream is = (conn.getResponseCode() >= 200 && conn.getResponseCode() < 300)
                ? conn.getInputStream() : conn.getErrorStream();
        if (is == null) return "";
        try (BufferedReader br = new BufferedReader(new InputStreamReader(is, StandardCharsets.UTF_8))) {
            StringBuilder sb = new StringBuilder();
            String ln;
            while ((ln = br.readLine()) != null) sb.append(ln);
            return sb.toString();
        }
    }

    // HTTP handler
    static class UsersHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                String method = exchange.getRequestMethod();
                if ("POST".equalsIgnoreCase(method)) {
                    String body = new String(exchange.getRequestBody().readAllBytes(), StandardCharsets.UTF_8);
                    String name = getJsonStringValue(body, "name");
                    String email = getJsonStringValue(body, "email");
                    Integer age = getJsonIntValue(body, "age");
                    if (name == null || email == null || age == null) {
                        sendJson(exchange, 400, "{\"error\":\"Invalid input\"}");
                        return;
                    }
                    int id = addUserToDb(name, email, age);
                    sendJson(exchange, 200, "{\"status\":\"ok\",\"id\":" + id + "}");
                } else if ("GET".equalsIgnoreCase(method)) {
                    String resp = getAllUsersJson();
                    sendJson(exchange, 200, resp);
                } else {
                    sendJson(exchange, 405, "{\"error\":\"Method not allowed\"}");
                }
            } catch (Exception e) {
                sendJson(exchange, 500, "{\"error\":\"Server error\"}");
            }
        }

        private void sendJson(HttpExchange exchange, int code, String json) throws IOException {
            Headers h = exchange.getResponseHeaders();
            h.set("Content-Type", "application/json; charset=UTF-8");
            byte[] out = json.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(code, out.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(out);
            }
        }
    }

    // Minimal JSON parsing helpers for specific keys
    private static String getJsonStringValue(String json, String key) {
        Pattern p = Pattern.compile("\"" + Pattern.quote(key) + "\"\\s*:\\s*\"(.*?)\"", Pattern.DOTALL);
        Matcher m = p.matcher(json);
        if (m.find()) return m.group(1);
        return null;
    }

    private static Integer getJsonIntValue(String json, String key) {
        Pattern p = Pattern.compile("\"" + Pattern.quote(key) + "\"\\s*:\\s*(-?\\d+)");
        Matcher m = p.matcher(json);
        if (m.find()) return Integer.parseInt(m.group(1));
        return null;
    }

    private static String escapeJson(String s) {
        return s.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n").replace("\r", "\\r");
    }

    public static void main(String[] args) throws Exception {
        int port = 8081;
        startServer(port);
        Thread.sleep(300);

        String base = "http://localhost:" + port + "/users";
        String[] payloads = new String[]{
                "{\"name\":\"Alice\",\"email\":\"alice@example.com\",\"age\":30}",
                "{\"name\":\"Bob\",\"email\":\"bob@example.com\",\"age\":25}",
                "{\"name\":\"Carol\",\"email\":\"carol@example.com\",\"age\":28}",
                "{\"name\":\"Dave\",\"email\":\"dave@example.com\",\"age\":40}",
                "{\"name\":\"Eve\",\"email\":\"eve@example.com\",\"age\":22}"
        };

        for (String p : payloads) {
            System.out.println(httpPost(base, p));
        }
        System.out.println(httpGet(base));

        stopServer();
    }
}