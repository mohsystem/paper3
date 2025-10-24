import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.LinkedHashMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.regex.Pattern;

public class Task49 {

    // Simple user record
    static final class User {
        final int id;
        final String username;
        final String email;
        final int age;
        User(int id, String username, String email, int age) {
            this.id = id;
            this.username = username;
            this.email = email;
            this.age = age;
        }
    }

    // In-memory database
    static final class InMemoryDatabase {
        private final ConcurrentHashMap<Integer, User> users = new ConcurrentHashMap<>();
        private final AtomicInteger idCounter = new AtomicInteger(1);

        public int addUser(String username, String email, int age) {
            int id = idCounter.getAndIncrement();
            users.put(id, new User(id, username, email, age));
            return id;
        }

        public int size() {
            return users.size();
        }
    }

    static final class UsersHandler implements HttpHandler {
        private static final int MAX_BODY_SIZE = 4096;
        private final InMemoryDatabase db;

        private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{3,20}$");
        private static final Pattern EMAIL_PATTERN = Pattern.compile("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,253}\\.[A-Za-z]{2,63}$");

        UsersHandler(InMemoryDatabase db) {
            this.db = db;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendJson(exchange, 405, "{\"error\":\"method_not_allowed\"}");
                    return;
                }
                String path = exchange.getRequestURI().getPath();
                if (!"/users".equals(path)) {
                    sendJson(exchange, 404, "{\"error\":\"not_found\"}");
                    return;
                }
                Headers reqHeaders = exchange.getRequestHeaders();
                String contentType = firstHeader(reqHeaders, "Content-Type");
                if (contentType == null || !contentType.toLowerCase().startsWith("application/x-www-form-urlencoded")) {
                    sendJson(exchange, 415, "{\"error\":\"unsupported_media_type\"}");
                    return;
                }

                byte[] body = readBodyLimited(exchange, MAX_BODY_SIZE);
                if (body == null) {
                    sendJson(exchange, 413, "{\"error\":\"payload_too_large\"}");
                    return;
                }
                String form = new String(body, StandardCharsets.UTF_8);
                Map<String, String> fields = parseForm(form);
                if (fields == null) {
                    sendJson(exchange, 400, "{\"error\":\"invalid_form\"}");
                    return;
                }

                String username = fields.get("username");
                String email = fields.get("email");
                String ageStr = fields.get("age");

                if (!isValidUsername(username) || !isValidEmail(email) || !isValidAge(ageStr)) {
                    sendJson(exchange, 400, "{\"error\":\"invalid_input\"}");
                    return;
                }
                int age = Integer.parseInt(ageStr);

