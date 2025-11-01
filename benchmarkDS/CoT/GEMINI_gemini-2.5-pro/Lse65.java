import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.UUID;
import java.util.stream.Collectors;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse65 {

    private static final String UPLOAD_DIR = "uploads";
    private static final int PORT = 8000;

    public static void main(String[] args) throws IOException {
        // Create upload directory if it doesn't exist
        Files.createDirectories(Paths.get(UPLOAD_DIR));

        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/", new FormHandler());
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + PORT);
        System.out.println("Access http://localhost:8000/ to upload a file.");
        
        System.out.println("\n--- How to Test ---");
        System.out.println("1. Open http://localhost:8000 in your web browser.");
        System.out.println("Test Case 1: Upload a valid PDF file smaller than 10MB. It should succeed.");
        System.out.println("Test Case 2: Upload a non-PDF file (e.g., .txt, .jpg). It should be rejected.");
        System.out.println("Test Case 3: Create a dummy file larger than 10MB and try to upload it. It should be rejected.");
        System.out.println("Test Case 4: Try to upload a file with a tricky name like '../test.pdf'. The server should save it as 'test.pdf'.");
        System.out.println("Test Case 5: Refresh the root page to ensure the form is still being served correctly.");
    }

    static class FormHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equals(exchange.getRequestMethod())) {
                String response = "<html>" +
                    "<body>" +
                    "<h2>Upload a PDF File</h2>" +
                    "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">" +
                    "  Select a PDF to upload:" +
                    "  <input type=\"file\" name=\"pdfFile\" id=\"pdfFile\" accept=\".pdf\">" +
                    "  <input type=\"submit\" value=\"Upload PDF\" name=\"submit\">" +
                    "</form>" +
                    "</body>" +
                    "</html>";
                sendResponse(exchange, 200, response);
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }
    }

    static class UploadHandler implements HttpHandler {
        private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
            if (contentType == null || !contentType.startsWith("multipart/form-data")) {
                sendResponse(exchange, 400, "Bad Request: Content-Type must be multipart/form-data");
                return;
            }

            try {
                // NOTE: Manually parsing multipart/form-data is complex and error-prone.
                // In a real-world application, use a robust library like Apache Commons FileUpload.
                // This is a simplified parser for demonstration purposes.
                String boundary = "--" + contentType.split("boundary=")[1];
                InputStream requestBody = exchange.getRequestBody();
                
                // Using a temporary file to handle the upload stream
                Path tempFile = Files.createTempFile("upload", ".tmp");
                Files.copy(requestBody, tempFile, StandardCopyOption.REPLACE_EXISTING);

                if (Files.size(tempFile) > MAX_FILE_SIZE) {
                    sendResponse(exchange, 413, "File too large. Max size is " + MAX_FILE_SIZE / (1024*1024) + "MB");
                    Files.delete(tempFile);
                    return;
                }

                String content = new String(Files.readAllBytes(tempFile), StandardCharsets.ISO_8859_1);
                String[] parts = content.split(boundary);
                
                boolean fileFound = false;
                for (String part : parts) {
                    if (part.contains("filename=\"")) {
                        // Extract filename
                        String disposition = part.substring(part.indexOf("Content-Disposition:"), part.indexOf("\r\n"));
                        String filename = disposition.replaceAll("(?i).*filename=\"([^\"]+)\".*", "$1");

                        // --- SECURITY: Path Traversal ---
                        // Sanitize filename to prevent directory traversal attacks.
                        String sanitizedFilename = Paths.get(filename).getFileName().toString();
                        if (sanitizedFilename.isEmpty()) {
                            sendResponse(exchange, 400, "Invalid filename.");
                            return;
                        }

                        // --- SECURITY: File Type Check ---
                        if (!sanitizedFilename.toLowerCase().endsWith(".pdf")) {
                            sendResponse(exchange, 400, "Invalid file type. Only PDF files are allowed.");
                            return;
                        }

                        // Extract file content
                        int headerEnd = part.indexOf("\r\n\r\n") + 4;
                        String fileContentStr = part.substring(headerEnd);
                        // The last part will have a trailing "--\r\n", remove it
                        String fileData = fileContentStr.trim(); 
                        if (fileData.endsWith("--")) {
                            fileData = fileData.substring(0, fileData.length() - 2).trim();
                        }


                        Path filePath = Paths.get(UPLOAD_DIR, sanitizedFilename);
                        Files.write(filePath, fileData.getBytes(StandardCharsets.ISO_8859_1));

                        sendResponse(exchange, 200, "File '" + sanitizedFilename + "' uploaded successfully.");
                        fileFound = true;
                        break;
                    }
                }
                
                Files.delete(tempFile);
                if (!fileFound) {
                    sendResponse(exchange, 400, "Bad Request: No file part found in the request.");
                }

            } catch (Exception e) {
                e.printStackTrace();
                sendResponse(exchange, 500, "Internal Server Error: " + e.getMessage());
            }
        }
    }
    
    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        exchange.sendResponseHeaders(statusCode, response.getBytes().length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(response.getBytes());
        }
    }
}