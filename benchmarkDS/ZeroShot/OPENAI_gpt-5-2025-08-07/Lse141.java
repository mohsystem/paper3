import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.time.Instant;
import java.util.Objects;

public class Lse141 {

    public static String getServerName(String defaultValue) {
        String v = System.getenv("SERVER_NAME");
        return (v != null && !v.isBlank()) ? v : defaultValue;
    }

    public static String getDbName(String defaultValue) {
        String v = System.getenv("DB_NAME");
        return (v != null && !v.isBlank()) ? v : defaultValue;
    }

    public static String getDbUser(String defaultValue) {
        String v = System.getenv("DB_USER");
        return (v != null && !v.isBlank()) ? v : defaultValue;
    }

    public static String getDbPassword(String defaultValue) {
        String v = System.getenv("DB_PASSWORD");
        return (v != null && !v.isBlank()) ? v : defaultValue;
    }

    public static Connection getDbConnection(String jdbcUrl, String user, String password) {
        if (jdbcUrl == null || !jdbcUrl.startsWith("jdbc:")) {
            return null;
        }
        try {
            DriverManager.setLoginTimeout(5);
            return DriverManager.getConnection(jdbcUrl, user, password);
        } catch (SQLException e) {
            return null;
        }
    }

    private static int getSafePort(int defaultPort) {
        String p = System.getenv("PORT");
        int port = defaultPort;
        if (p != null) {
            try {
                int candidate = Integer.parseInt(p.trim());
                if (candidate >= 1024 && candidate <= 65535) {
                    port = candidate;
                }
            } catch (NumberFormatException ignored) {
            }
        }
        return port;
    }

    private static String jsonEscape(String s) {
        if (s == null) return "null";
        StringBuilder sb = new StringBuilder("\"");
        for (char c : s.toCharArray()) {
            switch (c) {
                case '\\': sb.append("\\\\"); break;
                case '"': sb.append("\\\""); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x", (int)c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        sb.append("\"");
        return sb.toString();
    }

    private static String mask(String s) {
        if (s == null || s.isEmpty()) return "";
        return "*".repeat(Math.min(12, s.length()));
    }

    private static void addSecurityHeaders(Headers headers) {
        headers.add("X-Content-Type-Options", "nosniff");
        headers.add("X-Frame-Options", "DENY");
        headers.add("X-XSS-Protection", "0");
        headers.add("Content-Security-Policy", "default-src 'none'; frame-ancestors 'none'; base-uri 'none';");
        headers.add("Referrer-Policy", "no-referrer");
        headers.add("Cache-Control", "no-store");
    }

    private static void startServer() throws IOException {
        int port = getSafePort(8080);
        HttpServer server = HttpServer.create(new InetSocketAddress("0.0.0.0", port), 0);

        server.createContext("/", exchange -> {
            if (!Objects.equals(exchange.getRequestMethod(), "GET")) {
                exchange.sendResponseHeaders(405, -1);
                exchange.close();
                return;
            }
            addSecurityHeaders(exchange.getResponseHeaders());
            byte[] body = "Lse141 Java web app is running".getBytes();
            exchange.getResponseHeaders().add("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(200, body.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(body);
            }
        });

        server.createContext("/health", exchange -> {
            if (!Objects.equals(exchange.getRequestMethod(), "GET")) {
                exchange.sendResponseHeaders(405, -1);
                exchange.close();
                return;
            }
            addSecurityHeaders(exchange.getResponseHeaders());
            String resp = "{\"status\":\"ok\",\"ts\":\"" + Instant.now() + "\"}";
            byte[] body = resp.getBytes();
            exchange.getResponseHeaders().add("Content-Type", "application/json; charset=utf-8");
            exchange.sendResponseHeaders(200, body.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(body);
            }
        });

        server.createContext("/config", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!Objects.equals(exchange.getRequestMethod(), "GET")) {
                    exchange.sendResponseHeaders(405, -1);
                    exchange.close();
                    return;
                }
                addSecurityHeaders(exchange.getResponseHeaders());
                String serverName = getServerName("localhost");
                String dbName = getDbName("appdb");
                String dbUser = getDbUser("appuser");
                String dbPass = getDbPassword("");
                String json = "{"
                        + "\"serverName\":" + jsonEscape(serverName) + ","
                        + "\"dbName\":" + jsonEscape(dbName) + ","
                        + "\"dbUser\":" + jsonEscape(dbUser) + ","
                        + "\"dbPasswordMasked\":" + jsonEscape(mask(dbPass))
                        + "}";
                byte[] body = json.getBytes();
                exchange.getResponseHeaders().add("Content-Type", "application/json; charset=utf-8");
                exchange.sendResponseHeaders(200, body.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(body);
                }
            }
        });

        server.setExecutor(null);
        server.start();
        System.out.println("Java server started on port " + port);
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        String t1 = getServerName("localhost");
        String t2 = getDbName("defaultdb");
        String t3 = getDbUser("defaultuser");
        String t4 = getDbPassword("defaultpass");
        Connection t5 = getDbConnection(
                System.getenv("JDBC_URL") != null ? System.getenv("JDBC_URL") : "jdbc:invalid://",
                t3,
                t4
        );

        System.out.println("Test1 serverName: " + t1);
        System.out.println("Test2 dbName: " + t2);
        System.out.println("Test3 dbUser: " + t3);
        System.out.println("Test4 dbPasswordMasked: " + mask(t4));
        System.out.println("Test5 connection established: " + (t5 != null));

        if (t5 != null) {
            try {
                t5.close();
            } catch (SQLException ignored) {}
        }

        startServer();
    }
}