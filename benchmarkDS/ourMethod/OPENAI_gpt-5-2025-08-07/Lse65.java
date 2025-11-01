import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.security.SecureRandom;
import java.util.Locale;

public class Lse65 {
    public static final class UploadResult {
        public final boolean ok;
        public final int httpStatus;
        public final String responseBody;
        public final String error;

        public UploadResult(boolean ok, int httpStatus, String responseBody, String error) {
            this.ok = ok;
            this.httpStatus = httpStatus;
            this.responseBody = responseBody;
            this.error = error;
        }

        @Override
        public String toString() {
            return "ok=" + ok + ", status=" + httpStatus + ", error=" + (error == null ? "" : error) +
                    ", body=" + (responseBody == null ? "" : responseBody);
        }
    }

    public static UploadResult uploadPdfToLocalhost(Path file, String endpoint, long maxSizeBytes, int port) {
        // Enforce loopback only
        String host = "127.0.0.1";
        if (endpoint == null || endpoint.isEmpty() || endpoint.charAt(0) != '/') {
            endpoint = "/upload";
        }
        // Validate filename
        String name = file.getFileName().toString();
        if (!name.toLowerCase(Locale.ROOT).endsWith(".pdf")) {
            return new UploadResult(false, -1, null, "Invalid extension");
        }
        try {
            // Reject symlinks and validate it's a regular file
            if (Files.isSymbolicLink(file)) {
                return new UploadResult(false, -1, null, "Symbolic links not allowed");
            }
            BasicFileAttributes attrs = Files.readAttributes(file, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                return new UploadResult(false, -1, null, "Not a regular file");
            }
            long size = attrs.size();
            if (size <= 0 || size > maxSizeBytes) {
                return new UploadResult(false, -1, null, "Invalid size");
            }

            byte[] content;
            try (InputStream is = new BufferedInputStream(Files.newInputStream(file, StandardOpenOption.READ))) {
                content = is.readNBytes((int)Math.min(size, Integer.MAX_VALUE));
                if (content.length != (int)size) {
                    return new UploadResult(false, -1, null, "Short read");
                }
            }
            // Validate PDF signature
            if (content.length < 5 ||
                content[0] != '%' || content[1] != 'P' || content[2] != 'D' || content[3] != 'F' || content[4] != '-') {
                return new UploadResult(false, -1, null, "Invalid PDF signature");
            }

            String boundary = generateBoundary();
            byte[] header = buildMultipartHeader(boundary, name);
            byte[] footer = buildMultipartFooter(boundary);
            long totalLen = (long)header.length + content.length + footer.length;
            if (totalLen > Integer.MAX_VALUE) {
                return new UploadResult(false, -1, null, "Body too large");
            }

            URL url = new URL("http", host, port, endpoint);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setConnectTimeout(4000);
            conn.setReadTimeout(8000);
            conn.setDoOutput(true);
            conn.setRequestMethod("POST");
            conn.setUseCaches(false);
            conn.setRequestProperty("Content-Type", "multipart/form-data; boundary=" + boundary);
            conn.setRequestProperty("Content-Length", String.valueOf((int)totalLen));
            conn.setRequestProperty("Connection", "close");

            try (OutputStream os = new BufferedOutputStream(conn.getOutputStream())) {
                os.write(header);
                os.write(content);
                os.write(footer);
                os.flush();
            }

            int status = conn.getResponseCode();
            String resp;
            try (InputStream in = (status >= 200 && status < 400) ? conn.getInputStream() : conn.getErrorStream()) {
                if (in != null) {
                    ByteArrayOutputStream baos = new ByteArrayOutputStream();
                    byte[] buf = new byte[4096];
                    int r;
                    while ((r = in.read(buf)) != -1) {
                        baos.write(buf, 0, r);
                    }
                    resp = baos.toString(StandardCharsets.UTF_8);
                } else {
                    resp = "";
                }
            } finally {
                conn.disconnect();
            }
            return new UploadResult(status >= 200 && status < 300, status, resp, null);
        } catch (Exception ex) {
            return new UploadResult(false, -1, null, "Network or IO error");
        }
    }

