import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.List;

public class Lse71 {

    private static final int PORT = 8080;
    private static final String UPLOAD_DIR = "uploads";

    public static void main(String[] args) throws IOException {
        // Create upload directory if it doesn't exist
        File uploadDir = new File(UPLOAD_DIR);
        if (!uploadDir.exists()) {
            uploadDir.mkdir();
        }

        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + PORT);
        System.out.println("Visit http://localhost:" + PORT + " to upload a file.");
        System.out.println("Or use curl for testing from your terminal:");
        System.out.println("-------------------- TEST CASES --------------------");
        System.out.println("1. Create a test file: echo \"Test file 1 content\" > test1.txt");
        System.out.println("2. Upload test1.txt: curl -F \"file=@test1.txt\" http://localhost:8080/upload");
        System.out.println("3. Create another file: echo \"This is the second file.\" > test2.txt");
        System.out.println("4. Upload test2.txt: curl -F \"file=@test2.txt\" http://localhost:8080/upload");
        System.out.println("5. Try a GET request (will show an error message): curl http://localhost:8080/upload");
        System.out.println("--------------------------------------------------");
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "<!DOCTYPE html>" +
                    "<html>" +
                    "<head><title>File Upload</title></head>" +
                    "<body>" +
                    "<h1>Upload a File</h1>" +
                    "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">" +
                    "Select file to upload:" +
                    "<input type=\"file\" name=\"file\" id=\"file\">" +
                    "<input type=\"submit\" value=\"Upload File\" name=\"submit\">" +
                    "</form>" +
                    "</body>" +
                    "</html>";
            exchange.sendResponseHeaders(200, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }

    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode;

            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                try {
                    String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
                    if (contentType != null && contentType.startsWith("multipart/form-data")) {
                        String boundary = contentType.substring(contentType.indexOf("boundary=") + 9);
                        InputStream requestBody = exchange.getRequestBody();
                        
                        // Simple parser for multipart/form-data
                        byte[] boundaryBytes = ("--" + boundary).getBytes();
                        String fileName = null;
                        
                        try (BufferedInputStream in = new BufferedInputStream(requestBody)) {
                             byte[] buffer = new byte[8192];
                             int bytesRead;
                             ByteArrayOutputStream dataStream = new ByteArrayOutputStream();
                             
                             while ((bytesRead = in.read(buffer)) != -1) {
                                 dataStream.write(buffer, 0, bytesRead);
                             }
                             byte[] requestData = dataStream.toByteArray();

                             int start = indexOf(requestData, "filename=\"".getBytes(), 0) + 10;
                             if(start > 10) {
                                int end = indexOf(requestData, "\"".getBytes(), start);
                                fileName = new String(requestData, start, end - start);
                                
                                int fileContentStart = indexOf(requestData, "\r\n\r\n".getBytes(), end) + 4;
                                int fileContentEnd = lastIndexOf(requestData, boundaryBytes, requestData.length - 1) - 2; // -2 for \r

                                
                                if (fileName != null && !fileName.isEmpty() && fileContentStart < fileContentEnd) {
                                    File uploadedFile = new File(UPLOAD_DIR, fileName);
                                    try (FileOutputStream fos = new FileOutputStream(uploadedFile)) {
                                        fos.write(requestData, fileContentStart, fileContentEnd - fileContentStart);
                                    }
                                    response = "File " + fileName + " uploaded successfully to " + UPLOAD_DIR + " directory.";
                                    statusCode = 200;
                                } else {
                                    response = "File upload failed: No file content found.";
                                    statusCode = 400;
                                }
                             } else {
                                response = "File upload failed: Malformed request.";
                                statusCode = 400;
                             }
                        }
                    } else {
                        response = "Invalid content type. Must be multipart/form-data.";
                        statusCode = 400;
                    }
                } catch (Exception e) {
                    response = "Error during file upload: " + e.getMessage();
                    statusCode = 500;
                    e.printStackTrace();
                }
            } else {
                response = "This endpoint only supports POST requests.";
                statusCode = 405; // Method Not Allowed
            }

            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
        
        // Helper to find byte sequence
        private int indexOf(byte[] data, byte[] pattern, int start) {
            for (int i = start; i < data.length - pattern.length + 1; i++) {
                boolean found = true;
                for (int j = 0; j < pattern.length; j++) {
                    if (data[i + j] != pattern[j]) {
                        found = false;
                        break;
                    }
                }
                if (found) return i;
            }
            return -1;
        }

        private int lastIndexOf(byte[] data, byte[] pattern, int start) {
            for (int i = start; i >= 0; i--) {
                boolean found = true;
                for (int j = 0; j < pattern.length; j++) {
                    if (i + j >= data.length || data[i + j] != pattern[j]) {
                        found = false;
                        break;
                    }
                }
                if (found) return i;
            }
            return -1;
        }
    }
}