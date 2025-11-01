import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URL;
import java.net.URLEncoder;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Lse71 {

    private static final int PORT = 8080;
    private static final long MAX_SIZE = 10L * 1024 * 1024; // 10 MB
    private static final SecureRandom RNG = new SecureRandom();
    private static volatile HttpServer serverRef = null;

    // Validate file name: [A-Za-z0-9._-], 1..100 chars
    public static String sanitizeName(String name) {
        if (name == null) return null;
        if (name.length() < 1 || name.length() > 100) return null;
        for (int i = 0; i < name.length(); i++) {
            char c = name.charAt(i);
            if (!(c >= 'a' && c <= 'z') &&
                !(c >= 'A' && c <= 'Z') &&
                !(c >= '0' && c <= '9') &&
                c != '.' && c != '_' && c != '-') {
                return null;
            }
        }
        // Reject traversal-like names
        if (name.contains("..")) return null;
        if (name.startsWith(".")) return null;
        return name;
    }

    public static Path ensureUploadDir() throws IOException {
        Path base = Paths.get("uploads");
        Files.createDirectories(base);
        // Try to ensure directory exists and is not a symlink
        Path real = base.toRealPath(LinkOption.NOFOLLOW_LINKS);
        if (!Files.isDirectory(real, LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Upload base is not a directory");
        }
        return real;
    }

    public static String genStoredName(String baseName) {
        String ext = "";
        int idx = baseName.lastIndexOf('.');
        if (idx > 0 && idx < baseName.length() - 1) {
            String e = baseName.substring(idx + 1);
            boolean ok = true;
            if (e.length() > 10) ok = false;
            for (int i = 0; i < e.length(); i++) {
                char c = e.charAt(i);
                if (!(c >= 'a' && c <= 'z') &&
                    !(c >= 'A' && c <= 'Z') &&
                    !(c >= '0' && c <= '9')) {
                    ok = false; break;
                }
            }
            if (ok) ext = "." + e;
        }
        byte[] rand = new byte[4];
        RNG.nextBytes(rand);
        StringBuilder sb = new StringBuilder();
        for (byte b : rand) {
            sb.append(String.format("%02x", b));
        }
        long ts = Instant.now().toEpochMilli();
        String prefix = baseName;
        if (idx > 0) prefix = baseName.substring(0, idx);
        return prefix + "-" + ts + "-" + sb + ext;
    }

    public static void startServer() throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", PORT), 50);
        ExecutorService executor = Executors.newFixedThreadPool(4);
        server.setExecutor(executor);
        Path base = ensureUploadDir();
        server.createContext("/upload", new UploadHandler(base));
        server.start();
        serverRef = server;
    }

    public static void stopServer() {
        HttpServer s = serverRef;
        if (s != null) {
            s.stop(0);
        }
        serverRef = null;
    }

    static class UploadHandler implements HttpHandler {
        private final Path base;

        UploadHandler(Path base) {
            this.base = base;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    send(exchange, 405, "Method Not Allowed\n");
                    return;
                }
                URI uri = exchange.getRequestURI();
                String query = uri.getRawQuery();
                Map<String, String> params = parseQuery(query);
                String rawName = params.get("name");
                String safeName = sanitizeName(rawName);
                if (safeName == null) {
                    send(exchange, 400, "Bad Request\n");
                    return;
                }

                Headers headers = exchange.getRequestHeaders();
                String cl = headers.getFirst("Content-Length");
                if (cl == null) {
                    send(exchange, 411, "Length Required\n");
                    return;
                }
                long length;
                try {
                    length = Long.parseLong(cl);
                } catch (NumberFormatException e) {
                    send(exchange, 400, "Bad Request\n");
                    return;
                }
                if (length < 0 || length > MAX_SIZE) {
                    send(exchange, 413, "Payload Too Large\n");
                    return;
                }

                String stored = genStoredName(safeName);
                Path out = base.resolve(stored).normalize();
                if (!out.startsWith(base)) {
                    send(exchange, 400, "Bad Request\n");
                    return;
                }

                // Create new file exclusively
                Set<OpenOption> opts = new HashSet<>(Arrays.asList(StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE));
                try (OutputStream fos = Files.newOutputStream(out, opts.toArray(new OpenOption[0]));
                     FileChannel chan = (fos instanceof FileOutputStream) ? ((FileOutputStream) fos).getChannel() : null;
                     InputStream is = exchange.getRequestBody()) {

                    // Try to set restrictive perms on POSIX
                    try {
                        Set<PosixFilePermission> perms = EnumSet.of(PosixFilePermission.OWNER_READ, PosixFilePermission.OWNER_WRITE);
                        Files.setPosixFilePermissions(out, perms);
                    } catch (UnsupportedOperationException ignored) {
                        // Non-POSIX FS; ignore
                    }

                    byte[] buf = new byte[8192];
                    long remaining = length;
                    while (remaining > 0) {
                        int toRead = (int) Math.min(buf.length, remaining);
                        int r = is.read(buf, 0, toRead);
                        if (r == -1) break;
                        fos.write(buf, 0, r);
                        remaining -= r;
                    }
                    if (remaining != 0) {
                        // client sent less than Content-Length
                        try { Files.deleteIfExists(out); } catch (IOException ignored) {}
                        send(exchange, 400, "Bad Request\n");
                        return;
                    }
                    if (chan != null) {
                        chan.force(true); // fsync file
                    }
                } catch (FileAlreadyExistsException e) {
                    send(exchange, 409, "Conflict\n");
                    return;
                } catch (IOException ioe) {
                    // cleanup
                    try { Files.deleteIfExists(out); } catch (IOException ignored) {}
                    send(exchange, 500, "Internal Server Error\n");
                    return;
                }

                String resp = "{\"stored\":\"" + stored + "\"}\n";
                send(exchange, 201, resp);
            } catch (Exception e) {
                send(exchange, 500, "Internal Server Error\n");
            }
        }

        private Map<String, String> parseQuery(String q) {
            Map<String, String> map = new HashMap<>();
            if (q == null || q.isEmpty()) return map;
            String[] parts = q.split("&");
            for (String p : parts) {
                int idx = p.indexOf('=');
                if (idx <= 0) continue;
                String k = urlDecode(p.substring(0, idx));
                String v = urlDecode(p.substring(idx + 1));
                map.put(k, v);
            }
            return map;
        }

        private String urlDecode(String s) {
            try {
                return java.net.URLDecoder.decode(s, StandardCharsets.UTF_8.name());
            } catch (Exception e) {
                return "";
            }
        }

        private void send(HttpExchange ex, int code, String body) throws IOException {
            byte[] data = body.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            ex.sendResponseHeaders(code, data.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(data);
            }
        }
    }

    private static int httpPost(String name, byte[] data) throws Exception {
        String url = "http://127.0.0.1:" + PORT + "/upload?name=" + URLEncoder.encode(name, "UTF-8");
        HttpURLConnection conn = (HttpURLConnection) new URL(url).openConnection();
        conn.setRequestMethod("POST");
        conn.setDoOutput(true);
        conn.setFixedLengthStreamingMode(data.length);
        conn.setRequestProperty("Content-Type", "application/octet-stream");
        conn.connect();
        try (OutputStream os = conn.getOutputStream()) {
            os.write(data);
        }
        int code = conn.getResponseCode();
        InputStream is = (code >= 200 && code < 300) ? conn.getInputStream() : conn.getErrorStream();
        if (is != null) {
            ByteArrayOutputStream bout = new ByteArrayOutputStream();
            byte[] buf = new byte[1024];
            int r;
            while ((r = is.read(buf)) != -1) bout.write(buf, 0, r);
            // optionally print response
        }
        conn.disconnect();
        return code;
    }

    public static void main(String[] args) throws Exception {
        startServer();
        Thread.sleep(300); // give server time to bind

        // 5 test cases
        int ok = 0;
        try {
            // 1) Small valid upload
            ok += (httpPost("hello.txt", "Hello".getBytes(StandardCharsets.UTF_8)) == 201) ? 1 : 0;
            // 2) Invalid name (traversal)
            ok += (httpPost("../evil", "data".getBytes(StandardCharsets.UTF_8)) == 400) ? 1 : 0;
            // 3) Empty file
            ok += (httpPost("empty.bin", new byte[0]) == 201) ? 1 : 0;
            // 4) Too large
            byte[] big = new byte[(int) (MAX_SIZE + 1)];
            ok += (httpPost("big.bin", big) == 413) ? 1 : 0;
            // 5) Another valid file
            byte[] payload = new byte[1024];
            RNG.nextBytes(payload);
            ok += (httpPost("rand.dat", payload) == 201) ? 1 : 0;
        } finally {
            stopServer();
        }
        System.out.println("Tests passed: " + ok + "/5");
    }
}