    private static String generateBoundary() {
        SecureRandom sr = new SecureRandom();
        byte[] b = new byte[16];
        sr.nextBytes(b);
        StringBuilder sb = new StringBuilder("----Boundary");
        for (byte x : b) {
            sb.append(Integer.toHexString((x & 0xFF) >>> 4));
            sb.append(Integer.toHexString(x & 0x0F));
        }
        return sb.toString();
    }

    private static byte[] buildMultipartHeader(String boundary, String filename) {
        StringBuilder sb = new StringBuilder();
        sb.append("--").append(boundary).append("\r\n");
        sb.append("Content-Disposition: form-data; name=\"file\"; filename=\"")
          .append(safeFilename(filename)).append("\"\r\n");
        sb.append("Content-Type: application/pdf\r\n\r\n");
        return sb.toString().getBytes(StandardCharsets.UTF_8);
    }

    private static byte[] buildMultipartFooter(String boundary) {
        String s = "\r\n--" + boundary + "--\r\n";
        return s.getBytes(StandardCharsets.UTF_8);
    }

    private static String safeFilename(String name) {
        // Minimal sanitization for header context
        return name.replace("\"", "_").replace("\r", "_").replace("\n", "_");
    }

    private static Path createPdf(Path dir, String name, byte[] payload) throws IOException {
        Path p = dir.resolve(name);
        try (OutputStream os = Files.newOutputStream(p, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
            os.write(payload);
            os.flush();
        }
        return p;
    }

    public static void main(String[] args) throws Exception {
        long maxSize = 512 * 1024; // 512 KiB for tests
        int port = 8000;
        Path tmp = Files.createTempDirectory("java_pdfup_");
        try {
            byte[] minimalPdf = ("%PDF-1.4\n1 0 obj\n<<>>\nendobj\ntrailer\n<<>>\n%%EOF\n").getBytes(StandardCharsets.UTF_8);
            // 1) Valid small PDF
            Path f1 = createPdf(tmp, "t1.pdf", minimalPdf);
            // 2) Another valid small PDF
            Path f2 = createPdf(tmp, "t2.pdf", minimalPdf);
            // 3) Invalid extension
            Path f3 = createPdf(tmp, "notes.txt", minimalPdf);
            // 4) Invalid header but .pdf extension
            Path f4 = createPdf(tmp, "bad.pdf", "HELLO".getBytes(StandardCharsets.UTF_8));
            // 5) Oversized PDF
            ByteArrayOutputStream big = new ByteArrayOutputStream();
            big.write("%PDF-1.4\n".getBytes(StandardCharsets.UTF_8));
            byte[] chunk = new byte[64 * 1024];
            for (int i = 0; i < chunk.length; i++) chunk[i] = 'A';
            int repeats = (int)((maxSize / chunk.length) + 2);
            for (int i = 0; i < repeats; i++) big.write(chunk);
            big.write("%%EOF\n".getBytes(StandardCharsets.UTF_8));
            Path f5 = createPdf(tmp, "big.pdf", big.toByteArray());

            UploadResult r1 = uploadPdfToLocalhost(f1, "/upload", maxSize, port);
            UploadResult r2 = uploadPdfToLocalhost(f2, "/upload", maxSize, port);
            UploadResult r3 = uploadPdfToLocalhost(f3, "/upload", maxSize, port);
            UploadResult r4 = uploadPdfToLocalhost(f4, "/upload", maxSize, port);
            UploadResult r5 = uploadPdfToLocalhost(f5, "/upload", maxSize, port);

            System.out.println("Test1: " + r1);
            System.out.println("Test2: " + r2);
            System.out.println("Test3: " + r3);
            System.out.println("Test4: " + r4);
            System.out.println("Test5: " + r5);
        } finally {
            // Cleanup temp files
            try (DirectoryStream<Path> ds = Files.newDirectoryStream(tmp)) {
                for (Path p : ds) {
                    try { Files.deleteIfExists(p); } catch (Exception ignored) {}
                }
            } catch (Exception ignored) {}
            try { Files.deleteIfExists(tmp); } catch (Exception ignored) {}
        }
    }
}