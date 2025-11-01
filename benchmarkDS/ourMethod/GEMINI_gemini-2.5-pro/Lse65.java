import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.File;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.time.Duration;
import java.util.UUID;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse65 {

    private static final String UPLOAD_DIR = "uploads";
    private static final int PORT = 8080;
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

    public static void main(String[] args) throws IOException {
        // Create upload directory if it doesn't exist
        Path uploadPath = Paths.get(UPLOAD_DIR);
        if (!Files.exists(uploadPath)) {
            Files.createDirectories(uploadPath);
            System.out.println("Created directory: " + uploadPath.toAbsolutePath());
        }

        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/", new FormHandler());
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + PORT);
        System.out.println("Visit http://localhost:8080 to upload a file.");
        
        // Running test cases
        runTestCases();
        
        System.out.println("Press Enter to stop the server...");
        System.in.read();
        server.stop(0);
        System.out.println("Server stopped.");
    }
    
    static class FormHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                String response = "<!DOCTYPE html><html><head><title>File Upload</title></head><body>"
                                + "<h2>Upload PDF File</h2>"
                                + "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">"
                                + "Select PDF file to upload:"
                                + "<input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\" accept=\".pdf\" required>"
                                + "<input type=\"submit\" value=\"Upload PDF\" name=\"submit\">"
                                + "</form></body></html>";
                exchange.getResponseHeaders().set("Content-Type", "text/html");
                exchange.sendResponseHeaders(200, response.length());
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            } else {
                exchange.sendResponseHeaders(405, -1); // Method Not Allowed
            }
        }
    }

    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed"); // Method Not Allowed
                return;
            }

            String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
            if (contentType == null || !contentType.toLowerCase().startsWith("multipart/form-data")) {
                sendResponse(exchange, 400, "Bad Request: Content-Type must be multipart/form-data");
                return;
            }

            try (InputStream requestBody = exchange.getRequestBody()) {
                String boundary = "--" + contentType.split("boundary=")[1];
                byte[] boundaryBytes = boundary.getBytes(StandardCharsets.UTF_8);
                
                // Read the stream and find the file part
                // This is a simplified parser for demonstration.
                // A robust implementation should use a proper multipart parsing library.
                byte[] body = requestBody.readAllBytes();
                
                // Crude split
                String[] parts = new String(body, StandardCharsets.ISO_8859_1).split(boundary);
                
                boolean fileProcessed = false;
                for (String part : parts) {
                    if (part.contains("Content-Disposition: form-data;")) {
                        Pattern fileNamePattern = Pattern.compile("filename=\"([^\"]+)\"");
                        Matcher matcher = fileNamePattern.matcher(part);
                        if (matcher.find()) {
                            String originalFilename = matcher.group(1);

                            // --- Security: Sanitize filename ---
                            String sanitizedFilename = Paths.get(originalFilename).getFileName().toString();
                            if (sanitizedFilename.isEmpty() || !sanitizedFilename.toLowerCase().endsWith(".pdf")) {
                                sendResponse(exchange, 400, "Bad Request: Invalid file. Only PDF files are allowed.");
                                return;
                            }
                            
                            // Generate a unique filename to prevent overwrites and other attacks
                            String uniqueFilename = UUID.randomUUID().toString() + "_" + sanitizedFilename;

                            // --- Security: Path Traversal Check ---
                            Path uploadDirPath = Paths.get(UPLOAD_DIR).toAbsolutePath();
                            Path destinationPath = uploadDirPath.resolve(uniqueFilename).normalize();
                            
                            if (!destinationPath.startsWith(uploadDirPath)) {
                                sendResponse(exchange, 400, "Bad Request: Invalid file path (Path Traversal attempt).");
                                return;
                            }

                            int headerEnd = part.indexOf("\r\n\r\n") + 4;
                            // The part string was decoded as ISO_8859_1, we need original bytes
                            int partStartIndex = new String(body, StandardCharsets.ISO_8859_1).indexOf(part);
                            int contentStartIndex = partStartIndex + headerEnd;
                            // The content ends before the next boundary or the final "--"
                            int contentLength = part.trim().getBytes(StandardCharsets.ISO_8859_1).length - headerEnd;
                            
                             // --- Security: Check file size ---
                            if (contentLength > MAX_FILE_SIZE) {
                                sendResponse(exchange, 413, "Payload Too Large: File size exceeds " + MAX_FILE_SIZE + " bytes.");
                                return;
                            }
                            if (contentLength <= 0) {
                                sendResponse(exchange, 400, "Bad Request: Uploaded file is empty.");
                                return;
                            }

                            try (OutputStream fos = new FileOutputStream(destinationPath.toFile())) {
                                fos.write(body, contentStartIndex, contentLength);
                            }
                            
                            System.out.println("File saved: " + destinationPath);
                            fileProcessed = true;
                            break; 
                        }
                    }
                }

                if (fileProcessed) {
                    sendResponse(exchange, 200, "File uploaded successfully!");
                } else {
                    sendResponse(exchange, 400, "Bad Request: No file part found in the request.");
                }

            } catch (Exception e) {
                e.printStackTrace();
                sendResponse(exchange, 500, "Internal Server Error: " + e.getMessage());
            }
        }
        
        private void sendResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
            byte[] response = message.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            exchange.sendResponseHeaders(statusCode, response.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response);
            }
        }
    }
    
    private static void runTestCases() {
        System.out.println("\n--- Running 5 Test Cases ---");
        // Test cases will run against the server started in main.
        // This requires the server to be running in a separate thread, which it is.
        HttpClient client = HttpClient.newBuilder()
            .version(HttpClient.Version.HTTP_1_1)
            .connectTimeout(Duration.ofSeconds(10))
            .build();

        // Test Case 1: Valid PDF upload
        testUpload(client, "test1.pdf", "This is a fake PDF.", "application/pdf", true);
        // Test Case 2: Another valid PDF upload
        testUpload(client, "document.pdf", "%PDF-1.4...", "application/pdf", true);
        // Test Case 3: Invalid file type
        testUpload(client, "test3.txt", "This is a text file.", "text/plain", false);
        // Test Case 4: Path traversal attempt
        testUpload(client, "../../test4.pdf", "Attempting traversal.", "application/pdf", false);
        // Test Case 5: File too large
        testUpload(client, "large_file.pdf", "a".repeat(11 * 1024 * 1024), "application/pdf", false);
        System.out.println("--- Test Cases Finished ---\n");
    }

    private static void testUpload(HttpClient client, String filename, String content, String contentType, boolean expectSuccess) {
        try {
            Path tempFile = Files.createTempFile("test_", filename);
            Files.writeString(tempFile, content);

            String boundary = "Boundary-" + System.currentTimeMillis();
            String requestBody = "--" + boundary + "\r\n"
                    + "Content-Disposition: form-data; name=\"fileToUpload\"; filename=\"" + filename + "\"\r\n"
                    + "Content-Type: " + contentType + "\r\n\r\n"
                    + content + "\r\n"
                    + "--" + boundary + "--\r\n";

            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create("http://localhost:" + PORT + "/upload"))
                    .header("Content-Type", "multipart/form-data; boundary=" + boundary)
                    .POST(HttpRequest.BodyPublishers.ofString(requestBody))
                    .build();

            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());

            System.out.print("Test Case [" + filename + "]: ");
            if (expectSuccess && response.statusCode() == 200) {
                System.out.println("PASSED (Status " + response.statusCode() + ")");
            } else if (!expectSuccess && response.statusCode() != 200) {
                System.out.println("PASSED (Status " + response.statusCode() + " as expected)");
            } else {
                System.out.println("FAILED (Expected success=" + expectSuccess + ", got status " + response.statusCode() + ")");
                System.out.println("   Response: " + response.body());
            }

            Files.delete(tempFile);
        } catch (IOException | InterruptedException e) {
            System.out.println("Test Case [" + filename + "]: FAILED with exception: " + e.getMessage());
        }
    }
}