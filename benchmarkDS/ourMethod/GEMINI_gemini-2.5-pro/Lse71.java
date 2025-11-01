import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URL;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.Executors;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

// Note: This example uses the built-in com.sun.net.httpserver.HttpServer.
// For a production environment, using a dedicated servlet container like Jetty or Tomcat
// with the Servlet API (@MultipartConfig) is strongly recommended for robust and secure file upload handling.
// Parsing multipart/form-data manually is complex and error-prone. This is a simplified educational example.

public class Lse71 {

    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(Arrays.asList(".txt", ".jpg", ".png", ".pdf"));

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        // Create upload directory if it doesn't exist
        Files.createDirectories(Paths.get(UPLOAD_DIR));
        
        server.createContext("/upload", new FileUploadHandler());
        server.setExecutor(Executors.newFixedThreadPool(10)); 
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Upload endpoint is at http://localhost:" + port + "/upload");

        // Running test cases
        runTestCases();

        // To stop the server manually, you might need to add a shutdown hook or another mechanism.
        // For this example, it runs until manually stopped.
    }

    static class FileUploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode = 200;

            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                statusCode = 405; // Method Not Allowed
                response = "Only POST method is allowed.";
            } else {
                String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
                if (contentType == null || !contentType.startsWith("multipart/form-data")) {
                    statusCode = 400; // Bad Request
                    response = "Content-Type must be multipart/form-data.";
                } else {
                    try {
                        String boundary = contentType.substring(contentType.indexOf("boundary=") + 9);
                        saveFile(exchange.getRequestBody(), boundary);
                        response = "File uploaded successfully.";
                    } catch (IOException | SecurityException | IllegalArgumentException e) {
                        statusCode = 400;
                        response = "File upload failed: " + e.getMessage();
                        e.printStackTrace();
                    }
                }
            }

            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    private static void saveFile(InputStream requestBody, String boundary) throws IOException {
        // Simplified parser. Not robust for all cases.
        // A proper library should be used in production.
        byte[] boundaryBytes = ("--" + boundary).getBytes(StandardCharsets.UTF_8);
        byte[] buffer = new byte[8192];
        int bytesRead;

        try (InputStream is = requestBody) {
            // Find the first boundary
            findNextBoundary(is, boundaryBytes);

            // Read headers for the part
            String headers = readHeaders(is);
            String filename = getFilename(headers);
            if (filename == null || filename.isEmpty()) {
                throw new IllegalArgumentException("Filename not found in multipart header.");
            }

            // Sanitize and validate filename
            Path originalPath = Paths.get(filename);
            String sanitizedFilename = originalPath.getFileName().toString(); // Removes directory info
            String extension = getFileExtension(sanitizedFilename);

            if (!ALLOWED_EXTENSIONS.contains(extension.toLowerCase())) {
                throw new SecurityException("File type not allowed.");
            }

            // Generate a unique filename to prevent overwrites and other attacks
            String uniqueFilename = UUID.randomUUID().toString() + extension;
            Path uploadDirPath = Paths.get(UPLOAD_DIR);
            
            if (!uploadDirPath.toFile().exists()) {
                Files.createDirectories(uploadDirPath);
            }

            Path tempFile = null;
            try {
                // Write to a temporary file first
                tempFile = Files.createTempFile(uploadDirPath, "upload-", ".tmp");
                long totalBytes = 0;
                
                try (OutputStream fos = Files.newOutputStream(tempFile)) {
                     while ((bytesRead = is.read(buffer)) != -1) {
                        int boundaryPos = KMPMatch.indexOf(buffer, 0, bytesRead, boundaryBytes);
                        if (boundaryPos != -1) {
                            fos.write(buffer, 0, boundaryPos - 2); // -2 to remove CRLF
                            totalBytes += boundaryPos - 2;
                            break; 
                        } else {
                            fos.write(buffer, 0, bytesRead);
                            totalBytes += bytesRead;
                        }

                        if (totalBytes > MAX_FILE_SIZE) {
                            throw new IOException("File size exceeds the " + MAX_FILE_SIZE / (1024*1024) + " MB limit.");
                        }
                    }
                }

                // Atomically move the temporary file to its final destination
                Path finalPath = uploadDirPath.resolve(uniqueFilename);
                Files.move(tempFile, finalPath, StandardCopyOption.ATOMIC_MOVE);
                
                // Set secure file permissions (owner read/write only)
                try {
                    Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                    Files.setPosixFilePermissions(finalPath, perms);
                } catch (UnsupportedOperationException e) {
                    // Not a POSIX system, like Windows.
                    System.err.println("Warning: POSIX file permissions not supported on this system.");
                }

            } finally {
                if (tempFile != null && Files.exists(tempFile)) {
                    Files.delete(tempFile);
                }
            }
        }
    }

    private static String readHeaders(InputStream is) throws IOException {
        StringBuilder headers = new StringBuilder();
        byte[] crlf = new byte[]{'\r', '\n', '\r', '\n'};
        int matchIndex = 0;
        int b;
        while((b = is.read()) != -1){
            headers.append((char)b);
            if(b == crlf[matchIndex]) {
                matchIndex++;
                if(matchIndex == crlf.length) return headers.toString();
            } else {
                matchIndex = 0;
            }
        }
        return headers.toString();
    }
    
    private static void findNextBoundary(InputStream is, byte[] boundaryBytes) throws IOException {
        int b;
        int matchIndex = 0;
        // Skip until start of boundary
        while ((b = is.read()) != -1) {
             if (b == boundaryBytes[matchIndex]) {
                matchIndex++;
                if (matchIndex == boundaryBytes.length) break;
            } else {
                matchIndex = 0;
            }
        }
        // Skip CRLF after boundary
        is.read(); is.read();
    }

    private static String getFilename(String headers) {
        String lowerCaseHeaders = headers.toLowerCase();
        if (lowerCaseHeaders.contains("content-disposition: form-data;")) {
            String[] parts = headers.split(";");
            for (String part : parts) {
                part = part.trim();
                if (part.toLowerCase().startsWith("filename=")) {
                    return part.substring(part.indexOf('=') + 1).trim().replace("\"", "");
                }
            }
        }
        return null;
    }

    private static String getFileExtension(String filename) {
        if (filename == null || filename.lastIndexOf('.') == -1) {
            return "";
        }
        return filename.substring(filename.lastIndexOf('.'));
    }
    
    // Knuth-Morris-Pratt algorithm for finding a subarray (the boundary) in a byte array.
    static class KMPMatch {
        public static int indexOf(byte[] data, int offset, int count, byte[] pattern) {
            int[] lps = computeLPSArray(pattern);
            int i = offset, j = 0;
            int end = offset + count;
            while (i < end) {
                if (pattern[j] == data[i]) {
                    i++; j++;
                }
                if (j == pattern.length) {
                    return i - j;
                } else if (i < end && pattern[j] != data[i]) {
                    if (j != 0) j = lps[j - 1]; else i++;
                }
            }
            return -1;
        }

        private static int[] computeLPSArray(byte[] pattern) {
            int[] lps = new int[pattern.length];
            int length = 0; int i = 1; lps[0] = 0;
            while (i < pattern.length) {
                if (pattern[i] == pattern[length]) {
                    length++; lps[i] = length; i++;
                } else {
                    if (length != 0) length = lps[length - 1]; else {lps[i] = length; i++;}
                }
            }
            return lps;
        }
    }


    private static void runTestCases() {
        System.out.println("\n--- Running Test Cases ---");
        // Test Case 1: Successful upload of a valid .txt file
        testUpload("test1.txt", "This is a valid text file.", "text/plain", true);

        // Test Case 2: Successful upload of a valid .jpg file
        testUpload("test2.jpg", "imagedata", "image/jpeg", true);

        // Test Case 3: Failed upload of a disallowed file type (.html)
        testUpload("test3.html", "<html></html>", "text/html", false);

        // Test Case 4: Failed upload due to path traversal attempt
        testUpload("../../etc/passwd.txt", "malicious content", "text/plain", true); // Server-side sanitization should handle this

        // Test Case 5: Failed upload due to missing filename
        testUpload("", "content with no filename", "application/octet-stream", false);
        System.out.println("--- Test Cases Finished ---\n");
    }

    private static void testUpload(String filename, String content, String contentType, boolean expectSuccess) {
        String testName = "Uploading '" + filename + "' (expect " + (expectSuccess ? "Success" : "Failure") + ")";
        try {
            String boundary = "------------" + UUID.randomUUID().toString();
            URL url = new URL("http://localhost:8080/upload");
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setDoOutput(true);
            conn.setRequestMethod("POST");
            conn.setRequestProperty("Content-Type", "multipart/form-data; boundary=" + boundary);
            
            try (OutputStream os = conn.getOutputStream();
                 PrintWriter writer = new PrintWriter(new OutputStreamWriter(os, StandardCharsets.UTF_8), true)) {

                writer.append("--").append(boundary).append("\r\n");
                writer.append("Content-Disposition: form-data; name=\"file\"; filename=\"").append(filename).append("\"\r\n");
                writer.append("Content-Type: ").append(contentType).append("\r\n");
                writer.append("\r\n").flush();
                os.write(content.getBytes(StandardCharsets.UTF_8));
                os.flush();
                writer.append("\r\n").flush();
                writer.append("--").append(boundary).append("--").append("\r\n").flush();
            }

            int responseCode = conn.getResponseCode();
            boolean success = (responseCode == HttpURLConnection.HTTP_OK);

            if (success == expectSuccess) {
                System.out.println("[PASS] " + testName + " - Response code: " + responseCode);
            } else {
                System.out.println("[FAIL] " + testName + " - Expected " + (expectSuccess ? "success" : "failure") + " but got response code " + responseCode);
            }
            conn.disconnect();
        } catch (Exception e) {
            if (!expectSuccess) {
                System.out.println("[PASS] " + testName + " - Threw expected exception: " + e.getMessage());
            } else {
                System.out.println("[FAIL] " + testName + " - Threw unexpected exception: " + e.getMessage());
            }
        }
    }
}