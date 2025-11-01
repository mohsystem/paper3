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

public class Lse71 {

    // 1) Problem understanding:
    // - Provide a simple web server to accept file uploads.
    // - Input: HTTP requests (GET to render form, POST multipart/form-data to upload).
    // - Output: HTML (form) or JSON with upload result.
    // - Operations: validate, sanitize, limit size, check content signatures, write to disk.

    // 2) Security requirements:
    // - Limit upload size to prevent DoS.
    // - Sanitize filenames and prevent path traversal.
    // - Validate extension whitelist and check basic magic bytes where applicable.
    // - Store files with randomized names.
    // - Use a dedicated upload directory with safe permissions.
    // - Do not trust Content-Type header blindly.

    // Constants
    private static final int PORT = 8080;
    private static final long MAX_UPLOAD_SIZE = 5L * 1024 * 1024; // 5 MB
    private static final Set<String> ALLOWED_EXT = Set.of("txt", "png", "jpg", "jpeg", "pdf");
    private static final Path UPLOAD_DIR = Paths.get("uploads");
    private static final SecureRandom RNG = new SecureRandom();

    // 3) Secure coding generation: helper functions

    // Reads up to maxBytes from stream. Throws if exceeds.
    public static byte[] readAll(InputStream in, long maxBytes) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        long total = 0;
        int r;
        while ((r = in.read(buf)) != -1) {
            total += r;
            if (total > maxBytes) {
                throw new IOException("Payload too large");
            }
            bos.write(buf, 0, r);
        }
        return bos.toByteArray();
    }

    // Sanitize filename: keep only safe chars; remove path separators.
    public static String sanitizeFilename(String filename) {
        if (filename == null) return "file";
        String base = filename.replace("\\", "/");
        int idx = base.lastIndexOf('/');
        if (idx >= 0) base = base.substring(idx + 1);
        // remove any null bytes and control chars
        StringBuilder sb = new StringBuilder();
        for (char c : base.toCharArray()) {
            if (c >= 32 && c <= 126) {
                if (Character.isLetterOrDigit(c) || c == '.' || c == '_' || c == '-' ) {
                    sb.append(c);
                } else {
                    // skip other punctuation for safety
                }
            }
        }
        String clean = sb.toString();
        if (clean.isEmpty()) clean = "file";
        // avoid dotfiles
        if (clean.startsWith(".")) clean = "file" + clean;
        return clean;
    }

    public static String getExtensionLower(String filename) {
        String f = filename == null ? "" : filename;
        int dot = f.lastIndexOf('.');
        if (dot >= 0 && dot < f.length() - 1) {
            return f.substring(dot + 1).toLowerCase(Locale.ROOT);
        }
        return "";
    }

    public static boolean isAllowedExtension(String filename) {
        String ext = getExtensionLower(filename);
        return ALLOWED_EXT.contains(ext);
    }

    // Basic content sniffing based on extension.
    public static boolean isSafeContent(String filename, byte[] data) {
        String ext = getExtensionLower(filename);
        if (data == null) return false;
        if (ext.equals("png")) {
            byte[] sig = new byte[]{(byte)0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
            if (data.length < sig.length) return false;
            for (int i = 0; i < sig.length; i++) if (data[i] != sig[i]) return false;
            return true;
        } else if (ext.equals("jpg") || ext.equals("jpeg")) {
            if (data.length < 3) return false;
            boolean start = (data[0] & 0xFF) == 0xFF && (data[1] & 0xFF) == 0xD8;
            return start;
        } else if (ext.equals("pdf")) {
            byte[] sig = "%PDF-".getBytes(StandardCharsets.US_ASCII);
            if (data.length < sig.length) return false;
            for (int i = 0; i < sig.length; i++) if (data[i] != sig[i]) return false;
            return true;
        } else if (ext.equals("txt")) {
            // verify printable / whitespace
            int allowance = Math.min(data.length, 4096);
            for (int i = 0; i < allowance; i++) {
                int b = data[i] & 0xFF;
                if (b == 0) return false;
                if (b < 9) return false;
                if (b > 126 && b < 160) return false;
            }
            return true;
        } else {
            return false;
        }
    }

    // Random safe storage name preserving extension
    public static String randomStoredName(String filename) {
        byte[] r = new byte[16];
        RNG.nextBytes(r);
        StringBuilder sb = new StringBuilder();
        for (byte b : r) sb.append(String.format("%02x", b));
        String ext = getExtensionLower(filename);
        if (!ext.isEmpty()) sb.append('.').append(ext);
        return sb.toString();
    }

    // Build a minimal HTML upload form
    public static byte[] buildFormPage() {
        String html = "<!doctype html><html><head><meta charset='utf-8'><title>Secure Upload</title></head>" +
                "<body><h1>Upload a file</h1>" +
                "<form method='POST' action='/upload' enctype='multipart/form-data'>" +
                "<input type='file' name='file' required>" +
                "<button type='submit'>Upload</button>" +
                "</form></body></html>";
        return html.getBytes(StandardCharsets.UTF_8);
    }

    // Parse multipart/form-data body into parts
    public static List<Part> parseMultipart(byte[] body, String contentType) throws IOException {
        if (contentType == null) throw new IOException("Missing Content-Type");
        String lower = contentType.toLowerCase(Locale.ROOT);
        if (!lower.startsWith("multipart/form-data")) throw new IOException("Not multipart");
        String boundary = null;
        for (String part : contentType.split(";")) {
            part = part.trim();
            if (part.toLowerCase(Locale.ROOT).startsWith("boundary=")) {
                boundary = part.substring(9);
                if (boundary.startsWith("\"") && boundary.endsWith("\"") && boundary.length() >= 2) {
                    boundary = boundary.substring(1, boundary.length() - 1);
                }
            }
        }
        if (boundary == null || boundary.isEmpty()) throw new IOException("Boundary not found");
        byte[] dashBoundary = ("--" + boundary).getBytes(StandardCharsets.ISO_8859_1);
        byte[] dashBoundaryFinal = ("--" + boundary + "--").getBytes(StandardCharsets.ISO_8859_1);
        int pos = 0;
        List<Part> parts = new ArrayList<>();
        while (true) {
            int start = indexOf(body, dashBoundary, pos);
            if (start < 0) break;
            int afterBoundary = start + dashBoundary.length;
            // Expect CRLF
            if (!matchCrlf(body, afterBoundary)) {
                if (indexOf(body, dashBoundaryFinal, pos) == start) break;
                afterBoundary = afterBoundary; // continue anyway
            } else {
                afterBoundary += 2;
            }
            int headerEnd = indexOf(body, new byte[]{'\r', '\n', '\r', '\n'}, afterBoundary);
            if (headerEnd < 0) break;
            String headersStr = new String(body, afterBoundary, headerEnd - afterBoundary, StandardCharsets.ISO_8859_1);
            Map<String, String> headers = parseHeaders(headersStr);
            int dataStart = headerEnd + 4;
            int nextBoundary = indexOf(body, dashBoundary, dataStart);
            int nextFinal = indexOf(body, dashBoundaryFinal, dataStart);
            int partEnd;
            boolean finalBoundary = false;
            if (nextBoundary < 0 && nextFinal < 0) {
                partEnd = body.length;
            } else if (nextBoundary >= 0 && (nextFinal < 0 || nextBoundary < nextFinal)) {
                partEnd = nextBoundary - 2; // strip CRLF before boundary
            } else {
                partEnd = nextFinal - 2;
                finalBoundary = true;
            }
            if (partEnd < dataStart) partEnd = dataStart;
            byte[] data = Arrays.copyOfRange(body, dataStart, partEnd);
            Part p = new Part();
            p.headers = headers;
            String cd = headers.getOrDefault("content-disposition", "");
            Map<String, String> cdAttrs = parseContentDisposition(cd);
            p.name = cdAttrs.getOrDefault("name", null);
            p.filename = cdAttrs.getOrDefault("filename", null);
            p.data = data;
            parts.add(p);
            if (finalBoundary) break;
            pos = partEnd + 2; // skip \r

        }
        return parts;
    }

    private static boolean matchCrlf(byte[] body, int pos) {
        return pos + 1 < body.length && body[pos] == '\r' && body[pos + 1] == '\n';
    }

    private static int indexOf(byte[] hay, byte[] needle, int from) {
        outer:
        for (int i = Math.max(0, from); i <= hay.length - needle.length; i++) {
            for (int j = 0; j < needle.length; j++) {
                if (hay[i + j] != needle[j]) continue outer;
            }
            return i;
        }
        return -1;
    }

    private static Map<String, String> parseHeaders(String headersStr) {
        Map<String, String> map = new TreeMap<>(String.CASE_INSENSITIVE_ORDER);
        String[] lines = headersStr.split("\r\n");
        for (String line : lines) {
            int c = line.indexOf(':');
            if (c > 0) {
                String k = line.substring(0, c).trim().toLowerCase(Locale.ROOT);
                String v = line.substring(c + 1).trim();
                map.put(k, v);
            }
        }
        return map;
    }

    private static Map<String, String> parseContentDisposition(String cd) {
        Map<String, String> out = new HashMap<>();
        if (cd == null) return out;
        String[] parts = cd.split(";");
        for (String p : parts) {
            String s = p.trim();
            int eq = s.indexOf('=');
            if (eq > 0) {
                String k = s.substring(0, eq).trim().toLowerCase(Locale.ROOT);
                String v = s.substring(eq + 1).trim();
                if (v.startsWith("\"") && v.endsWith("\"") && v.length() >= 2) {
                    v = v.substring(1, v.length() - 1);
                }
                out.put(k, v);
            } else {
                out.putIfAbsent(s.toLowerCase(Locale.ROOT), "");
            }
        }
        return out;
    }

    // Encapsulated upload handling function for testability
    public static String handleUploadRequest(Map<String, List<String>> headers, byte[] body) {
        try {
            String contentType = getFirstHeader(headers, "Content-Type");
            List<Part> parts = parseMultipart(body, contentType);
            if (parts.size() > 20) throw new IOException("Too many parts");
            Part filePart = null;
            for (Part p : parts) {
                if (p.filename != null && "file".equals(p.name)) {
                    filePart = p;
                    break;
                }
            }
            if (filePart == null) {
                return jsonError("No file part 'file' found");
            }
            if (filePart.data.length == 0) return jsonError("Empty file");
            String safeClientName = sanitizeFilename(filePart.filename);
            if (!isAllowedExtension(safeClientName)) return jsonError("Disallowed file type");
            if (!isSafeContent(safeClientName, filePart.data)) return jsonError("File content not allowed");
            if (!Files.exists(UPLOAD_DIR)) {
                Files.createDirectories(UPLOAD_DIR);
            }
            String storedName = randomStoredName(safeClientName);
            Path dest = UPLOAD_DIR.resolve(storedName).normalize();
            if (!dest.startsWith(UPLOAD_DIR)) return jsonError("Path traversal detected");
            // Write file atomically
            Path tmp = Files.createTempFile(UPLOAD_DIR, "up_", ".part");
            Files.write(tmp, filePart.data, StandardOpenOption.TRUNCATE_EXISTING);
            Files.move(tmp, dest, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            return "{\"status\":\"ok\",\"original\":\"" + escapeJson(safeClientName) + "\",\"stored\":\"" + escapeJson(storedName) + "\",\"size\":" + filePart.data.length + "}";
        } catch (IOException e) {
            return jsonError(e.getMessage());
        }
    }

    private static String getFirstHeader(Map<String, List<String>> headers, String name) {
        for (Map.Entry<String, List<String>> e : headers.entrySet()) {
            if (e.getKey() != null && e.getKey().equalsIgnoreCase(name)) {
                if (!e.getValue().isEmpty()) return e.getValue().get(0);
            }
        }
        return null;
    }

    private static String jsonError(String msg) {
        return "{\"status\":\"error\",\"message\":\"" + escapeJson(msg == null ? "error" : msg) + "\"}";
    }

    private static String escapeJson(String s) {
        StringBuilder sb = new StringBuilder();
        for (char c : s.toCharArray()) {
            switch (c) {
                case '"': sb.append("\\\""); break;
                case '\\': sb.append("\\\\"); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 32) sb.append(String.format("\\u%04x", (int)c));
                    else sb.append(c);
            }
        }
        return sb.toString();
    }

    // 4) Code review: Make sure bounds checks and validations are applied consistently (done above).

    // 5) Secure code output: Handler wiring
    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if ("GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    byte[] html = buildFormPage();
                    exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                    exchange.sendResponseHeaders(200, html.length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(html);
                    os.close();
                    return;
                } else if ("POST".equalsIgnoreCase(exchange.getRequestMethod()) && "/upload".equals(exchange.getRequestURI().getPath())) {
                    Headers h = exchange.getRequestHeaders();
                    long contentLength = -1;
                    if (h.containsKey("Content-Length")) {
                        try {
                            contentLength = Long.parseLong(h.getFirst("Content-Length"));
                        } catch (NumberFormatException ignored) {}
                    }
                    InputStream is = exchange.getRequestBody();
                    byte[] body;
                    if (contentLength >= 0 && contentLength <= MAX_UPLOAD_SIZE) {
                        body = readAll(is, MAX_UPLOAD_SIZE);
                    } else {
                        body = readAll(is, MAX_UPLOAD_SIZE);
                    }
                    Map<String, List<String>> map = new HashMap<>();
                    for (Map.Entry<String, List<String>> e : h.entrySet()) map.put(e.getKey(), e.getValue());
                    String json = handleUploadRequest(map, body);
                    byte[] resp = json.getBytes(StandardCharsets.UTF_8);
                    exchange.getResponseHeaders().set("Content-Type", "application/json; charset=UTF-8");
                    exchange.sendResponseHeaders(200, resp.length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(resp);
                    os.close();
                    return;
                } else {
                    byte[] msg = "Not Found".getBytes(StandardCharsets.UTF_8);
                    exchange.sendResponseHeaders(404, msg.length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(msg);
                    os.close();
                }
            } catch (IOException ex) {
                byte[] msg = ("{\"status\":\"error\",\"message\":\"" + escapeJson(ex.getMessage()) + "\"}").getBytes(StandardCharsets.UTF_8);
                exchange.getResponseHeaders().set("Content-Type", "application/json; charset=UTF-8");
                exchange.sendResponseHeaders(500, msg.length);
                OutputStream os = exchange.getResponseBody();
                os.write(msg);
                os.close();
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for helper functions
        // 1) sanitize filename
        assert "evil.txt".equals(sanitizeFilename("../../evil.txt"));
        // 2) allowed extension
        assert isAllowedExtension("image.PNG") == true;
        assert isAllowedExtension("script.sh") == false;
        // 3) content sniff PNG
        byte[] pngSig = new byte[]{(byte)0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0, 1, 2};
        assert isSafeContent("x.png", pngSig);
        // 4) random name preserves extension
        String rn = randomStoredName("a.pdf");
        assert rn.endsWith(".pdf");
        // 5) JSON escape
        String esc = escapeJson("a\"b\\c");
        assert esc.contains("\\\"") && esc.contains("\\\\");

        if (!Files.exists(UPLOAD_DIR)) {
            Files.createDirectories(UPLOAD_DIR);
        }
        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/", new UploadHandler());
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null);
        System.out.println("Server running on http://127.0.0.1:" + PORT + "/");
        server.start();
    }

    static class Part {
        String name;
        String filename;
        Map<String, String> headers;
        byte[] data;
    }
}