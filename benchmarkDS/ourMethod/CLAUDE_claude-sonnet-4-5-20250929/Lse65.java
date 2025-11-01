
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse65 {
    private static final String UPLOAD_DIR = "uploads";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final int MAX_FILENAME_LENGTH = 255;
    
    public static void main(String[] args) throws Exception {
        // Create uploads directory with restricted permissions
        Path uploadPath = Paths.get(UPLOAD_DIR);
        if (!Files.exists(uploadPath)) {
            Files.createDirectory(uploadPath);
            // Set directory permissions to owner-only on POSIX systems
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                perms.add(PosixFilePermission.OWNER_EXECUTE);
                Files.setPosixFilePermissions(uploadPath, perms);
            } catch (UnsupportedOperationException e) {
                // Not a POSIX system, skip permission setting
            }
        }
        
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/upload", new UploadHandler());
        server.createContext("/", new FormHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000");
        
        // Test cases
        System.out.println("\\nTest Cases:");
        System.out.println("1. Valid PDF upload");
        System.out.println("2. File too large");
        System.out.println("3. Invalid filename with path traversal");
        System.out.println("4. Non-PDF file");
        System.out.println("5. Empty file");
    }
    
    static class FormHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException {
            String html = "<!DOCTYPE html><html><head><title>PDF Upload</title></head><body>" +
                    "<h1>Upload PDF File</h1>" +
                    "<form action='/upload' method='post' enctype='multipart/form-data'>" +
                    "<input type='file' name='file' accept='.pdf' required>" +
                    "<input type='submit' value='Upload'>" +
                    "</form></body></html>";
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, html.length());
            OutputStream os = exchange.getResponseBody();
            os.write(html.getBytes("UTF-8"));
            os.close();
        }
    }
    
    static class UploadHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method not allowed");
                return;
            }
            
            try {
                String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
                if (contentType == null || !contentType.startsWith("multipart/form-data")) {
                    sendResponse(exchange, 400, "Invalid content type");
                    return;
                }
                
                String boundary = extractBoundary(contentType);
                if (boundary == null) {
                    sendResponse(exchange, 400, "Missing boundary");
                    return;
                }
                
                InputStream is = exchange.getRequestBody();
                byte[] data = readInputStream(is, MAX_FILE_SIZE);
                
                MultipartData parsed = parseMultipart(data, boundary);
                if (parsed == null || parsed.filename == null || parsed.content == null) {
                    sendResponse(exchange, 400, "Invalid upload data");
                    return;
                }
                
                String result = saveFile(parsed.filename, parsed.content);
                sendResponse(exchange, 200, result);
                
            } catch (Exception e) {
                sendResponse(exchange, 500, "Upload failed");
            }
        }
        
        private String extractBoundary(String contentType) {
            String[] parts = contentType.split(";");
            for (String part : parts) {
                part = part.trim();
                if (part.startsWith("boundary=")) {
                    return part.substring(9);
                }
            }
            return null;
        }
        
        private byte[] readInputStream(InputStream is, long maxSize) throws IOException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            byte[] buffer = new byte[8192];
            int read;
            long total = 0;
            
            while ((read = is.read(buffer)) != -1) {
                total += read;
                if (total > maxSize) {
                    throw new IOException("File too large");
                }
                baos.write(buffer, 0, read);
            }
            return baos.toByteArray();
        }
        
        private MultipartData parseMultipart(byte[] data, String boundary) {
            try {
                String boundaryStr = "--" + boundary;
                String dataStr = new String(data, "ISO-8859-1");
                int start = dataStr.indexOf(boundaryStr);
                int end = dataStr.indexOf(boundaryStr, start + boundaryStr.length());
                
                if (start == -1 || end == -1) return null;
                
                String part = dataStr.substring(start + boundaryStr.length(), end);
                int headerEnd = part.indexOf("\\r\\n\\r\\n");
                if (headerEnd == -1) return null;
                
                String headers = part.substring(0, headerEnd);
                String filename = extractFilename(headers);
                
                if (filename == null) return null;
                
                int contentStart = start + boundaryStr.length() + headerEnd + 4;
                int contentEnd = end - 2;
                byte[] content = new byte[contentEnd - contentStart];
                System.arraycopy(data, contentStart, content, 0, content.length);
                
                return new MultipartData(filename, content);
            } catch (Exception e) {
                return null;
            }
        }
        
        private String extractFilename(String headers) {
            String[] lines = headers.split("\\r\\n");
            for (String line : lines) {
                if (line.toLowerCase().contains("content-disposition")) {
                    int fnameIdx = line.indexOf("filename=\\"");
                    if (fnameIdx != -1) {
                        int endIdx = line.indexOf("\\"", fnameIdx + 10);
                        if (endIdx != -1) {
                            return line.substring(fnameIdx + 10, endIdx);
                        }
                    }
                }
            }
            return null;
        }
        
        private void sendResponse(HttpExchange exchange, int code, String message) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
            exchange.sendResponseHeaders(code, message.length());
            OutputStream os = exchange.getResponseBody();
            os.write(message.getBytes("UTF-8"));
            os.close();
        }
    }
    
    // Validate and save file securely
    public static String saveFile(String filename, byte[] content) throws Exception {
        // Input validation: check filename length
        if (filename == null || filename.isEmpty() || filename.length() > MAX_FILENAME_LENGTH) {
            throw new IllegalArgumentException("Invalid filename");
        }
        
        // Sanitize filename: remove path components and special characters
        filename = Paths.get(filename).getFileName().toString();
        filename = filename.replaceAll("[^a-zA-Z0-9._-]", "_");
        
        // Validate PDF extension
        if (!filename.toLowerCase().endsWith(".pdf")) {
            throw new IllegalArgumentException("Only PDF files allowed");
        }
        
        // Check file size
        if (content == null || content.length == 0 || content.length > MAX_FILE_SIZE) {
            throw new IllegalArgumentException("Invalid file size");
        }
        
        // Validate PDF magic bytes (PDF header)
        if (content.length < 5 || content[0] != 0x25 || content[1] != 0x50 || 
            content[2] != 0x44 || content[3] != 0x46 || content[4] != 0x2D) {
            throw new IllegalArgumentException("Not a valid PDF file");
        }
        
        // Generate unique filename to prevent overwrites
        SecureRandom random = new SecureRandom();
        byte[] randomBytes = new byte[16];
        random.nextBytes(randomBytes);
        String uniqueId = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
        String safeName = uniqueId + "_" + filename;
        
        // Resolve base directory and ensure it exists
        Path baseDir = Paths.get(UPLOAD_DIR).toRealPath();
        Path targetPath = baseDir.resolve(safeName);
        
        // Verify resolved path is within base directory (prevent path traversal)
        if (!targetPath.normalize().startsWith(baseDir)) {
            throw new SecurityException("Path traversal attempt detected");
        }
        
        // Write to temporary file first with restricted permissions
        Path tempFile = Files.createTempFile(baseDir, "upload_", ".tmp");
        try {
            // Set restrictive permissions on temp file
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(tempFile, perms);
            } catch (UnsupportedOperationException e) {
                // Not POSIX, skip
            }
            
            Files.write(tempFile, content, StandardOpenOption.WRITE);
            
            // Atomic move to final location
            Files.move(tempFile, targetPath, StandardCopyOption.ATOMIC_MOVE, 
                      StandardCopyOption.REPLACE_EXISTING);
            
            return "File uploaded successfully: " + safeName;
            
        } catch (Exception e) {
            // Clean up temp file on failure
            try {
                Files.deleteIfExists(tempFile);
            } catch (IOException ignored) {}
            throw e;
        }
    }
    
    static class MultipartData {
        String filename;
        byte[] content;
        
        MultipartData(String filename, byte[] content) {
            this.filename = filename;
            this.content = content;
        }
    }
}
