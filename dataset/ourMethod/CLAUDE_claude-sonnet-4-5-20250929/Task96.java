
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.*;

public class Task96 {
    private static final int MAX_MESSAGE_SIZE = 8192;
    private static final int SOCKET_TIMEOUT = 30000;
    private static final int MIN_PORT = 1024;
    private static final int MAX_PORT = 65535;

    public static void startEchoServer(int port) throws IOException {
        if (port < MIN_PORT || port > MAX_PORT) {
            throw new IllegalArgumentException("Port must be between " + MIN_PORT + " and " + MAX_PORT);
        }

        try (ServerSocket serverSocket = new ServerSocket(port)) {
            serverSocket.setSoTimeout(SOCKET_TIMEOUT);
            System.out.println("Server listening on port " + port);

            try (Socket clientSocket = serverSocket.accept()) {
                clientSocket.setSoTimeout(SOCKET_TIMEOUT);
                
                try (BufferedReader in = new BufferedReader(
                        new InputStreamReader(clientSocket.getInputStream(), StandardCharsets.UTF_8));
                     PrintWriter out = new PrintWriter(
                        new OutputStreamWriter(clientSocket.getOutputStream(), StandardCharsets.UTF_8), true)) {
                    
                    String message = in.readLine();
                    if (message != null && message.length() <= MAX_MESSAGE_SIZE) {
                        out.println(message);
                    }
                }
            }
        }
    }

    public static void main(String[] args) {
        ExecutorService executor = Executors.newFixedThreadPool(5);
        
        for (int testCase = 1; testCase <= 5; testCase++) {
            final int tc = testCase;
            final int port = 9000 + testCase;
            
            executor.submit(() -> {
                try {
                    Thread serverThread = new Thread(() -> {
                        try {
                            startEchoServer(port);
                        } catch (Exception e) {
                            System.err.println("Test " + tc + " server error: " + e.getMessage());
                        }
                    });
                    serverThread.start();
                    
                    Thread.sleep(500);
                    
                    try (Socket socket = new Socket("localhost", port)) {
                        socket.setSoTimeout(5000);
                        
                        try (PrintWriter out = new PrintWriter(
                                new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8), true);
                             BufferedReader in = new BufferedReader(
                                new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8))) {
                            
                            String testMessage = "Test message " + tc;
                            out.println(testMessage);
                            
                            String response = in.readLine();
                            System.out.println("Test " + tc + ": Sent='" + testMessage + "', Received='" + response + "'");
                        }
                    }
                    
                    serverThread.join(2000);
                } catch (Exception e) {
                    System.err.println("Test " + tc + " failed: " + e.getMessage());
                }
            });
        }
        
        executor.shutdown();
        try {
            executor.awaitTermination(15, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}
