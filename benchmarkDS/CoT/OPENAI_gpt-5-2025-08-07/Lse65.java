import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.*;

public class Lse65 {
    // Configuration
    private static final int PORT = 8080;
    private static final long MAX_UPLOAD_SIZE = 10L * 1024 * 1024; // 10 MB
    private static final Path UPLOAD_DIR = Paths.get("uploads");
    private static final SecureRandom RNG = new SecureRandom();

    public static void main(String[] args) throws Exception {
        // Run 5 basic tests on helper functions (not on the HTTP stack).
        runTests();

        // Ensure uploads directory exists with proper permissions
        createUploadsDirSecure();

        // Start server
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", PORT), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null);
        System.out.println("Java server running at http://127.0.0.1:" + PORT + "/");
        server.start();
    }

    // Root handler: serves a simple HTML form for uploading PDFs
    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            setSecurityHeaders(exchange.getResponseHeaders());
            if (!exchange.getRequestMethod().equalsIgnoreCase("GET")) {
                sendPlainText(exchange, 405, "Method Not Allowed");
                return;
            }
            String html = ""
                + "<!doctype html>\n"
                + "<html lang=\"en\"><head><meta charset=\"utf-8\"><title>Upload PDF</title></head>"
                + "<body>"
                + "<h1>Upload a PDF</h1>"
                + "<form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">"
                + "<input type=\"file\" name=\"file\" accept=\"application/pdf,.pdf\" required>"
                + "<button type=\"submit\">Upload</button>"
                + "</form>"
                + "</body></html>";
            byte[] body = html.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(200, body.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(body);
            }
        }
    }

    // Upload handler: parses multipart/form-data and saves the PDF to uploads/
    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            setSecurityHeaders(exchange.getResponseHeaders());
            if (!exchange.getRequestMethod().equalsIgnoreCase("POST")) {
                sendPlainText(exchange, 405, "Method Not Allowed");
                return;
            }

            String contentType = getFirstHeader(exchange.getRequestHeaders(), "Content-Type");
            if (contentType == null || !contentType.toLowerCase(Locale.ROOT).startsWith("multipart/form-data")) {
                sendPlainText(exchange, 400, "Bad Request: expected multipart/form-data");
                return;
            }

            String boundary = extractBoundary(contentType);
            if (boundary == null || boundary.isEmpty()) {
                sendPlainText(exchange, 400, "Bad Request: missing boundary");
                return;
            }

            byte[] body;
            try (InputStream is = exchange.getRequestBody()) {
                body = readAllBytesWithLimit(is, MAX_UPLOAD_SIZE);
            } catch (IOException ex) {
                sendPlainText(exchange, 413, "Payload Too Large");
                return;
            }

            List<Part> parts;
            try {
                parts = parseMultipart(body, boundary.getBytes(StandardCharsets.ISO_8859_1));
            } catch (Exception e) {
                sendPlainText(exchange, 400, "Bad Request: invalid multipart data");
                return;
            }

            Part filePart = null;
            for (Part p : parts) {
                String cd = p.headers.getOrDefault("content-disposition", "");
                if (cd.toLowerCase(Locale.ROOT).contains("name=\"file\"") && cd.toLowerCase(Locale.ROOT).contains("filename=")) {
                    filePart = p;
                    break;
                }
            }

            if (filePart == null) {
                sendPlainText(exchange, 400, "Bad Request: file field missing");
                return;
            }

            String filename = extractFileName(filePart.headers.get("content-disposition"));
            if (filename == null || filename.isEmpty()) {
                sendPlainText(exchange, 400, "Bad Request: filename missing");
                return;
            }

            String sanitized = sanitizeFileName(filename);
            if (!sanitized.toLowerCase(Locale.ROOT).endsWith(".pdf")) {
                sendPlainText(exchange, 415, "Unsupported Media Type: only .pdf allowed");
                return;
            }

            byte[] data = filePart.data;
            if (data.length == 0) {
                sendPlainText(exchange, 400, "Bad Request: empty file");
                return;
            }

            if (!isPdfMagic(data)) {
                sendPlainText(exchange, 415, "Unsupported Media Type: invalid PDF signature");
                return;
            }

            String uniqueName = ensureUniqueName(sanitized);
            Path dest = UPLOAD_DIR.resolve(uniqueName).normalize();
            if (!dest.startsWith(UPLOAD_DIR)) { // extra defense
                sendPlainText(exchange, 400, "Bad Request: invalid path");
                return;
            }

            try (OutputStream os = Files.newOutputStream(dest, StandardOpenOption.CREATE_NEW)) {
                os.write(data);
            } catch (IOException e) {
                sendPlainText(exchange, 500, "Server Error: could not save file");
                return;
            }

            sendPlainText(exchange, 200, "Uploaded as: " + uniqueName);
        }
    }

    // Helper: set basic security headers
    private static void setSecurityHeaders(Headers headers) {
        headers.set("X-Content-Type-Options", "nosniff");
        headers.set("X-Frame-Options", "DENY");
        headers.set("Referrer-Policy", "no-referrer");
        headers.set("Content-Security-Policy", "default-src 'none'; style-src 'unsafe-inline'; form-action 'self'");
        headers.set("Cache-Control", "no-store");
    }

    private static void sendPlainText(HttpExchange exchange, int status, String msg) throws IOException {
        byte[] data = msg.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
        exchange.sendResponseHeaders(status, data.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(data);
        }
    }

    private static String getFirstHeader(Headers headers, String key) {
        List<String> vals = headers.get(key);
        if (vals == null || vals.isEmpty()) return null;
        return vals.get(0);
    }

    private static String extractBoundary(String contentType) {
        // e.g., multipart/form-data; boundary=----WebKitFormBoundaryabc123
        String[] parts = contentType.split(";");
        for (String p : parts) {
            String s = p.trim();
            if (s.toLowerCase(Locale.ROOT).startsWith("boundary=")) {
                String b = s.substring(9);
                if ((b.startsWith("\"") && b.endsWith("\"")) || (b.startsWith("'") && b.endsWith("'"))) {
                    b = b.substring(1, b.length() - 1);
                }
                return b;
            }
        }
        return null;
    }

    private static byte[] readAllBytesWithLimit(InputStream is, long limit) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        long total = 0;
        int r;
        while ((r = is.read(buf)) != -1) {
            total += r;
            if (total > limit) {
                throw new IOException("too large");
            }
            bos.write(buf, 0, r);
        }
        return bos.toByteArray();
    }

    // Multipart parsing utilities
    static class Part {
        Map<String, String> headers;
        byte[] data;
        Part(Map<String, String> headers, byte[] data) {
            this.headers = headers;
            this.data = data;
        }
    }

    private static List<Part> parseMultipart(byte[] body, byte[] boundary) throws IOException {
        List<Part> parts = new ArrayList<>();
        byte[] delimiter = concat("--".getBytes(StandardCharsets.ISO_8859_1), boundary);
        byte[] delimiterCRLF = concat("\r\n--".getBytes(StandardCharsets.ISO_8859_1), boundary);
        byte[] closeDelimiter = concat(delimiter, "--".getBytes(StandardCharsets.ISO_8859_1));

        int pos = indexOf(body, delimiter, 0);
        if (pos < 0) return parts;
        pos += delimiter.length;

        // Expect CRLF after initial delimiter
        if (!regionEquals(body, pos, "\r\n".getBytes(StandardCharsets.ISO_8859_1))) {
            // Could be end?
            if (regionEquals(body, pos, "--".getBytes(StandardCharsets.ISO_8859_1))) {
                return parts;
            }
        } else {
            pos += 2;
        }

        while (true) {
            // Parse headers until \r
\r

            int headerEnd = indexOf(body, "\r\n\r\n".getBytes(StandardCharsets.ISO_8859_1), pos);
            if (headerEnd < 0) break;
            String headerText = new String(body, pos, headerEnd - pos, StandardCharsets.ISO_8859_1);
            Map<String, String> headers = parseHeaders(headerText);
            int dataStart = headerEnd + 4;

            // Find next boundary
            int boundaryPos = indexOf(body, delimiterCRLF, dataStart);
            int closePos = indexOf(body, concat("\r\n".getBytes(StandardCharsets.ISO_8859_1), closeDelimiter), dataStart);

            int dataEnd;
            boolean isLast = false;
            if (boundaryPos < 0 && closePos < 0) {
                // maybe final boundary without preceding CRLF; try closeDelimiter without CRLF
                int closeNoCRLF = indexOf(body, closeDelimiter, dataStart);
                if (closeNoCRLF < 0) throw new IOException("boundary not found");
                dataEnd = closeNoCRLF - 2; // remove the preceding \r
 from data
                isLast = true;
            } else if (closePos >= 0 && (boundaryPos < 0 || closePos < boundaryPos)) {
                dataEnd = closePos - 2; // strip \r
 before boundary
                isLast = true;
            } else {
                dataEnd = boundaryPos - 2; // strip \r
 before boundary
            }

            if (dataEnd < dataStart) dataEnd = dataStart;
            byte[] data = Arrays.copyOfRange(body, dataStart, dataEnd);
            parts.add(new Part(headers, data));

            if (isLast) break;

            // Move pos to after delimiterCRLF and CRLF after it
            int nextStart = boundaryPos + delimiterCRLF.length;
            // Expect CRLF after boundary line
            if (regionEquals(body, nextStart, "\r\n".getBytes(StandardCharsets.ISO_8859_1))) {
                nextStart += 2;
            }
            pos = nextStart;
        }

        return parts;
    }

    private static Map<String, String> parseHeaders(String headerText) {
        Map<String, String> map = new HashMap<>();
        String[] lines = headerText.split("\r\n");
        for (String line : lines) {
            int idx = line.indexOf(':');
            if (idx > 0) {
                String name = line.substring(0, idx).trim().toLowerCase(Locale.ROOT);
                String value = line.substring(idx + 1).trim();
                map.put(name, value);
            }
        }
        return map;
    }

    private static int indexOf(byte[] haystack, byte[] needle, int from) {
        outer:
        for (int i = from; i <= haystack.length - needle.length; i++) {
            for (int j = 0; j < needle.length; j++) {
                if (haystack[i + j] != needle[j]) continue outer;
            }
            return i;
        }
        return -1;
    }

    private static boolean regionEquals(byte[] a, int off, byte[] b) {
        if (off < 0 || off + b.length > a.length) return false;
        for (int i = 0; i < b.length; i++) {
            if (a[off + i] != b[i]) return false;
        }
        return true;
    }

    private static byte[] concat(byte[] a, byte[] b) {
        byte[] r = new byte[a.length + b.length];
        System.arraycopy(a, 0, r, 0, a.length);
        System.arraycopy(b, 0, r, a.length, b.length);
        return r;
    }

    private static String extractFileName(String contentDisposition) {
        if (contentDisposition == null) return null;
        // filename="name.pdf" or filename=name.pdf
        String[] parts = contentDisposition.split(";");
        for (String p : parts) {
            String s = p.trim();
            if (s.toLowerCase(Locale.ROOT).startsWith("filename=")) {
                String val = s.substring(9).trim();
                if ((val.startsWith("\"") && val.endsWith("\"")) || (val.startsWith("'") && val.endsWith("'"))) {
                    val = val.substring(1, val.length() - 1);
                }
                return val;
            }
        }
        return null;
    }

    private static String sanitizeFileName(String input) {
        String name = input.replace('\\', '/');
        int idx = name.lastIndexOf('/');
        if (idx >= 0) name = name.substring(idx + 1);
        // Only allow safe chars
        name = name.replaceAll("[^A-Za-z0-9._-]", "_");
        // Prevent hidden files
        if (name.startsWith(".")) name = "_" + name.substring(1);
        if (name.isEmpty()) name = "file.pdf";
        return name;
    }

    private static boolean isPdfMagic(byte[] data) {
        // Allow optional leading whitespace/newlines before %PDF-
        int i = 0;
        while (i < data.length && (data[i] == 0x20 || data[i] == 0x09 || data[i] == 0x0D || data[i] == 0x0A)) i++;
        byte[] magic = "%PDF-".getBytes(StandardCharsets.US_ASCII);
        if (i + magic.length > data.length) return false;
        for (int j = 0; j < magic.length; j++) {
            if (data[i + j] != magic[j]) return false;
        }
        return true;
    }

    private static String ensureUniqueName(String base) {
        String name = base;
        String lower = base.toLowerCase(Locale.ROOT);
        String stem = base;
        String ext = "";
        int dot = lower.lastIndexOf(".pdf");
        if (dot >= 0 && dot == lower.length() - 4) {
            stem = base.substring(0, dot);
            ext = base.substring(dot);
        }
        for (int i = 0; i < 1000; i++) {
            Path p = UPLOAD_DIR.resolve(name);
            if (!Files.exists(p)) return name;
            name = stem + "-" + randomAlphaNum(6) + ext;
        }
        // last resort
        return stem + "-" + System.currentTimeMillis() + ext;
    }

    private static String randomAlphaNum(int len) {
        final String chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        StringBuilder sb = new StringBuilder(len);
        for (int i = 0; i < len; i++) {
            sb.append(chars.charAt(RNG.nextInt(chars.length())));
        }
        return sb.toString();
    }

    private static void createUploadsDirSecure() throws IOException {
        if (!Files.exists(UPLOAD_DIR)) {
            Files.createDirectories(UPLOAD_DIR);
        }
        // No specific POSIX perms here to maintain cross-platform compatibility
    }

    // 5 test cases
    private static void runTests() {
        System.out.println("Running Java tests...");
        // 1) sanitize filename traversal
        String t1 = sanitizeFileName("../etc/passwd");
        assert !t1.contains("..") && !t1.contains("/") && !t1.contains("\\");
        // 2) sanitize hidden file
        String t2 = sanitizeFileName(".hidden.pdf");
        assert !t2.startsWith(".");
        // 3) isPdfMagic true
        assert isPdfMagic("%PDF-1.7 test".getBytes(StandardCharsets.US_ASCII));
        // 4) isPdfMagic with leading whitespace
        assert isPdfMagic("\n\n\t %PDF-1.4".getBytes(StandardCharsets.US_ASCII));
        // 5) isPdfMagic false
        assert !isPdfMagic("not a pdf".getBytes(StandardCharsets.US_ASCII));
        System.out.println("All Java tests passed.");
    }
}