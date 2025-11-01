import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
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
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.Executors;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse65 {

    /**
     * Uploads a PDF file to a given URL using a multipart/form-data POST request.
     *
     * @param serverUrl The URL of the server endpoint (e.g., "http://localhost:8080/upload").
     * @param pdfFilePath The path to the PDF file to upload.
     * @return The server's response body as a String.
     * @throws IOException If an I/O error occurs when reading the file.
     * @throws InterruptedException If the operation is interrupted.
     */
    public static String uploadPdfFile(String serverUrl, Path pdfFilePath) throws IOException, InterruptedException {
        if (pdfFilePath == null || !Files.exists(pdfFilePath) || !Files.isRegularFile(pdfFilePath)) {
            throw new IllegalArgumentException("Invalid file path provided.");
        }

        HttpClient client = HttpClient.newHttpClient();
        String boundary = "----WebKitFormBoundary" + UUID.randomUUID().toString();

        HttpRequest request = HttpRequest.newBuilder()
                .uri(URI.create(serverUrl))
                .header("Content-Type", "multipart/form-data; boundary=" + boundary)
                .POST(ofMimeMultipartData(pdfFilePath, boundary))
                .build();

        HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());
        return response.body();
    }

    private static HttpRequest.BodyPublisher ofMimeMultipartData(Path path, String boundary) throws IOException {
        List<byte[]> byteArrays = new ArrayList<>();
        String filename = path.getFileName().toString();

        String start = "--" + boundary + "\r\n"
                + "Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\n"
                + "Content-Type: application/pdf\r\n\r\n";
        byteArrays.add(start.getBytes(StandardCharsets.UTF_8));
        byteArrays.add(Files.readAllBytes(path));
        String end = "\r\n--" + boundary + "--\r\n";
        byteArrays.add(end.getBytes(StandardCharsets.UTF_8));

        return HttpRequest.BodyPublishers.ofByteArrays(byteArrays);
    }

    // --- Test Server and Main Method for Demonstration ---

    public static void main(String[] args) throws IOException {
        int port = 8080;
        // Start a simple server in the background to receive uploads
        HttpServer server = startTestServer(port);
        System.out.println("Test server started on port " + port);

        // --- Test Cases ---
        runTestCases("http://localhost:" + port + "/upload");

        // Stop the server
        System.out.println("Stopping test server...");
        server.stop(0);
        System.out.println("Server stopped.");
    }

    private static void runTestCases(String url) {
        Path tempDir = null;
        try {
            tempDir = Files.createTempDirectory("lse65_tests");
            System.out.println("Running 5 test cases...");
            for (int i = 1; i <= 5; i++) {
                Path testFile = tempDir.resolve("test" + i + ".pdf");
                try {
                    Files.writeString(testFile, "This is a dummy PDF content for test " + i);
                    System.out.println("\n--- Test Case " + i + " ---");
                    System.out.println("Uploading file: " + testFile.toAbsolutePath());
                    String response = uploadPdfFile(url, testFile);
                    System.out.println("Server response: " + response);
                } catch (Exception e) {
                    System.err.println("Test case " + i + " failed: " + e.getMessage());
                }
            }
        } catch (IOException e) {
            System.err.println("Failed to create temp directory for tests: " + e.getMessage());
        } finally {
            // Cleanup
            if (tempDir != null) {
                try {
                    Files.walk(tempDir)
                         .sorted((a, b) -> b.compareTo(a)) // reverse order
                         .forEach(p -> {
                             try { Files.delete(p); } catch (IOException e) { /* ignored */ }
                         });
                } catch (IOException e) { /* ignored */ }
            }
            Path uploadsDir = Paths.get("uploads");
            if (Files.exists(uploadsDir)) {
                 try {
                    Files.walk(uploadsDir)
                         .sorted((a, b) -> b.compareTo(a))
                         .forEach(p -> {
                             try { Files.delete(p); } catch (IOException e) { /* ignored */ }
                         });
                } catch (IOException e) { /* ignored */ }
            }
        }
    }

    private static HttpServer startTestServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(Executors.newSingleThreadExecutor());
        server.start();
        return server;
    }

    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            Path uploadDir = Paths.get("uploads");
            if (!Files.exists(uploadDir)) {
                Files.createDirectories(uploadDir);
            }
            
            String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
            if (contentType == null || !contentType.startsWith("multipart/form-data")) {
                 sendResponse(exchange, 400, "Bad Request: Content-Type must be multipart/form-data");
                 return;
            }
            
            // Note: This is a simplified parser. A robust server should use a library.
            try (InputStream is = exchange.getRequestBody()) {
                byte[] body = is.readAllBytes();
                String bodyStr = new String(body, StandardCharsets.ISO_8859_1); // Use an encoding that preserves bytes
                String[] parts = bodyStr.split("filename=\"");
                if (parts.length < 2) {
                     sendResponse(exchange, 400, "Bad Request: Invalid multipart format (filename not found)");
                     return;
                }
                String filenamePart = parts[1];
                String filename = filenamePart.substring(0, filenamePart.indexOf("\""));

                // SECURITY: Sanitize filename to prevent path traversal attacks
                Path safeFilename = Paths.get(filename).getFileName();
                if (safeFilename == null) {
                    sendResponse(exchange, 400, "Bad Request: Invalid filename provided.");
                    return;
                }
                
                Path outputFile = uploadDir.resolve(safeFilename).normalize();

                // SECURITY: Ensure the final path is still within the intended directory
                if (!outputFile.startsWith(uploadDir.toAbsolutePath())) {
                    sendResponse(exchange, 400, "Bad Request: Path traversal attempt detected.");
                    return;
                }

                int contentStartIndex = bodyStr.indexOf("\r\n\r\n") + 4;
                String boundary = contentType.split("boundary=")[1];
                int contentEndIndex = bodyStr.lastIndexOf("\r\n--" + boundary + "--");
                
                if (contentStartIndex == 3 || contentEndIndex == -1) {
                     sendResponse(exchange, 400, "Bad Request: Could not parse file content.");
                     return;
                }
                
                try (OutputStream os = Files.newOutputStream(outputFile)) {
                    os.write(body, contentStartIndex, contentEndIndex - contentStartIndex);
                }

                String response = "File '" + safeFilename + "' uploaded successfully.";
                sendResponse(exchange, 200, response);

            } catch (Exception e) {
                sendResponse(exchange, 500, "Server Error: " + e.getMessage());
            }
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }
}