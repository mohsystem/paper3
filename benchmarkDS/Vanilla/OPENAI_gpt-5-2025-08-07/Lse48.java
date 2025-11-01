import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;

public class Lse48 {

    // img() function: accepts a file path and returns file content as bytes
    public static byte[] img(String filePath) throws IOException {
        return Files.readAllBytes(Paths.get(filePath));
    }

    // Minimal HTTP server with /img route that uses img()
    static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/img", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                try {
                    if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                        exchange.sendResponseHeaders(405, -1);
                        return;
                    }
                    String query = exchange.getRequestURI().getQuery();
                    String fileParam = null;
                    if (query != null) {
                        for (String kv : query.split("&")) {
                            int idx = kv.indexOf('=');
                            if (idx >= 0) {
                                String key = URLDecoder.decode(kv.substring(0, idx), StandardCharsets.UTF_8);
                                String val = URLDecoder.decode(kv.substring(idx + 1), StandardCharsets.UTF_8);
                                if ("file".equals(key)) {
                                    fileParam = val;
                                    break;
                                }
                            }
                        }
                    }
                    if (fileParam == null || fileParam.isEmpty()) {
                        byte[] msg = "Missing 'file' parameter".getBytes(StandardCharsets.UTF_8);
                        exchange.sendResponseHeaders(400, msg.length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(msg);
                        }
                        return;
                    }
                    byte[] content;
                    try {
                        content = img(fileParam);
                    } catch (IOException ioe) {
                        byte[] msg = ("File not found or unreadable: " + ioe.getMessage())
                                .getBytes(StandardCharsets.UTF_8);
                        exchange.sendResponseHeaders(404, msg.length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(msg);
                        }
                        return;
                    }
                    exchange.getResponseHeaders().add("Content-Type", "application/octet-stream");
                    exchange.sendResponseHeaders(200, content.length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(content);
                    }
                } catch (Exception e) {
                    byte[] msg = ("Server error: " + e.getMessage()).getBytes(StandardCharsets.UTF_8);
                    exchange.sendResponseHeaders(500, msg.length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(msg);
                    }
                } finally {
                    exchange.close();
                }
            }
        });
        server.setExecutor(null);
        server.start();
        System.out.println("Java HTTP server started on http://localhost:" + port + "/img?file=<path>");
    }

    public static void main(String[] args) throws Exception {
        // Create 5 test files
        List<String> files = new ArrayList<>();
        files.add("j_test1.bin");
        files.add("j_test2.bin");
        files.add("j_test3.bin");
        files.add("j_test4.bin");
        files.add("j_test5.bin");

        Files.write(Paths.get(files.get(0)), "Hello Java".getBytes(StandardCharsets.UTF_8));
        Files.write(Paths.get(files.get(1)), new byte[]{0x00, 0x01, 0x02, (byte) 0xFF, 0x10});
        Files.write(Paths.get(files.get(2)), "1234567890".repeat(5).getBytes(StandardCharsets.UTF_8));
        Files.write(Paths.get(files.get(3)), new byte[0]);
        byte[] big = new byte[1024];
        new Random(42).nextBytes(big);
        Files.write(Paths.get(files.get(4)), big);

        // 5 test cases calling img()
        for (String f : files) {
            byte[] data = img(f);
            System.out.println("Read " + f + " -> length=" + data.length +
                    (data.length > 8 ? ", first8=" + Arrays.toString(Arrays.copyOf(data, 8)) : ""));
        }

        // Start server to serve /img route
        startServer(8080);
    }
}