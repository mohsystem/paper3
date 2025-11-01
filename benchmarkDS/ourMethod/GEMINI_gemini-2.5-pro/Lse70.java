import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ByteArrayOutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse70 {

    private static final String UPLOADS_DIR = "uploads";
    private static final int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
    private static final int PORT = 8080;

    public static void main(String[] args) throws IOException {
        // Create the uploads directory if it doesn't exist
        Path uploadPath = Paths.get(UPLOADS_DIR);
        if (!Files.exists(uploadPath)) {
            Files.createDirectories(uploadPath);
        }

        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null); // creates a default executor
        System.out.println("Server started on port " + PORT);
        System.out.println("Use 'curl -F \"file=@<your_file_path>\" http://localhost:" + PORT + "/upload' to test.");
        server.start();

        // The main method will exit, but the server thread will keep running.
        // For a real application, you'd want to manage the server lifecycle.
        // For test cases, manual testing is required for a server.
        System.out.println("\n--- Test Cases ---");
        System.out.println("1. Valid file upload: curl -F \"file=@test.txt\" http://localhost:8080/upload");
        System.out.println("2. File too large (>10MB): Prepare a large file and upload it.");
        System.out.println("3. No file part: curl -X POST http://localhost:8080/upload");
        System.out.println("4. Path traversal attempt: curl -F \"file=@test.txt;filename=../../test.txt\" http://localhost:8080/upload");
        System.out.println("5. Overwrite attempt: Upload a file, then try to upload another with the same name.");
    }

    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode;

            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                try {
                    uploadFile(exchange);
                    response = "File uploaded successfully.";
                    statusCode = 200;
                } catch (IllegalArgumentException | IOException e) {
                    response = "Error: " + e.getMessage();
                    if (e.getMessage().contains("File too large")) {
                        statusCode = 413; // Payload Too Large
                    } else if (e.getMessage().contains("File already exists")) {
                        statusCode = 409; // Conflict
                    }
                    else {
                        statusCode = 400; // Bad Request
                    }
                }
            } else {
                response = "Method not allowed. Use POST.";
                statusCode = 405;
            }

            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    public static void uploadFile(HttpExchange exchange) throws IOException {
        String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
        if (contentType == null || !contentType.startsWith("multipart/form-data")) {
            throw new IllegalArgumentException("Invalid content type. Expected multipart/form-data.");
        }

        String boundary = "--" + contentType.substring(contentType.indexOf("boundary=") + 9);
        byte[] boundaryBytes = boundary.getBytes(StandardCharsets.UTF_8);

        try (InputStream in = exchange.getRequestBody()) {
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] data = new byte[8192];
            int bytesRead;
            long totalBytesRead = 0;
            while ((bytesRead = in.read(data)) != -1) {
                totalBytesRead += bytesRead;
                if (totalBytesRead > MAX_FILE_SIZE) {
                    throw new IOException("File too large. Limit is " + MAX_FILE_SIZE + " bytes.");
                }
                buffer.write(data, 0, bytesRead);
            }

            byte[] requestBody = buffer.toByteArray();
            
            // Very basic multipart parsing
            // Find the first boundary
            int start = indexOf(requestBody, boundaryBytes, 0);
            if (start == -1) {
                throw new IllegalArgumentException("Malformed request: boundary not found.");
            }
            start += boundaryBytes.length;

            // Find Content-Disposition header to get filename
            String headersPart = new String(requestBody, start, Math.min(1024, requestBody.length - start), StandardCharsets.ISO_8859_1);
            
            Pattern pattern = Pattern.compile("filename=\"([^\"]*)\"");
            Matcher matcher = pattern.matcher(headersPart);
            if (!matcher.find()) {
                 throw new IllegalArgumentException("Malformed request: filename not found in Content-Disposition.");
            }
            String clientFilename = matcher.group(1);
            if (clientFilename.isEmpty()) {
                throw new IllegalArgumentException("No file selected or filename is empty.");
            }

            // Sanitize filename to prevent path traversal
            String sanitizedFilename = Paths.get(clientFilename).getFileName().toString();
            if(sanitizedFilename.isEmpty() || sanitizedFilename.equals(".") || sanitizedFilename.equals("..")){
                throw new IllegalArgumentException("Invalid filename provided.");
            }
            Path destinationPath = Paths.get(UPLOADS_DIR, sanitizedFilename);

            // Find start of file content (after CRLF CRLF)
            byte[] separator = "\r\n\r\n".getBytes(StandardCharsets.UTF_8);
            int contentStart = indexOf(requestBody, separator, start);
            if (contentStart == -1) {
                 throw new IllegalArgumentException("Malformed request: header/body separator not found.");
            }
            contentStart += separator.length;
            
            // Find end of file content (start of the next boundary)
            int contentEnd = indexOf(requestBody, boundaryBytes, contentStart);
            if (contentEnd == -1) {
                contentEnd = requestBody.length; // Assume it ends at the end of the body
            }
            // The content actually ends just before the CRLF preceding the boundary
            if (contentEnd > 2 && requestBody[contentEnd - 2] == '\r' && requestBody[contentEnd - 1] == '\n') {
                contentEnd -= 2;
            }

            if (contentStart >= contentEnd) {
                 throw new IllegalArgumentException("No file content found.");
            }
            
            try (OutputStream out = Files.newOutputStream(destinationPath, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
                out.write(requestBody, contentStart, contentEnd - contentStart);
            } catch (java.nio.file.FileAlreadyExistsException e) {
                throw new IOException("File already exists: " + sanitizedFilename, e);
            }
        }
    }

    // Helper to find byte array pattern
    private static int indexOf(byte[] source, byte[] pattern, int fromIndex) {
        for (int i = fromIndex; i <= source.length - pattern.length; i++) {
            boolean found = true;
            for (int j = 0; j < pattern.length; j++) {
                if (source[i + j] != pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return i;
            }
        }
        return -1;
    }
}