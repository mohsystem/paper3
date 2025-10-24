import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.*;

/*
Chain-of-Through process in code generation:
1. Problem understanding:
   - Build a TCP echo server that listens on a specified port and echoes back received messages.
   - Inputs: port for server; for client tests: host, port, message.
   - Outputs: echoed messages from server.
2. Security requirements:
   - Bind to localhost only.
   - Use timeouts; limit input sizes; handle exceptions; close resources reliably; use thread pool to avoid unbounded threads.
3. Secure coding generation:
   - Implement bounded line reading; UTF-8 encoding; controlled thread pool; graceful shutdown.
4. Code review:
   - Ensure all sockets/streams are closed; timeouts set; input length limited; no shared mutable state without proper handling.
5. Secure code output:
   - Final implementation with mitigations applied.
*/

public class Task96 {

    private static final int BACKLOG = 50;
    private static final int MAX_LINE = 8192;
    private static final int CLIENT_SO_TIMEOUT_MS = 30000;
    private static final int SERVER_SO_TIMEOUT_MS = 1000;

    public static class EchoServer implements Closeable {
        private final ExecutorService pool;
        private volatile boolean running = false;
        private Thread acceptThread;
        private ServerSocket serverSocket;

        public EchoServer() {
            this.pool = new ThreadPoolExecutor(
                    2, 16, 60L, TimeUnit.SECONDS,
                    new LinkedBlockingQueue<>(100),
                    r -> {
                        Thread t = new Thread(r, "EchoServer-Worker");
                        t.setDaemon(true);
                        return t;
                    },
                    new ThreadPoolExecutor.AbortPolicy()
            );
        }

        public synchronized int start(int port) throws IOException {
            if (running) throw new IllegalStateException("Server already running");
            serverSocket = new ServerSocket();
            serverSocket.setReuseAddress(true);
            serverSocket.bind(new InetSocketAddress("127.0.0.1", port), BACKLOG);
            serverSocket.setSoTimeout(SERVER_SO_TIMEOUT_MS);
            running = true;

            acceptThread = new Thread(this::acceptLoop, "EchoServer-Acceptor");
            acceptThread.setDaemon(true);
            acceptThread.start();
            return serverSocket.getLocalPort();
        }

        private void acceptLoop() {
            while (running) {
                try {
                    Socket client = serverSocket.accept();
                    pool.execute(() -> handleClient(client));
                } catch (SocketTimeoutException ste) {
                    // periodic timeout allows checking running flag
                } catch (IOException e) {
                    if (running) {
                        // log to stderr safely
                        System.err.println("Accept error: " + e.getMessage());
                    }
                }
            }
        }

        private static String readLineLimited(Reader reader, int maxLen) throws IOException {
            StringBuilder sb = new StringBuilder();
            while (true) {
                int c = reader.read();
                if (c == -1) {
                    if (sb.length() == 0) return null;
                    break;
                }
                if (c == '\n') break;
                if (c != '\r') {
                    if (sb.length() >= maxLen) throw new IOException("Input line too long");
                    sb.append((char) c);
                }
            }
            return sb.toString();
        }

        private void handleClient(Socket client) {
            try (Socket c = client) {
                c.setSoTimeout(CLIENT_SO_TIMEOUT_MS);
                InputStream in = c.getInputStream();
                OutputStream out = c.getOutputStream();

                try (InputStreamReader isr = new InputStreamReader(in, StandardCharsets.UTF_8);
                     BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(out, StandardCharsets.UTF_8))) {

                    while (true) {
                        String line = readLineLimited(isr, MAX_LINE);
                        if (line == null) break;
                        writer.write(line);
                        writer.write('\n');
                        writer.flush();
                    }
                }
            } catch (IOException e) {
                // Log minimal info; socket will be closed by try-with-resources
                System.err.println("Client handler error: " + e.getMessage());
            }
        }

        public synchronized void stop() {
            running = false;
            if (serverSocket != null) {
                try {
                    serverSocket.close();
                } catch (IOException ignored) {
                }
            }
            if (acceptThread != null) {
                try {
                    acceptThread.join(2000);
                } catch (InterruptedException ignored) {
                    Thread.currentThread().interrupt();
                }
            }
            pool.shutdown();
            try {
                if (!pool.awaitTermination(2000, TimeUnit.MILLISECONDS)) {
                    pool.shutdownNow();
                }
            } catch (InterruptedException e) {
                pool.shutdownNow();
                Thread.currentThread().interrupt();
            }
        }

        @Override
        public void close() {
            stop();
        }
    }

    public static int startEchoServer(int port) throws IOException {
        // Returns the actual bound port
        EchoServer server = new EchoServer();
        int actualPort = server.start(port);
        // Store server in a static registry if needed; for demo we'll keep it in a field
        serverHolder = server;
        return actualPort;
    }

    private static EchoServer serverHolder;

    public static void stopEchoServer() {
        if (serverHolder != null) {
            serverHolder.stop();
            serverHolder = null;
        }
    }

    private static String readLineLimited(BufferedReader reader, int maxLen) throws IOException {
        StringBuilder sb = new StringBuilder();
        while (true) {
            int c = reader.read();
            if (c == -1) {
                if (sb.length() == 0) return null;
                break;
            }
            if (c == '\n') break;
            if (c != '\r') {
                if (sb.length() >= maxLen) throw new IOException("Input line too long");
                sb.append((char) c);
            }
        }
        return sb.toString();
    }

    public static String echoClient(String host, int port, String message) throws IOException {
        try (Socket socket = new Socket()) {
            socket.connect(new InetSocketAddress(host, port), 5000);
            socket.setSoTimeout(CLIENT_SO_TIMEOUT_MS);
            try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8));
                 BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8))) {
                writer.write(message);
                writer.write('\n');
                writer.flush();
                String response = readLineLimited(reader, MAX_LINE);
                return response == null ? "" : response;
            }
        }
    }

    public static void main(String[] args) throws Exception {
        int port = startEchoServer(0);
        String host = "127.0.0.1";

        String[] tests = new String[] {
                "Hello",
                "",
                "The quick brown fox jumps over the lazy dog",
                "1234567890!@#$%^&*()_+",
                "Unicode: Привет, こんにちは, 🙂"
        };

        for (int i = 0; i < tests.length; i++) {
            String resp = echoClient(host, port, tests[i]);
            System.out.println("Test " + (i + 1) + " -> " + resp);
        }

        stopEchoServer();
    }
}