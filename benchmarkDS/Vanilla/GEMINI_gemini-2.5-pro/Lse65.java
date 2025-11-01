import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.Executors;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse65 {

    private static final String UPLOAD_DIR = "uploads";
    private static final int PORT = 8080;
    private static final String SERVER_URL = "http://localhost:" + PORT;

    // --- Server-side Code ---
    static class FileUploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode = 200;

            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                try {
                    // Create uploads directory if it doesn't exist
                    Files.createDirectories(Paths.get(UPLOAD_DIR));

                    String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
                    if (contentType != null && contentType.startsWith("multipart/form-data")) {
                        
                        // Extract boundary
                        String boundary = "--" + contentType.split("boundary=")[1];
                        InputStream requestBody = exchange.getRequestBody();
                        
                        // Simple parser for multipart/form-data
                        byte[] boundaryBytes = (boundary).getBytes();
                        
                        // Read until we find the file content
                        String headers = readUntilBoundary(requestBody, boundaryBytes);
                        
                        // Extract filename
                        String filename = "unknown.pdf";
                        Matcher matcher = Pattern.compile("filename=\"(.*)\"").matcher(headers);
                        if (matcher.find()) {
                            filename = matcher.group(1);
                        }

                        Path savePath = Paths.get(UPLOAD_DIR, filename);
                        
                        // Read file data and save
                        saveFile(requestBody, boundaryBytes, savePath);

                        response = "File '" + filename + "' uploaded successfully.";
                        System.out.println("Server: " + response);

                    } else {
                        statusCode = 400;
                        response = "Bad Request: Content-Type must be multipart/form-data.";
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    statusCode = 500;
                    response = "Internal Server Error: " + e.getMessage();
                }
            } else {
                statusCode = 405;
                response = "Method Not Allowed";
            }
            
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }

        private String readUntilBoundary(InputStream is, byte[] boundary) throws IOException {
            StringBuilder headers = new StringBuilder();
            byte[] b = new byte[1];
            int prev = -1;
            int boundaryIndex = 0;
            
            while(is.read(b) != -1) {
                 // CRLFCRLF indicates end of headers
                if (b[0] == '\n' && prev == '\r') {
                    String line = headers.toString();
                    if (line.endsWith("\r\n\r")) { // end of headers part
                        return line;
                    }
                }
                headers.append((char)b[0]);
                prev = b[0];
            }
            return headers.toString();
        }

        private void saveFile(InputStream is, byte[] boundary, Path path) throws IOException {
            try (OutputStream os = Files.newOutputStream(path)) {
                byte[] buffer = new byte[4096];
                byte[] endBoundary = ("\r\n" + new String(boundary) + "--").getBytes();
                
                int bytesRead;
                byte[] lastBytes = new byte[endBoundary.length];
                int lastBytesIndex = 0;
                
                while ((bytesRead = is.read(buffer)) != -1) {
                    int writeEnd = bytesRead;

                    // Naive check for boundary at the end of buffer
                    if (bytesRead > endBoundary.length) {
                         // Check if the end of the buffer might contain the boundary
                        for (int i = 0; i < endBoundary.length; i++) {
                             if(buffer[bytesRead - endBoundary.length + i] == endBoundary[i]) {
                                 // potential match
                             }
                        }
                    }
                    os.write(buffer, 0, bytesRead);
                }
            }
             // Trim the trailing boundary from the saved file.
             // A robust implementation would not write the boundary in the first place.
             long size = Files.size(path);
             Files.newByteChannel(path, java.nio.file.StandardOpenOption.WRITE).truncate(size - (endBoundary.length)).close();
        }
    }

    public static HttpServer startServer() throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("localhost", PORT), 0);
        server.createContext("/upload", new FileUploadHandler());
        server.setExecutor(Executors.newSingleThreadExecutor());
        server.start();
        System.out.println("Server started on port " + PORT);
        return server;
    }

    // --- Client-side Code ---
    public static String uploadPdfFile(Path filePath) throws IOException, InterruptedException {
        HttpClient client = HttpClient.newHttpClient();
        String boundary = "Boundary-" + UUID.randomUUID().toString();

        HttpRequest request = HttpRequest.newBuilder()
                .uri(URI.create(SERVER_URL + "/upload"))
                .header("Content-Type", "multipart/form-data;boundary=" + boundary)
                .POST(ofMimeMultipartData(filePath, boundary))
                .build();

        HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());
        return "Status: " + response.statusCode() + ", Response: " + response.body();
    }

    private static HttpRequest.BodyPublisher ofMimeMultipartData(Path path, String boundary) throws IOException {
        byte[] separator = ("--" + boundary + "\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" + path.getFileName() + "\"\r\nContent-Type: application/pdf\r\n\r\n").getBytes();
        byte[] fileBytes = Files.readAllBytes(path);
        byte[] trailer = ("\r\n--" + boundary + "--\r\n").getBytes();

        byte[] allBytes = new byte[separator.length + fileBytes.length + trailer.length];
        System.arraycopy(separator, 0, allBytes, 0, separator.length);
        System.arraycopy(fileBytes, 0, allBytes, separator.length, fileBytes.length);
        System.arraycopy(trailer, 0, allBytes, separator.length + fileBytes.length, trailer.length);

        return HttpRequest.BodyPublishers.ofByteArray(allBytes);
    }
    
    // --- Main method with test cases ---
    public static void main(String[] args) {
        HttpServer server = null;
        List<Path> testFiles = new ArrayList<>();
        try {
            // 1. Start Server
            server = startServer();
            
            // 2. Setup test directory and files
            Path uploadDirPath = Paths.get(UPLOAD_DIR);
            Files.createDirectories(uploadDirPath);
            
            System.out.println("\n--- Running 5 Test Cases ---");
            for (int i = 1; i <= 5; i++) {
                Path testFile = Paths.get("test_file_" + i + ".pdf");
                Files.write(testFile, ("This is a dummy PDF file content for test " + i).getBytes());
                testFiles.add(testFile);
                
                System.out.println("\nTest Case " + i + ": Uploading " + testFile.getFileName());
                String result = uploadPdfFile(testFile);
                System.out.println("Client Result: " + result);
                
                // Verify file exists in uploads directory
                if (Files.exists(uploadDirPath.resolve(testFile.getFileName()))) {
                    System.out.println("Verification: SUCCESS - File found in '" + UPLOAD_DIR + "' folder.");
                } else {
                    System.out.println("Verification: FAILED - File not found in '" + UPLOAD_DIR + "' folder.");
                }
                Thread.sleep(100); // Small delay
            }

        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        } finally {
            // 4. Stop Server
            if (server != null) {
                server.stop(0);
                System.out.println("\nServer stopped.");
            }
            // 5. Cleanup
            System.out.println("Cleaning up test files and directory...");
            try {
                for (Path p : testFiles) {
                    Files.deleteIfExists(p);
                }
                // Note: The uploads directory is not deleted to allow inspection of results.
            } catch (IOException e) {
                System.err.println("Cleanup failed: " + e.getMessage());
            }
        }
    }
}