
import com.sun.net.httpserver.*;
import java.io.*;
import java.net.*;
import java.nio.file.*;
import java.util.*;

public class Lse65 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final String UPLOAD_DIR = "uploads";
    
    public static void main(String[] args) throws IOException {
        Files.createDirectories(Paths.get(UPLOAD_DIR));
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/", new FormHandler());
        server.createContext("/upload", new UploadHandler());
        server.start();
        System.out.println("Server started on port 8000");
    }
    
    static class FormHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException {
            String html = "<!DOCTYPE html><html><body>" +
                "<form action='/upload' method='post' enctype='multipart/form-data'>" +
                "<input type='file' name='file' accept='.pdf'>" +
                "<input type='submit' value='Upload PDF'></form></body></html>";
            exchange.sendResponseHeaders(200, html.length());
            OutputStream os = exchange.getResponseBody();
            os.write(html.getBytes());
            os.close();
        }
    }
    
    static class UploadHandler implements HttpHandler {
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                String result = handleFileUpload(exchange);
                exchange.sendResponseHeaders(200, result.length());
                OutputStream os = exchange.getResponseBody();
                os.write(result.getBytes());
                os.close();
            }
        }
        
        private String handleFileUpload(HttpExchange exchange) throws IOException {
            InputStream is = exchange.getRequestBody();
            String boundary = extractBoundary(exchange.getRequestHeaders().getFirst("Content-Type"));
            
            if (boundary == null) return "Invalid request";
            
            byte[] data = is.readAllBytes();
            String content = new String(data, "ISO-8859-1");
            
            int fileStart = content.indexOf("\\r\\n\\r\\n") + 4;
            int fileEnd = content.lastIndexOf("\\r\\n--" + boundary);
            
            if (fileStart < 4 || fileEnd < 0) return "No file found";
            
            byte[] fileData = Arrays.copyOfRange(data, fileStart, fileEnd);
            
            if (fileData.length > MAX_FILE_SIZE) return "File too large";
            if (!isPDF(fileData)) return "Invalid PDF file";
            
            String filename = sanitizeFilename("upload_" + System.currentTimeMillis() + ".pdf");
            Path filePath = Paths.get(UPLOAD_DIR, filename).normalize();
            
            if (!filePath.startsWith(Paths.get(UPLOAD_DIR).toAbsolutePath())) {
                return "Invalid path";
            }
            
            Files.write(filePath, fileData);
            return "File uploaded successfully: " + filename;
        }
        
        private String extractBoundary(String contentType) {
            if (contentType == null) return null;
            String[] parts = contentType.split("boundary=");
            return parts.length > 1 ? parts[1] : null;
        }
        
        private boolean isPDF(byte[] data) {
            if (data.length < 5) return false;
            return data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46;
        }
        
        private String sanitizeFilename(String filename) {
            return filename.replaceAll("[^a-zA-Z0-9._-]", "_");
        }
    }
}
