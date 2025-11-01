
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.UUID;

public class Lse71 {
    private static final String UPLOAD_DIR = "./uploads/";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList("txt", "pdf", "jpg", "jpeg", "png", "gif", "doc", "docx")
    );

    public static void main(String[] args) throws Exception {
        // Create upload directory if it doesn't exist\n        Files.createDirectories(Paths.get(UPLOAD_DIR));\n        \n        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);\n        server.createContext("/upload", new FileUploadHandler());\n        server.createContext("/", new HomeHandler());\n        server.setExecutor(null);\n        System.out.println("Server started on port 8080");\n        server.start();\n    }\n\n    static class HomeHandler implements HttpHandler {\n        @Override\n        public void handle(HttpExchange exchange) throws IOException {\n            String response = "<html><body>" +\n                "<h2>Secure File Upload</h2>" +\n                "<form action='/upload' method='post' enctype='multipart/form-data'>" +\n                "<input type='file' name='file' required>" +\n                "<input type='submit' value='Upload'>" +\n                "</form></body></html>";\n            \n            exchange.getResponseHeaders().set("Content-Type", "text/html");\n            exchange.sendResponseHeaders(200, response.length());\n            OutputStream os = exchange.getResponseBody();\n            os.write(response.getBytes());\n            os.close();\n        }\n    }\n\n    static class FileUploadHandler implements HttpHandler {\n        @Override\n        public void handle(HttpExchange exchange) throws IOException {\n            if (!"POST".equals(exchange.getRequestMethod())) {\n                sendResponse(exchange, 405, "Method not allowed");\n                return;\n            }\n\n            try {\n                InputStream is = exchange.getRequestBody();\n                ByteArrayOutputStream buffer = new ByteArrayOutputStream();\n                byte[] data = new byte[1024];\n                int nRead;\n                long totalSize = 0;\n\n                while ((nRead = is.read(data, 0, data.length)) != -1) {\n                    totalSize += nRead;\n                    if (totalSize > MAX_FILE_SIZE) {\n                        sendResponse(exchange, 413, "File too large");\n                        return;\n                    }\n                    buffer.write(data, 0, nRead);\n                }\n\n                byte[] fileData = buffer.toByteArray();\n                String filename = extractFilename(fileData);\n                \n                if (filename == null || !isValidFile(filename)) {\n                    sendResponse(exchange, 400, "Invalid file type");\n                    return;\n                }\n\n                String safeName = sanitizeFilename(filename);\n                String uniqueName = UUID.randomUUID().toString() + "_" + safeName;\n                Files.write(Paths.get(UPLOAD_DIR + uniqueName), extractFileContent(fileData));\n\n                sendResponse(exchange, 200, "File uploaded successfully: " + uniqueName);\n            } catch (Exception e) {\n                sendResponse(exchange, 500, "Upload failed: " + e.getMessage());\n            }\n        }\n\n        private String extractFilename(byte[] data) {\n            String content = new String(data, 0, Math.min(500, data.length));\n            int idx = content.indexOf("filename=\\"");\n            if (idx != -1) {\n                int endIdx = content.indexOf("\\"", idx + 10);\n                return content.substring(idx + 10, endIdx);\n            }\n            return null;\n        }\n\n        private byte[] extractFileContent(byte[] data) {\n            String content = new String(data);\n            int startIdx = content.indexOf("\\r\\n\\r\\n") + 4;\n            int endIdx = content.lastIndexOf("\\r\\n--");\n            return Arrays.copyOfRange(data, startIdx, endIdx);\n        }\n\n        private boolean isValidFile(String filename) {\n            String ext = getFileExtension(filename).toLowerCase();\n            return ALLOWED_EXTENSIONS.contains(ext);\n        }\n\n        private String getFileExtension(String filename) {\n            int lastDot = filename.lastIndexOf('.');
            return lastDot > 0 ? filename.substring(lastDot + 1) : "";
        }

        private String sanitizeFilename(String filename) {
            return filename.replaceAll("[^a-zA-Z0-9._-]", "_");
        }

        private void sendResponse(HttpExchange exchange, int code, String message) throws IOException {
            exchange.sendResponseHeaders(code, message.length());
            OutputStream os = exchange.getResponseBody();
            os.write(message.getBytes());
            os.close();
        }
    }
}
