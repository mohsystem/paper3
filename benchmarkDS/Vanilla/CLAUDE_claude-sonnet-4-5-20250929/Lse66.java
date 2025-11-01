
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;

public class Lse66 {
    private static final String UPLOAD_DIR = "uploads";
    
    public static void handlePdfUpload(InputStream inputStream, String filename) throws IOException {
        File uploadDir = new File(UPLOAD_DIR);
        if (!uploadDir.exists()) {
            uploadDir.mkdirs();
        }
        
        File destinationFile = new File(uploadDir, filename);
        Files.copy(inputStream, destinationFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
    }
    
    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
                
                if (contentType != null && contentType.contains("multipart/form-data")) {
                    InputStream is = exchange.getRequestBody();
                    String filename = "uploaded_file.pdf";
                    
                    handlePdfUpload(is, filename);
                    
                    String response = "File uploaded successfully!";
                    exchange.sendResponseHeaders(200, response.length());
                    OutputStream os = exchange.getResponseBody();
                    os.write(response.getBytes());
                    os.close();
                } else {
                    String response = "Invalid content type";
                    exchange.sendResponseHeaders(400, response.length());
                    OutputStream os = exchange.getResponseBody();
                    os.write(response.getBytes());
                    os.close();
                }
            }
        }
    }
    
    public static void main(String[] args) throws Exception {
        System.out.println("PDF Upload Web App - Java");
        System.out.println("Testing file upload functionality...");
        
        // Test case 1: Create upload directory
        File uploadDir = new File(UPLOAD_DIR);
        if (!uploadDir.exists()) {
            uploadDir.mkdirs();
            System.out.println("Test 1: Upload directory created");
        }
        
        // Test case 2: Simulate PDF upload
        String testContent = "Test PDF content";
        ByteArrayInputStream testStream = new ByteArrayInputStream(testContent.getBytes());
        handlePdfUpload(testStream, "test1.pdf");
        System.out.println("Test 2: File test1.pdf uploaded");
        
        // Test case 3: Upload another file
        testStream = new ByteArrayInputStream("Another test".getBytes());
        handlePdfUpload(testStream, "test2.pdf");
        System.out.println("Test 3: File test2.pdf uploaded");
        
        // Test case 4: Replace existing file
        testStream = new ByteArrayInputStream("Updated content".getBytes());
        handlePdfUpload(testStream, "test1.pdf");
        System.out.println("Test 4: File test1.pdf replaced");
        
        // Test case 5: Upload with special characters in name
        testStream = new ByteArrayInputStream("Special test".getBytes());
        handlePdfUpload(testStream, "test_special_123.pdf");
        System.out.println("Test 5: File test_special_123.pdf uploaded");
        
        System.out.println("\\nAll tests completed. Check 'uploads' directory for files.");
    }
}
