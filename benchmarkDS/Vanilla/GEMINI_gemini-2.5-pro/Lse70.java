import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.UUID;

public class Lse70 {

    private static final String UPLOAD_DIR = "uploads";

    public static void main(String[] args) throws IOException {
        // Create upload directory if it doesn't exist
        Files.createDirectories(Paths.get(UPLOAD_DIR));

        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null); // creates a default executor
        server.start();

        System.out.println("Java server started on port 8000");
        System.out.println("Visit http://localhost:8000 to upload a file.");
        System.out.println("\n--- 5 Test Cases (using cURL) ---");
        System.out.println("NOTE: Ensure you have a file (e.g., test1.txt) in your current directory before running these commands.");
        System.out.println("1. Create a file for testing: echo 'hello java' > test1.txt");
        System.out.println("2. Test Case 1 (text file): curl -X POST -F 'file=@test1.txt' http://localhost:8000/upload");
        System.out.println("3. Create another file: echo 'public class Test {}' > Test.java");
        System.out.println("4. Test Case 2 (java file): curl -X POST -F 'file=@Test.java' http://localhost:8000/upload");
        System.out.println("5. Test Case 3 (attempt upload via GET): curl -v http://localhost:8000/upload");
        System.out.println("6. Test Case 4 (POST with no file): curl -X POST http://localhost:8000/upload");
        System.out.println("7. Test Case 5 (POST with wrong field name): curl -X POST -F 'data=@test1.txt' http://localhost:8000/upload\n");
    }
    
    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "<!DOCTYPE html>" +
                              "<html><head><title>Java File Upload</title></head>" +
                              "<body><h1>Upload a File</h1>" +
                              "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">" +
                              "<input type=\"file\" name=\"file\" />" +
                              "<input type=\"submit\" value=\"Upload\" />" +
                              "</form></body></html>";
            sendResponse(exchange, 200, response);
        }
    }

    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                upload_file(exchange);
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }
    }

    /**
     * The function that handles the file upload.
     * NOTE: This is a simplified multipart/form-data parser for demonstration.
     * It is not robust and has limitations (e.g., loads the entire file into memory).
     * For production code, use a dedicated library like Apache Commons FileUpload.
     * @param exchange The HttpExchange object from the server.
     */
    public static void upload_file(HttpExchange exchange) throws IOException {
        String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
        if (contentType == null || !contentType.startsWith("multipart/form-data")) {
            sendResponse(exchange, 400, "Bad Request: Content-Type must be multipart/form-data");
            return;
        }

        String filename = "unknown_file_" + UUID.randomUUID().toString(); // Default filename
        byte[] body = exchange.getRequestBody().readAllBytes();
        
        // Find filename in headers
        String bodyAsStr = new String(body, 0, Math.min(body.length, 1024));
        if (bodyAsStr.contains("filename=\"")) {
            filename = bodyAsStr.split("filename=\"")[1].split("\"")[0];
        } else {
            sendResponse(exchange, 400, "Bad Request: Filename not found in multipart header.");
            return;
        }

        // Find the start of the actual file content
        byte[] separator = "\r\n\r\n".getBytes();
        int contentStart = indexOf(body, separator, 0);
        if (contentStart == -1) {
            sendResponse(exchange, 400, "Bad Request: Invalid multipart format (no content separator).");
            return;
        }
        contentStart += separator.length;

        // Find the end of the file content (the final boundary)
        String boundary = "--" + contentType.split("boundary=")[1];
        byte[] boundaryBytes = ("\r\n" + boundary).getBytes();
        int contentEnd = indexOf(body, boundaryBytes, contentStart);
        if (contentEnd == -1) {
            sendResponse(exchange, 400, "Bad Request: Invalid multipart format (no closing boundary).");
            return;
        }
        
        File uploadedFile = new File(Paths.get(UPLOAD_DIR, filename).toString());
        try (FileOutputStream fos = new FileOutputStream(uploadedFile)) {
            fos.write(body, contentStart, contentEnd - contentStart);
        }

        sendResponse(exchange, 200, "File '" + filename + "' uploaded successfully.");
    }

    // Helper to find a byte array within another byte array
    private static int indexOf(byte[] source, byte[] target, int fromIndex) {
        for (int i = fromIndex; i <= source.length - target.length; i++) {
            boolean found = true;
            for (int j = 0; j < target.length; j++) {
                if (source[i + j] != target[j]) {
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

    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        exchange.sendResponseHeaders(statusCode, response.getBytes().length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(response.getBytes());
        }
    }
}