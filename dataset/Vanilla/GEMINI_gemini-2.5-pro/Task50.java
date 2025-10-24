import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.concurrent.Executors;
import java.util.UUID;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Task50 {

    private static final int PORT = 8080;
    private static final String UPLOAD_DIR = "uploads_java";
    private static HttpServer server;

    public static void startServer() throws IOException {
        Files.createDirectories(Paths.get(UPLOAD_DIR));
        server = HttpServer.create(new InetSocketAddress(PORT), 0);
        server.createContext("/upload", new FileUploadHandler());
        server.setExecutor(Executors.newSingleThreadExecutor());
        server.start();
        System.out.println("Java server started on port " + PORT);
    }

    public static void stopServer() {
        if (server != null) {
            server.stop(0);
            System.out.println("Java server stopped.");
        }
    }

    static class FileUploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode;

            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                try (InputStream is = exchange.getRequestBody()) {
                    byte[] fileBytes = is.readAllBytes();
                    String fileName = UUID.randomUUID().toString() + ".tmp";
                    Path filePath = Paths.get(UPLOAD_DIR, fileName);
                    Files.write(filePath, fileBytes);

                    response = "File uploaded successfully to " + filePath.toAbsolutePath();
                    statusCode = 200;
                    System.out.println("Server received and saved file: " + filePath.toAbsolutePath());

                } catch (IOException e) {
                    response = "Error uploading file: " + e.getMessage();
                    statusCode = 500;
                    e.printStackTrace();
                }
            } else {
                response = "Method not allowed. Please use POST.";
                statusCode = 405;
            }

            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }

    public static String uploadFile(String serverUrl, String fileName, byte[] fileContent) {
        try {
            HttpClient client = HttpClient.newHttpClient();
            HttpRequest request = HttpRequest.newBuilder()
                    .uri(URI.create(serverUrl))
                    .POST(HttpRequest.BodyPublishers.ofByteArray(fileContent))
                    .header("Content-Type", "application/octet-stream")
                    .build();

            HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());
            return "Client Response (" + response.statusCode() + "): " + response.body();
        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
            return "Client Error: " + e.getMessage();
        }
    }

    public static void main(String[] args) throws Exception {
        // 1. Start the server in a background thread
        Thread serverThread = new Thread(() -> {
            try {
                startServer();
            } catch (IOException e) {
                System.err.println("Failed to start server: " + e.getMessage());
            }
        });
        serverThread.start();
        
        // Give the server a moment to start up
        Thread.sleep(1000);

        // 2. Run test cases
        System.out.println("\n--- Running Java Test Cases ---");
        String serverUrl = "http://localhost:" + PORT + "/upload";
        
        for (int i = 1; i <= 5; i++) {
            String fileName = "test" + i + ".txt";
            String content = "This is test file " + i + ".";
            System.out.println("Test " + i + ": Uploading '" + fileName + "'...");
            String result = uploadFile(serverUrl, fileName, content.getBytes());
            System.out.println(result);
        }
        System.out.println("--- Java Test Cases Finished ---\n");


        // 3. Stop the server
        stopServer();
        // The server's executor needs to be shut down if it's not a daemon
        System.exit(0);
    }
}