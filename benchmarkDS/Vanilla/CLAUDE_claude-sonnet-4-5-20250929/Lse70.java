
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Lse70 {
    private static final String UPLOAD_DIR = "uploads";

    static class FileUploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                upload_file(exchange);
            } else {
                String response = "Method not allowed";
                exchange.sendResponseHeaders(405, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        }
    }

    public static void upload_file(HttpExchange exchange) throws IOException {
        Files.createDirectories(Paths.get(UPLOAD_DIR));
        
        InputStream is = exchange.getRequestBody();
        String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
        
        if (contentType != null && contentType.contains("multipart/form-data")) {
            String boundary = contentType.split("boundary=")[1];
            BufferedReader reader = new BufferedReader(new InputStreamReader(is));
            
            String line;
            String filename = null;
            boolean fileDataStarted = false;
            ByteArrayOutputStream fileContent = new ByteArrayOutputStream();
            
            while ((line = reader.readLine()) != null) {
                if (line.contains("Content-Disposition")) {
                    int filenameIndex = line.indexOf("filename=\\"");
                    if (filenameIndex != -1) {
                        int startIndex = filenameIndex + 10;
                        int endIndex = line.indexOf("\\"", startIndex);
                        filename = line.substring(startIndex, endIndex);
                    }
                } else if (line.contains("Content-Type:")) {
                    reader.readLine();
                    fileDataStarted = true;
                } else if (line.contains("--" + boundary)) {
                    fileDataStarted = false;
                    if (filename != null && fileContent.size() > 0) {
                        break;
                    }
                } else if (fileDataStarted) {
                    fileContent.write(line.getBytes());
                    fileContent.write('\\n');
                }
            }
            
            if (filename != null) {
                FileOutputStream fos = new FileOutputStream(UPLOAD_DIR + "/" + filename);
                fos.write(fileContent.toByteArray());
                fos.close();
                
                String response = "File uploaded successfully: " + filename;
                exchange.sendResponseHeaders(200, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        }
    }

    public static void main(String[] args) throws IOException {
        System.out.println("Starting file upload server on port 8000...");
        System.out.println("Upload directory: " + UPLOAD_DIR);
        
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/upload", new FileUploadHandler());
        server.setExecutor(null);
        server.start();
        
        System.out.println("Server started. Send POST requests to http://localhost:8000/upload");
    }
}
