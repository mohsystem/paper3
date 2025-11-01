import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpExchange;
import java.net.InetSocketAddress;
import java.net.HttpURLConnection;
import java.net.URL;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.URI;
import java.util.HashMap;
import java.util.Map;

public class Lse89 {

    public static double priceForCode(String code) {
        if (code == null) return 0.0;
        switch (code) {
            case "A100": return 10.0;
            case "B200": return 20.0;
            case "C300": return 7.5;
            case "D400": return 15.25;
            case "E500": return 5.75;
            default: return 0.0;
        }
    }

    public static double totalPrice(String code, int qty) {
        return priceForCode(code) * Math.max(0, qty);
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new HashMap<>();
        if (query == null || query.isEmpty()) return map;
        String[] parts = query.split("&");
        for (String p : parts) {
            int i = p.indexOf('=');
            if (i >= 0) {
                String k = decode(p.substring(0, i));
                String v = decode(p.substring(i + 1));
                map.put(k, v);
            } else {
                map.put(decode(p), "");
            }
        }
        return map;
    }

    private static String decode(String s) {
        try {
            return java.net.URLDecoder.decode(s, "UTF-8");
        } catch (Exception e) {
            return s;
        }
    }

    private static void handlePrice(HttpExchange exchange) throws IOException {
        URI uri = exchange.getRequestURI();
        Map<String, String> query = parseQuery(uri.getQuery());
        String code = query.getOrDefault("code", "");
        int qty = 0;
        try {
            qty = Integer.parseInt(query.getOrDefault("qty", "0"));
        } catch (NumberFormatException e) {
            qty = 0;
        }
        double unit = priceForCode(code);
        double total = unit * Math.max(0, qty);
        String json = String.format("{\"code\":\"%s\",\"unitPrice\":%.2f,\"qty\":%d,\"total\":%.2f}",
                code, unit, qty, total);
        byte[] bytes = json.getBytes("UTF-8");
        exchange.getResponseHeaders().add("Content-Type", "application/json; charset=utf-8");
        exchange.sendResponseHeaders(200, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }

    private static HttpServer startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/price", Lse89::handlePrice);
        server.setExecutor(null);
        server.start();
        return server;
    }

    private static String httpGet(String urlStr) throws IOException {
        URL url = new URL(urlStr);
        HttpURLConnection c = (HttpURLConnection) url.openConnection();
        c.setRequestMethod("GET");
        c.setConnectTimeout(2000);
        c.setReadTimeout(2000);
        try (InputStream is = c.getInputStream();
             ByteArrayOutputStream bos = new ByteArrayOutputStream()) {
            byte[] buf = new byte[4096];
            int r;
            while ((r = is.read(buf)) != -1) bos.write(buf, 0, r);
            return bos.toString("UTF-8");
        } finally {
            c.disconnect();
        }
    }

    public static void main(String[] args) throws Exception {
        int port = 5000;
        HttpServer server = startServer(port);

        // 5 test cases: calling endpoint
        String base = "http://127.0.0.1:" + port;
        String[] tests = new String[] {
            "/price?code=A100&qty=3",
            "/price?code=B200&qty=2",
            "/price?code=C300&qty=5",
            "/price?code=D400&qty=1",
            "/price?code=Z999&qty=4"
        };

        // Wait briefly for server to fully start
        Thread.sleep(200);

        for (String t : tests) {
            String resp = httpGet(base + t);
            System.out.println("GET " + t + " -> " + resp);
        }

        // Also demonstrate the function directly
        System.out.println("Direct priceForCode('A100'): " + priceForCode("A100"));
        System.out.println("Direct totalPrice('B200', 3): " + totalPrice("B200", 3));

        // Stop server after tests
        server.stop(0);
    }
}