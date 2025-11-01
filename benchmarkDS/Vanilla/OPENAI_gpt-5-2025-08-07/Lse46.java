import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;
import java.util.Arrays;

public class Lse46 {

    // img() gets a file name and returns a file object (InputStream)
    public static InputStream img(String name) throws IOException {
        File file = new File("images", name);
        return new BufferedInputStream(new FileInputStream(file));
    }

    // Optional: start a minimal HTTP server mapping /img to img()
    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/img", (HttpExchange exchange) -> {
            String query = exchange.getRequestURI().getQuery();
            String name = null;
            if (query != null) {
                for (String pair : query.split("&")) {
                    int idx = pair.indexOf('=');
                    if (idx >= 0) {
                        String key = pair.substring(0, idx);
                        String val = pair.substring(idx + 1);
                        if ("name".equals(key)) {
                            name = URLDecoder.decode(val, StandardCharsets.UTF_8.name());
                            break;
                        }
                    }
                }
            }
            if (name == null || name.isEmpty()) {
                byte[] msg = "Missing 'name' parameter".getBytes(StandardCharsets.UTF_8);
                exchange.sendResponseHeaders(400, msg.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(msg);
                }
                return;
            }
            File file = new File("images", name);
            if (!file.exists() || !file.isFile()) {
                exchange.sendResponseHeaders(404, -1);
                exchange.close();
                return;
            }
            try (InputStream in = img(name)) {
                exchange.getResponseHeaders().add("Content-Type", "application/octet-stream");
                exchange.sendResponseHeaders(200, file.length());
                try (OutputStream os = exchange.getResponseBody()) {
                    byte[] buf = new byte[8192];
                    int r;
                    while ((r = in.read(buf)) != -1) {
                        os.write(buf, 0, r);
                    }
                }
            } catch (IOException e) {
                exchange.sendResponseHeaders(500, -1);
                exchange.close();
            }
        });
        server.setExecutor(null);
        server.start();
    }

    // Helper to create sample files for tests
    private static void ensureSampleFiles() throws IOException {
        Files.createDirectories(Paths.get("images"));
        String[] names = {"t1.txt", "t2.txt", "t3.txt", "t4.txt", "t5.txt"};
        String[] contents = {
            "alpha",
            "bravo charlie",
            "delta echo foxtrot",
            "golf hotel india juliet",
            "kilo lima mike november oscar"
        };
        for (int i = 0; i < names.length; i++) {
            Path p = Paths.get("images", names[i]);
            Files.write(p, contents[i].getBytes(StandardCharsets.UTF_8));
        }
    }

    public static void main(String[] args) throws Exception {
        ensureSampleFiles();
        String[] tests = {"t1.txt", "t2.txt", "t3.txt", "t4.txt", "t5.txt"};
        for (String name : tests) {
            try (InputStream in = img(name)) {
                byte[] data = in.readAllBytes();
                System.out.println("Read " + data.length + " bytes from images/" + name + " -> " + Arrays.toString(Arrays.copyOf(data, Math.min(16, data.length))));
            }
        }
        // Optionally start the HTTP server:
        // startServer(8080);
        // Then access: http://localhost:8080/img?name=t1.txt
    }
}