
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Lse46 {
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/img", new ImgHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000");
        
        // Test cases (commented out as server runs continuously)
        // Test with: http://localhost:8000/img?name=test1.jpg
        // Test with: http://localhost:8000/img?name=test2.png
        // Test with: http://localhost:8000/img?name=test3.gif
        // Test with: http://localhost:8000/img?name=test4.bmp
        // Test with: http://localhost:8000/img?name=test5.jpg
    }
    
    static class ImgHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            String name = "";
            
            if (query != null && query.startsWith("name=")) {
                name = query.substring(5);
            }
            
            String filePath = "images/" + name;
            File file = new File(filePath);
            
            if (file.exists() && !file.isDirectory()) {
                byte[] fileContent = Files.readAllBytes(Paths.get(filePath));
                exchange.sendResponseHeaders(200, fileContent.length);
                OutputStream os = exchange.getResponseBody();
                os.write(fileContent);
                os.close();
            } else {
                String response = "File not found";
                exchange.sendResponseHeaders(404, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        }
    }
}
