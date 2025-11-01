
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;

public class Lse71 {
    private static final String UPLOAD_DIR = "uploads";
    private static final int PORT = 8000;

    public static void main(String[] args) throws Exception {
        // Create upload directory if it doesn't exist\n        Files.createDirectories(Paths.get(UPLOAD_DIR));\n        \n        HttpServer server = HttpServer.create(new InetSocketAddress(PORT), 0);\n        server.createContext("/", new HomeHandler());\n        server.createContext("/upload", new UploadHandler());\n        server.setExecutor(null);\n        \n        System.out.println("Server started on port " + PORT);\n        System.out.println("Open http://localhost:" + PORT + " in your browser");\n        server.start();\n    }\n\n    static class HomeHandler implements HttpHandler {\n        @Override\n        public void handle(HttpExchange exchange) throws IOException {\n            String html = "<html><body>" +\n                    "<h2>File Upload Application</h2>" +\n                    "<form action='/upload' method='post' enctype='multipart/form-data'>" +\n                    "<input type='file' name='file' required><br><br>" +\n                    "<input type='submit' value='Upload File'>" +\n                    "</form></body></html>";\n            \n            exchange.sendResponseHeaders(200, html.length());\n            OutputStream os = exchange.getResponseBody();\n            os.write(html.getBytes());\n            os.close();\n        }\n    }\n\n    static class UploadHandler implements HttpHandler {\n        @Override\n        public void handle(HttpExchange exchange) throws IOException {\n            if ("POST".equals(exchange.getRequestMethod())) {\n                String contentType = exchange.getRequestHeaders().getFirst("Content-Type");\n                String boundary = contentType.split("boundary=")[1];\n                \n                InputStream is = exchange.getRequestBody();\n                ByteArrayOutputStream baos = new ByteArrayOutputStream();\n                byte[] buffer = new byte[8192];\n                int bytesRead;\n                while ((bytesRead = is.read(buffer)) != -1) {\n                    baos.write(buffer, 0, bytesRead);\n                }\n                \n                byte[] data = baos.toByteArray();\n                String content = new String(data);\n                \n                // Parse filename\n                String filename = "uploaded_file";\n                int filenameIndex = content.indexOf("filename=\\"");\n                if (filenameIndex != -1) {\n                    int start = filenameIndex + 10;\n                    int end = content.indexOf("\\"", start);\n                    filename = content.substring(start, end);\n                }\n                \n                // Find file data\n                String boundaryStr = "--" + boundary;\n                int fileStart = content.indexOf("\\r\\n\\r\\n") + 4;\n                int fileEnd = content.lastIndexOf(boundaryStr);\n                \n                // Write file\n                byte[] fileData = new byte[fileEnd - fileStart - 2];\n                System.arraycopy(data, fileStart, fileData, 0, fileData.length);\n                \n                File uploadedFile = new File(UPLOAD_DIR, filename);\n                try (FileOutputStream fos = new FileOutputStream(uploadedFile)) {\n                    fos.write(fileData);\n                }\n                \n                String response = "<html><body><h2>File uploaded successfully!</h2>" +\n                        "<p>Filename: " + filename + "</p>" +\n                        "<a href='/'>Upload another file</a></body></html>";
                
                exchange.sendResponseHeaders(200, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        }
    }
}
