import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.*;

public class Task96 {

    // Secure Echo Server with bounded thread pool and timeouts
    public static class EchoServer implements Runnable {
        private static final int MAX_MESSAGE_LEN = 8192; // 8KB limit
        private static final int CLIENT_SO_TIMEOUT_MS = 5000;
        private static final int SERVER_ACCEPT_TIMEOUT_MS = 1000;

        private final ServerSocket serverSocket;
        private final ExecutorService pool;
        private final Thread acceptThread;
        private volatile boolean running = false;

        public EchoServer(int port, int maxClients) throws IOException {
            if (port < 0 || port > 65535) {
                throw new IllegalArgumentException("Port out of range");
            }
            if (maxClients <= 0 || maxClients > 1024) {
                throw new IllegalArgumentException("Invalid maxClients");
            }
            this.serverSocket = new ServerSocket();
            this.serverSocket.setReuseAddress(true);
            this.serverSocket.bind(new InetSocketAddress("127.0.0.1", port));
            this.serverSocket.setSoTimeout(SERVER_ACCEPT_TIMEOUT_MS);
            this.pool = new ThreadPoolExecutor(
                    Math.min(2, maxClients),
                    maxClients,
                    60L, TimeUnit.SECONDS,
                    new LinkedBlockingQueue<>(maxClients),
                    r -> {
                        Thread t = new Thread(r);
                        t.setDaemon(true);
                        t.setName("EchoServer-Worker-" + t.getId());
                        return t;
                    },
                    new ThreadPoolExecutor.AbortPolicy()
            );
            this.acceptThread = new Thread(this, "EchoServer-Acceptor");
            this.acceptThread.setDaemon(true);
        }

        public int getPort() {
            return serverSocket.getLocalPort();
        }

        public void start() {
            running = true;
            acceptThread.start();
        }

        public void stop() {
            running = false;
            try {
                serverSocket.close();
            } catch (IOException ignored) {}
            pool.shutdownNow();
            try {
                acceptThread.join(3000);
            } catch (InterruptedException ignored) {
                Thread.currentThread().interrupt();
            }
        }

        @Override
        public void run() {
            while (running) {
                try {
                    Socket client = serverSocket.accept();
                    client.setSoTimeout(CLIENT_SO_TIMEOUT_MS);
                    try {
                        pool.execute(() -> handleClient(client));
                    } catch (RejectedExecutionException ex) {
                        try (client) {
                            // Server overloaded, close client gracefully
                        } catch (IOException ignored) {}
                    }
                } catch (SocketTimeoutException ste) {
                    // periodic check for running flag
                } catch (SocketException se) {
                    // Likely closed during stop()
                    break;
                } catch (IOException ioe) {
                    // Unexpected IO error; continue if running
                }
            }
        }

        private void handleClient(Socket client) {
            try (Socket c = client;
                 InputStream in = new BufferedInputStream(c.getInputStream());
                 OutputStream out = new BufferedOutputStream(c.getOutputStream())) {
                byte[] response = readMessageUpToNewline(in, MAX_MESSAGE_LEN);
                out.write(response);
                out.flush();
            } catch (IOException ignored) {
                // Ignore client-specific errors
            }
        }

        // Reads until newline or EOF or limit; returns bytes excluding newline
        private static byte[] readMessageUpToNewline(InputStream in, int maxLen) throws IOException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream(Math.min(maxLen, 256));
            byte[] buf = new byte[512];
            int total = 0;
            boolean done = false;

            while (!done && total < maxLen) {
                int toRead = Math.min(buf.length, maxLen - total);
                int n = in.read(buf, 0, toRead);
                if (n == -1) break;
                int end = -1;
                for (int i = 0; i < n; i++) {
                    if (buf[i] == (byte) '\n') {
                        end = i;
                        break;
                    }
                }
                if (end >= 0) {
                    baos.write(buf, 0, end);
                    total += end;
                    done = true;
                } else {
                    baos.write(buf, 0, n);
                    total += n;
                }
            }
            return baos.toByteArray();
        }
    }

    // Simple client for tests
    public static String echoClient(String host, int port, String message) throws IOException {
        if (host == null || host.isEmpty()) throw new IllegalArgumentException("Invalid host");
        if (port <= 0 || port > 65535) throw new IllegalArgumentException("Invalid port");
        if (message == null) message = "";

        try (Socket s = new Socket()) {
            s.connect(new InetSocketAddress(host, port), 3000);
            s.setSoTimeout(5000);
            try (OutputStream out = new BufferedOutputStream(s.getOutputStream());
                 InputStream in = new BufferedInputStream(s.getInputStream())) {
                byte[] data = message.getBytes(StandardCharsets.UTF_8);
                if (data.length > 8192) {
                    data = Arrays.copyOf(data, 8192); // enforce max length similar to server
                }
                out.write(data);
                out.write('\n'); // delimiter
                out.flush();
                // read echoed data until EOF (server closes after echo)
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                byte[] buf = new byte[512];
                int n;
                while ((n = in.read(buf)) != -1) {
                    baos.write(buf, 0, n);
                    if (baos.size() > 8192) break; // safety
                }
                return baos.toString(StandardCharsets.UTF_8);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        EchoServer server = new EchoServer(0, 16);
        server.start();
        int port = server.getPort();

        List<String> tests = List.of(
                "hello",
                "test message",
                "",
                "A".repeat(100),
                "Line1\\nLine2 with symbols !@#$%^&*()_+"
        );

        for (int i = 0; i < tests.size(); i++) {
            String msg = tests.get(i);
            String resp = echoClient("127.0.0.1", port, msg);
            System.out.println("Case " + (i + 1) + ": " + resp);
        }
        server.stop();
    }
}