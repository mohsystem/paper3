import com.sun.net.httpserver.*;
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.*;

public class Lse71 {

    // Start the HTTP server
    public static HttpServer startServer(int port, String uploadDirName) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        Path uploadDir = Paths.get(uploadDirName);
        if (!Files.exists(uploadDir)) {
            Files.createDirectories(uploadDir);
        }
        server.createContext("/", new UploadHandler(uploadDir));
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        return server;
    }

    // Build multipart body for client test
    public static byte[] buildMultipartBody(String boundary, String fieldName, String filename, byte[] content) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        String partHeader = "--" + boundary + "\r\n" +
                "Content-Disposition: form-data; name=\"" + fieldName + "\"; filename=\"" + filename + "\"\r\n" +
                "Content-Type: application/octet-stream\r\n\r\n";
        baos.write(partHeader.getBytes(StandardCharsets.ISO_8859_1));
        baos.write(content);
        baos.write("\r\n".getBytes(StandardCharsets.ISO_8859_1));
        String closing = "--" + boundary + "--\r\n";
        baos.write(closing.getBytes(StandardCharsets.ISO_8859_1));
        return baos.toByteArray();
    }

    // Client-side upload test
    public static int uploadTest(String urlStr, String filename, byte[] content) throws IOException {
        String boundary = "----LSE71BOUNDARY" + System.nanoTime();
        byte[] body = buildMultipartBody(boundary, "file", filename, content);

        HttpURLConnection conn = (HttpURLConnection) new URL(urlStr).openConnection();
        conn.setDoOutput(true);
        conn.setRequestMethod("POST");
        conn.setRequestProperty("Content-Type", "multipart/form-data; boundary=" + boundary);
        conn.setRequestProperty("Content-Length", String.valueOf(body.length));
        try (OutputStream os = conn.getOutputStream()) {
            os.write(body);
            os.flush();
        }
        int code = conn.getResponseCode();
        try (InputStream is = (code >= 200 && code < 300 ? conn.getInputStream() : conn.getErrorStream())) {
            if (is != null) {
                // drain response
                byte[] buf = new byte[4096];
                while (is.read(buf) != -1) {}
            }
        }
        conn.disconnect();
        return code;
    }

    // Save uploaded file from multipart/form-data body
    public static Path saveUploadedFile(byte[] bodyBytes, String contentTypeHeader, Path uploadDir) throws IOException {
        if (contentTypeHeader == null || !contentTypeHeader.contains("multipart/form-data")) {
            throw new IOException("Invalid Content-Type");
        }
        String boundary = null;
        for (String part : contentTypeHeader.split(";")) {
            part = part.trim();
            if (part.toLowerCase(Locale.ROOT).startsWith("boundary=")) {
                boundary = part.substring(part.indexOf("=") + 1).trim();
                if (boundary.startsWith("\"") && boundary.endsWith("\"")) {
                    boundary = boundary.substring(1, boundary.length() - 1);
                }
                break;
            }
        }
        if (boundary == null || boundary.isEmpty()) throw new IOException("Boundary not found");

        String data = new String(bodyBytes, StandardCharsets.ISO_8859_1);
        String boundaryMarker = "--" + boundary;
        int pos = 0;
        while (true) {
            int partStart = data.indexOf(boundaryMarker, pos);
            if (partStart == -1) break;
            partStart += boundaryMarker.length();
            if (partStart + 2 <= data.length() && data.startsWith("--", partStart)) break; // end
            if (partStart + 2 > data.length() || !data.startsWith("\r\n", partStart)) break;
            int headersStart = partStart + 2;
            int headersEnd = data.indexOf("\r\n\r\n", headersStart);
            if (headersEnd == -1) break;
            String headersBlock = data.substring(headersStart, headersEnd);
            Map<String, String> headers = new HashMap<>();
            for (String line : headersBlock.split("\r\n")) {
                int idx = line.indexOf(':');
                if (idx > 0) {
                    headers.put(line.substring(0, idx).trim().toLowerCase(Locale.ROOT), line.substring(idx + 1).trim());
                }
            }
            String cd = headers.get("content-disposition");
            if (cd != null && cd.toLowerCase(Locale.ROOT).contains("form-data")) {
                String filename = null;
                for (String token : cd.split(";")) {
                    token = token.trim();
                    if (token.toLowerCase(Locale.ROOT).startsWith("filename=")) {
                        filename = token.substring(token.indexOf('=') + 1).trim();
                        if (filename.startsWith("\"") && filename.endsWith("\"") && filename.length() >= 2) {
                            filename = filename.substring(1, filename.length() - 1);
                        }
                        break;
                    }
                }
                int contentStart = headersEnd + 4;
                int contentEnd = data.indexOf("\r\n" + boundaryMarker, contentStart);
                if (contentEnd == -1) contentEnd = data.length();
                if (filename != null && !filename.isEmpty()) {
                    String safeName = filename.replace("\\", "/");
                    int slash = safeName.lastIndexOf('/');
                    if (slash >= 0) safeName = safeName.substring(slash + 1);
                    if (safeName.isEmpty()) safeName = "upload.bin";
                    // ensure unique
                    Path target = uploadDir.resolve(safeName);
                    if (Files.exists(target)) {
                        String base = safeName;
                        String ext = "";
                        int dot = safeName.lastIndexOf('.');
                        if (dot > 0) {
                            base = safeName.substring(0, dot);
                            ext = safeName.substring(dot);
                        }
                        int counter = 1;
                        while (Files.exists(target)) {
                            target = uploadDir.resolve(base + "_" + counter + ext);
                            counter++;
                        }
                    }
                    byte[] fileBytes = Arrays.copyOfRange(bodyBytes, contentStart, contentEnd);
                    Files.write(target, fileBytes);
                    return target;
                }
            }
            pos = headersEnd + 4;
        }
        throw new IOException("No file part found");
    }

    static class UploadHandler implements HttpHandler {
        private final Path uploadDir;
        UploadHandler(Path uploadDir) {
            this.uploadDir = uploadDir;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String method = exchange.getRequestMethod();
            if ("GET".equalsIgnoreCase(method)) {
                String html = "<!doctype html><html><body>" +
                        "<h3>Upload File</h3>" +
                        "<form method='POST' enctype='multipart/form-data'>" +
                        "<input type='file' name='file' />" +
                        "<button type='submit'>Upload</button></form>" +
                        "</body></html>";
                byte[] bytes = html.getBytes(StandardCharsets.UTF_8);
                exchange.getResponseHeaders().set("Content-Type", "text/html; charset=utf-8");
                exchange.sendResponseHeaders(200, bytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(bytes);
                }
                return;
            } else if ("POST".equalsIgnoreCase(method)) {
                Headers reqHeaders = exchange.getRequestHeaders();
                String contentType = reqHeaders.getFirst("Content-Type");
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                try (InputStream is = exchange.getRequestBody()) {
                    byte[] buf = new byte[8192];
                    int n;
                    while ((n = is.read(buf)) != -1) {
                        baos.write(buf, 0, n);
                    }
                }
                String response;
                int code = 200;
                try {
                    Path saved = saveUploadedFile(baos.toByteArray(), contentType, uploadDir);
                    response = "Uploaded to: " + saved.toAbsolutePath();
                } catch (Exception ex) {
                    code = 400;
                    response = "Upload failed: " + ex.getMessage();
                }
                byte[] respBytes = response.getBytes(StandardCharsets.UTF_8);
                exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
                exchange.sendResponseHeaders(code, respBytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(respBytes);
                }
                return;
            } else {
                exchange.sendResponseHeaders(405, -1);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // Start server
        HttpServer server = startServer(8080, "uploads_java");

        // Give server a moment to start
        Thread.sleep(500);

        // 5 test cases: upload different files
        List<byte[]> contents = new ArrayList<>();
        contents.add("Hello World 1".getBytes(StandardCharsets.UTF_8));
        contents.add("Second file content\nLine2".getBytes(StandardCharsets.UTF_8));
        contents.add(new byte[] {0x00, 0x01, 0x02, 0x03, 0x7F}); // binary
        contents.add("".getBytes(StandardCharsets.UTF_8)); // empty file
        contents.add("最后一个测试".getBytes(StandardCharsets.UTF_8)); // unicode

        for (int i = 0; i < contents.size(); i++) {
            String fname = "test" + (i + 1) + (i == 2 ? ".bin" : ".txt");
            int code = uploadTest("http://localhost:8080/", fname, contents.get(i));
            // Optionally print status
            // System.out.println("Upload " + fname + " -> HTTP " + code);
        }

        // Server continues running. Uncomment to stop:
        // server.stop(0);
    }
}