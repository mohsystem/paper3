import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Lse71 {

    // --- Security Configuration ---
    // Define a secure, non-web-accessible directory for uploads.
    // For this example, we create a directory in the system's temp location.
    private static final Path UPLOAD_DIR = Paths.get(System.getProperty("java.io.tmpdir"), "webapp_uploads");
    // Whitelist of allowed file extensions.
    private static final List<String> ALLOWED_EXTENSIONS = Arrays.asList(".txt", ".jpg", ".png", ".pdf");
    // Maximum file size in bytes (e.g., 10 MB).
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024;

    public static void main(String[] args) throws IOException {
        // Create the upload directory if it doesn't exist.
        if (!Files.exists(UPLOAD_DIR)) {
            Files.createDirectories(UPLOAD_DIR);
        }

        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null); // creates a default executor
        server.start();

        System.out.println("Server started on port 8000");
        System.out.println("Upload directory: " + UPLOAD_DIR.toAbsolutePath());
        System.out.println("Visit http://localhost:8000 to upload a file.");
        System.out.println("\n--- Test Cases (run in a separate terminal) ---");
        System.out.println("1. Valid TXT file upload:");
        System.out.println("   echo \"This is a test.\" > test.txt");
        System.out.println("   curl -F \"file=@test.txt\" http://localhost:8000/upload");
        System.out.println("\n2. Valid PNG file upload (create a dummy 1x1 png):");
        System.out.println("   curl -o test.png https://via.placeholder.com/1");
        System.out.println("   curl -F \"file=@test.png\" http://localhost:8000/upload");
        System.out.println("\n3. Invalid file type (e.g., .html):");
        System.out.println("   echo \"<html></html>\" > test.html");
        System.out.println("   curl -F \"file=@test.html\" http://localhost:8000/upload");
        System.out.println("\n4. File too large (create an ~11MB file):");
        System.out.println("   dd if=/dev/zero of=largefile.dat bs=1M count=11");
        System.out.println("   curl -F \"file=@largefile.dat\" http://localhost:8000/upload");
        System.out.println("\n5. Malicious filename (directory traversal attempt):");
        System.out.println("   echo \"malicious\" > ../../test.txt");
        System.out.println("   curl -F \"file=@../../test.txt\" http://localhost:8000/upload");
    }
    
    // Handler for the root URL, serves the HTML upload form.
    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "<html>" +
                "<body>" +
                "<h1>Secure File Upload</h1>" +
                "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">" +
                "<input type=\"file\" name=\"file\" />" +
                "<input type=\"submit\" value=\"Upload\" />" +
                "</form>" +
                "</body>" +
                "</html>";
            exchange.sendResponseHeaders(200, response.getBytes().length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }

    // Handler for the /upload URL, processes the file upload.
    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
            if (contentType == null || !contentType.startsWith("multipart/form-data")) {
                sendResponse(exchange, 400, "Bad Request: Content-Type must be multipart/form-data");
                return;
            }

            try {
                // NOTE: A robust multipart parser is complex. For a real application,
                // use a library like Apache Commons FileUpload. This is a simplified parser for demonstration.
                String boundary = "--" + contentType.split("=")[1];
                InputStream requestBody = exchange.getRequestBody();
                
                // Read headers of the first part
                String headers = readLine(requestBody);
                String filename = getOriginalFilename(headers);
                
                // Security Check 1: Sanitize filename
                String sanitizedFilename = sanitizeFilename(filename);
                if (sanitizedFilename.isEmpty()) {
                    sendResponse(exchange, 400, "Invalid filename provided.");
                    return;
                }

                // Security Check 2: Validate file extension
                String extension = getFileExtension(sanitizedFilename);
                if (!ALLOWED_EXTENSIONS.contains(extension)) {
                    sendResponse(exchange, 400, "Invalid file type. Allowed types: " + ALLOWED_EXTENSIONS);
                    return;
                }
                
                // Generate a new, secure filename to prevent path traversal and overwrites
                String newFilename = UUID.randomUUID().toString() + extension;
                Path targetPath = UPLOAD_DIR.resolve(newFilename);

                // Find the start of the file content
                while (true) {
                    String line = readLine(requestBody);
                    if (line == null || line.trim().isEmpty()) {
                        break;
                    }
                }

                // Security Check 3: Save file while checking size
                long size = saveFile(requestBody, targetPath, boundary);

                if (size > MAX_FILE_SIZE) {
                    Files.delete(targetPath); // Clean up partial file
                    sendResponse(exchange, 413, "File is too large. Max size is " + MAX_FILE_SIZE / (1024*1024) + " MB.");
                } else {
                    sendResponse(exchange, 200, "File uploaded successfully as " + newFilename);
                }

            } catch (Exception e) {
                e.printStackTrace();
                sendResponse(exchange, 500, "Internal Server Error: " + e.getMessage());
            }
        }

        private long saveFile(InputStream is, Path targetPath, String boundary) throws IOException {
            ByteArrayOutputStream fileContent = new ByteArrayOutputStream();
            byte[] buffer = new byte[4096];
            int bytesRead;
            long totalBytes = 0;
            
            byte[] boundaryBytes = ("\r\n" + boundary).getBytes();

            // This is a simplified way to find the end boundary.
            // A robust solution needs a more advanced stream searching algorithm.
            while ((bytesRead = is.read(buffer)) != -1) {
                totalBytes += bytesRead;
                if (totalBytes > MAX_FILE_SIZE + boundaryBytes.length) {
                    // Exceeded max size, stop writing but continue reading to drain the request
                    // to avoid client-side errors, then return the oversized value.
                    while (is.read(buffer) != -1) {}
                    return totalBytes;
                }
                fileContent.write(buffer, 0, bytesRead);
            }

            byte[] allBytes = fileContent.toByteArray();
            int endOfFile = findBoundary(allBytes, boundaryBytes);

            try (FileOutputStream fos = new FileOutputStream(targetPath.toFile())) {
                fos.write(allBytes, 0, endOfFile);
            }
            
            return endOfFile;
        }

        private int findBoundary(byte[] haystack, byte[] needle) {
            for (int i = 0; i <= haystack.length - needle.length; i++) {
                boolean found = true;
                for (int j = 0; j < needle.length; j++) {
                    if (haystack[i + j] != needle[j]) {
                        found = false;
                        break;
                    }
                }
                if (found) {
                    return i;
                }
            }
            return haystack.length; // Boundary not found, return full length
        }


        private String sanitizeFilename(String filename) {
            if (filename == null) return "";
            // Basic sanitization: remove path characters.
            return filename.replaceAll("[/\\\\..]", "");
        }

        private String getOriginalFilename(String contentDisposition) {
            Pattern pattern = Pattern.compile("filename=\"([^\"]*)\"");
            Matcher matcher = pattern.matcher(contentDisposition);
            if (matcher.find()) {
                return matcher.group(1);
            }
            return null;
        }

        private String getFileExtension(String filename) {
            int lastDot = filename.lastIndexOf('.');
            if (lastDot > 0) {
                return filename.substring(lastDot).toLowerCase();
            }
            return "";
        }

        private String readLine(InputStream is) throws IOException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            int b;
            while ((b = is.read()) != -1) {
                if (b == '\n') {
                    break;
                }
                baos.write(b);
            }
            if (baos.size() == 0 && b == -1) {
                return null;
            }
            return baos.toString().trim();
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
            exchange.sendResponseHeaders(statusCode, message.getBytes().length);
            OutputStream os = exchange.getResponseBody();
            os.write(message.getBytes());
            os.close();
        }
    }
}