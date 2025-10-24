import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.*;

public class Task96 {
    private static final int MAX_MESSAGE_BYTES = 4096;
    private static final int SERVER_BACKLOG = 50;
    private static final int SOCKET_TIMEOUT_MS = 5000;
    private static final int THREAD_POOL_SIZE = 8;

    public static final class ServerHandle {
        private final ServerSocket serverSocket;
        private final ExecutorService pool;
        private final Thread acceptThread;
        private final int port;
        private volatile boolean running = true;

        private ServerHandle(ServerSocket serverSocket, ExecutorService pool, Thread acceptThread) {
            this.serverSocket = serverSocket;
            this.pool = pool;
            this.acceptThread = acceptThread;
            this.port = serverSocket.getLocalPort();
        }

        public int getPort() {
            return port;
        }
    }

    public static ServerHandle startEchoServer(int port) throws IOException {
        if (port < 0 || port > 65535) {
            throw new IllegalArgumentException("Port out of range");
        }
        ServerSocket serverSocket = new ServerSocket();
        serverSocket.setReuseAddress(true);
        serverSocket.bind(new InetSocketAddress("127.0.0.1", port), SERVER_BACKLOG);

        ExecutorService pool = Executors.newFixedThreadPool(THREAD_POOL_SIZE);
        ServerHandle handle = new ServerHandle(serverSocket, pool, null);

        Thread acceptThread = new Thread(() -> {
            while (handle.running) {
                try {
                    Socket client = serverSocket.accept();
                    client.setSoTimeout(SOCKET_TIMEOUT_MS);
                    pool.execute(() -> handleClient(client));
                } catch (SocketException se) {
                    if (handle.running) {
                        // Unexpected socket exception while running; continue loop
                    } else {
                        break;
                    }
                } catch (IOException ignored) {
                    if (!handle.running) break;
                }
            }
        }, "EchoServer-Accept");
        acceptThread.setDaemon(true);
        acceptThread.start();

        // Set the thread in handle via reflection-free way: recreate handle
        return new ServerHandle(serverSocket, pool, acceptThread);
    }

    private static void handleClient(Socket client) {
        try (Socket c = client;
             InputStream in = c.getInputStream();
             OutputStream out = c.getOutputStream()) {

            while (true) {
                String line = readLineLimited(in, MAX_MESSAGE_BYTES);
                if (line == null) {
                    break; // client closed or timeout
                }
                // Echo back the same message
                byte[] data = line.getBytes(StandardCharsets.UTF_8);
                if (data.length > MAX_MESSAGE_BYTES) {
                    // Should not happen due to read cap, but check anyway
                    break;
                }
                out.write(data);
                out.write('\n');
                out.flush();
            }
        } catch (IOException ignored) {
            // Connection error; close handled by try-with-resources
        }
    }

    private static String readLineLimited(InputStream in, int maxBytes) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream(Math.min(128, maxBytes));
        int readCount = 0;
        while (readCount < maxBytes) {
            int b = in.read();
            if (b == -1) {
                if (baos.size() == 0) return null;
                break;
            }
            readCount++;
            if (b == '\n') break;
            if (b == '\r') continue; // tolerate CRLF
            baos.write(b);
        }
        if (baos.size() == 0 && readCount == 0) return null;
        return baos.toString(StandardCharsets.UTF_8);
    }

    public static void stopEchoServer(ServerHandle handle) {
        if (handle == null) return;
        handle.running = false;
        try {
            handle.serverSocket.close();
        } catch (IOException ignored) {
        }
        handle.pool.shutdown();
        try {
            if (handle.acceptThread != null) {
                handle.acceptThread.join(2000);
            }
        } catch (InterruptedException ignored) {
            Thread.currentThread().interrupt();
        }
        try {
            if (!handle.pool.awaitTermination(2000, TimeUnit.MILLISECONDS)) {
                handle.pool.shutdownNow();
            }
        } catch (InterruptedException e) {
            handle.pool.shutdownNow();
            Thread.currentThread().interrupt();
        }
    }

    public static String sendEcho(String host, int port, String message) throws IOException {
        if (host == null || host.isEmpty()) {
            throw new IllegalArgumentException("Host required");
        }
        if (port < 1 || port > 65535) {
            throw new IllegalArgumentException("Port out of range");
        }
        byte[] msgBytes = message == null ? new byte[0] : message.getBytes(StandardCharsets.UTF_8);
        if (msgBytes.length > MAX_MESSAGE_BYTES) {
            throw new IllegalArgumentException("Message too long");
        }
        try (Socket socket = new Socket()) {
            socket.connect(new InetSocketAddress(host, port), SOCKET_TIMEOUT_MS);
            socket.setSoTimeout(SOCKET_TIMEOUT_MS);
            try (OutputStream out = socket.getOutputStream();
                 InputStream in = socket.getInputStream()) {
                out.write(msgBytes);
                out.write('\n');
                out.flush();
                String echoed = readLineLimited(in, MAX_MESSAGE_BYTES);
                return echoed == null ? "" : echoed;
            }
        }
    }

    public static void main(String[] args) {
        ServerHandle handle = null;
        try {
            handle = startEchoServer(0);
            int port = handle.getPort();
            List<String> tests = Arrays.asList(
                    "Hello, world!",
                    "",
                    "The quick brown fox jumps over 13 lazy dogs.",
                    "Symbols: !@#$%^&*()_+-=[]{}|;:',.<>/?",
                    "One more test line"
            );
            for (String t : tests) {
                String r = sendEcho("127.0.0.1", port, t);
                System.out.println("Sent: [" + t + "] | Echoed: [" + r + "]");
            }
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        } finally {
            stopEchoServer(handle);
        }
    }
}