                int id = db.addUser(username, email, age);
                String resp = "{\"id\":" + id + ",\"status\":\"created\"}";
                sendJson(exchange, 201, resp);
            } catch (Exception e) {
                sendJson(exchange, 500, "{\"error\":\"internal_error\"}");
            }
        }

        private static boolean isValidUsername(String s) {
            if (s == null) return false;
            return USERNAME_PATTERN.matcher(s).matches();
        }

        private static boolean isValidEmail(String s) {
            if (s == null) return false;
            if (s.length() > 320) return false;
            return EMAIL_PATTERN.matcher(s).matches();
        }

        private static boolean isValidAge(String s) {
            if (s == null) return false;
            if (s.length() > 3) return false;
            try {
                int v = Integer.parseInt(s);
                return v >= 1 && v <= 120;
            } catch (NumberFormatException ex) {
                return false;
            }
        }

        private static String firstHeader(Headers headers, String name) {
            if (headers == null) return null;
            var list = headers.get(name);
            if (list == null || list.isEmpty()) return null;
            return list.get(0);
        }

        private static byte[] readBodyLimited(HttpExchange exchange, int max) throws IOException {
            int contentLength = 0;
            String clHeader = firstHeader(exchange.getRequestHeaders(), "Content-Length");
            if (clHeader == null) return null;
            try {
                contentLength = Integer.parseInt(clHeader.trim());
            } catch (NumberFormatException e) {
                return null;
            }
            if (contentLength < 0 || contentLength > max) return null;

            try (var is = exchange.getRequestBody();
                 var baos = new ByteArrayOutputStream(Math.min(contentLength, max))) {
                byte[] buf = new byte[1024];
                int total = 0, r;
                while (total < contentLength && (r = is.read(buf, 0, Math.min(buf.length, contentLength - total))) != -1) {
                    baos.write(buf, 0, r);
                    total += r;
                    if (total > max) return null;
                }
                if (total != contentLength) return null;
                return baos.toByteArray();
            }
        }

        private static Map<String, String> parseForm(String s) {
            LinkedHashMap<String, String> map = new LinkedHashMap<>();
            String[] pairs = s.split("&", -1);
            for (String pair : pairs) {
                if (pair.isEmpty()) continue;
                int eq = pair.indexOf('=');
                if (eq <= 0) return null;
                String keyEnc = pair.substring(0, eq);
                String valEnc = pair.substring(eq + 1);
                String key = urlDecode(keyEnc);
                String val = urlDecode(valEnc);
                if (key == null || val == null) return null;
                if (key.length() > 32 || val.length() > 256) return null;
                map.put(key, val);
            }
            return map;
        }

        private static String urlDecode(String s) {
            StringBuilder out = new StringBuilder(s.length());
            for (int i = 0; i < s.length(); i++) {
                char c = s.charAt(i);
                if (c == '+') {
                    out.append(' ');
                } else if (c == '%') {
                    if (i + 2 >= s.length()) return null;
                    int hi = hex(s.charAt(i + 1));
                    int lo = hex(s.charAt(i + 2));
                    if (hi < 0 || lo < 0) return null;
                    out.append((char) ((hi << 4) | lo));
                    i += 2;
                } else {
                    out.append(c);
                }
            }
            return out.toString();
        }

        private static int hex(char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
            if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
            return -1;
        }

        private static void sendJson(HttpExchange exchange, int status, String json) throws IOException {
            byte[] data = json.getBytes(StandardCharsets.UTF_8);
            Headers respHeaders = exchange.getResponseHeaders();
            respHeaders.set("Content-Type", "application/json; charset=utf-8");
            exchange.sendResponseHeaders(status, data.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(data);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        InMemoryDatabase db = new InMemoryDatabase();
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 0), 0);
        server.createContext("/users", new UsersHandler(db));
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
        int port = server.getAddress().getPort();
        System.out.println("Server started on port: " + port);

        // 5 test cases
        String[] tests = new String[] {
                "username=alice_01&email=alice%40example.com&age=30",
                "username=bob&email=invalid_email&age=25",
                "username=ch&email=charlie%40example.com&age=22", // short username
                "username=david12345678901234567890&email=david%40example.com&age=40", // long username
                "username=eve_ok&email=eve.ok%2Btag%40example.co&age=27"
        };

        for (int i = 0; i < tests.length; i++) {
            String body = tests[i];
            URL url = new URL("http://127.0.0.1:" + port + "/users");
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setConnectTimeout(3000);
            conn.setReadTimeout(3000);
            conn.setRequestMethod("POST");
            conn.setDoOutput(true);
            conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
            byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
            conn.setFixedLengthStreamingMode(bytes.length);
            conn.connect();
            conn.getOutputStream().write(bytes);
            int code = conn.getResponseCode();
            BufferedReader br = new BufferedReader(new InputStreamReader(
                    (code >= 200 && code < 400) ? conn.getInputStream() : conn.getErrorStream(),
                    StandardCharsets.UTF_8));
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) sb.append(line);
            br.close();
            conn.disconnect();
            System.out.println("Test " + (i + 1) + " HTTP " + code + " -> " + sb.toString());
        }

        System.out.println("Users stored: " + db.size());
        server.stop(0);
        System.out.println("Server stopped.");
    }
}