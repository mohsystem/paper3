import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.CopyOnWriteArrayList;

public class Task90 {

    // Step 1: Problem understanding (high-level): Multi-client chat server with broadcast.
    // Step 2: Security requirements: Limit clients, message size, timeouts, sanitize, local bind, safe concurrency.
    // Step 3: Secure coding generation: Implement with resource management and safe patterns.
    // Step 4: Code review: In-code comments highlight safety checks and mitigations.
    // Step 5: Secure code output: Final implementation.

    public static class ChatServer {
        private final AtomicBoolean running = new AtomicBoolean(false);
        private final List<ClientHandler> clients = new CopyOnWriteArrayList<>();
        private final ExecutorService pool;
        private final int maxClients;
        private ServerSocket serverSocket;
        private Thread acceptThread;

        private static final int DEFAULT_BACKLOG = 50;
        private static final int SOCKET_READ_TIMEOUT_MS = 15000;   // avoid stuck sockets
        private static final int ACCEPT_TIMEOUT_MS = 1000;         // allow graceful shutdown
        private static final int MAX_MESSAGE_LEN = 1024;           // limit message size

        public ChatServer(int maxClients) {
            this.maxClients = Math.max(1, maxClients);
            // Fixed thread pool to avoid unbounded thread creation
            this.pool = Executors.newFixedThreadPool(Math.min(64, Math.max(4, this.maxClients + 2)));
        }

        public int start(int port) throws IOException {
            if (running.get()) throw new IllegalStateException("Server already running");
            // Bind to localhost only for safety
            InetSocketAddress addr = new InetSocketAddress(InetAddress.getByName("127.0.0.1"), port);
            serverSocket = new ServerSocket();
            serverSocket.setReuseAddress(true);
            serverSocket.bind(addr, DEFAULT_BACKLOG);
            serverSocket.setSoTimeout(ACCEPT_TIMEOUT_MS);

            running.set(true);
            acceptThread = new Thread(this::acceptLoop, "ChatServer-AcceptLoop");
            acceptThread.setDaemon(true);
            acceptThread.start();
            return serverSocket.getLocalPort(); // return the actual bound port (useful if port 0 was used)
        }

        private void acceptLoop() {
            while (running.get()) {
                try {
                    Socket s = serverSocket.accept();
                    // Enforce connection throttling and security guardrails
                    s.setTcpNoDelay(true);
                    s.setSoTimeout(SOCKET_READ_TIMEOUT_MS);
                    if (clients.size() >= maxClients) {
                        safeClose(s);
                        continue;
                    }
                    ClientHandler handler = new ClientHandler(s, this);
                    clients.add(handler);
                    pool.execute(handler);
                } catch (SocketTimeoutException ste) {
                    // periodic timeout to check running state
                } catch (IOException e) {
                    if (running.get()) {
                        // log suppressed
                    }
                }
            }
        }

        public void stop() {
            if (!running.getAndSet(false)) return;
            safeClose(serverSocket);
            for (ClientHandler c : clients) {
                c.close();
            }
            clients.clear();
            if (acceptThread != null) {
                try { acceptThread.join(2000); } catch (InterruptedException ignored) { Thread.currentThread().interrupt(); }
            }
            pool.shutdownNow();
        }

        private static void safeClose(Closeable c) {
            if (c != null) {
                try { c.close(); } catch (IOException ignored) {}
            }
        }
        private static void safeClose(Socket s) {
            if (s != null) {
                try { s.close(); } catch (IOException ignored) {}
            }
        }

        void removeClient(ClientHandler h) {
            clients.remove(h);
        }

        void broadcast(String message, ClientHandler from) {
            String sanitized = sanitizeMessage(message);
            if (sanitized.isEmpty()) return;
            String prefixed = "[Client-" + from.id + "]: " + sanitized + "\n";
            byte[] data = prefixed.getBytes(StandardCharsets.UTF_8);
            for (ClientHandler c : clients) {
                if (c != from) {
                    c.send(data);
                }
            }
        }

        private static String sanitizeMessage(String input) {
            if (input == null) return "";
            StringBuilder sb = new StringBuilder(Math.min(input.length(), MAX_MESSAGE_LEN));
            int count = 0;
            for (int i = 0; i < input.length() && count < MAX_MESSAGE_LEN; i++) {
                char ch = input.charAt(i);
                // Permit common printable characters; drop control chars
                if (ch >= 32 && ch != 127) {
                    sb.append(ch);
                    count++;
                }
            }
            return sb.toString().trim();
        }

        private static class ClientHandler implements Runnable {
            private static final AtomicInteger ID_GEN = new AtomicInteger(1);
            private final int id = ID_GEN.getAndIncrement();
            private final ChatServer server;
            private final Socket socket;
            private final BufferedInputStream in;
            private final OutputStream out;
            private final AtomicBoolean alive = new AtomicBoolean(true);

            ClientHandler(Socket socket, ChatServer server) throws IOException {
                this.socket = socket;
                this.server = server;
                this.in = new BufferedInputStream(socket.getInputStream());
                this.out = new BufferedOutputStream(socket.getOutputStream());
                send(("[Server]: Welcome Client-" + id + "!\n").getBytes(StandardCharsets.UTF_8));
                flushQuietly();
            }

            @Override
            public void run() {
                try {
                    readLoop();
                } finally {
                    close();
                }
            }

            private void readLoop() {
                byte[] buf = new byte[1];
                ByteArrayOutputStream line = new ByteArrayOutputStream();
                int read;
                long lastRead = System.currentTimeMillis();
                while (alive.get()) {
                    try {
                        read = in.read(buf);
                        if (read == -1) break;
                        lastRead = System.currentTimeMillis();
                        if (buf[0] == (byte) '\n') {
                            String msg = line.toString(StandardCharsets.UTF_8);
                            line.reset();
                            if ("quit".equalsIgnoreCase(msg.trim())) {
                                break;
                            }
                            server.broadcast(msg, this);
                        } else if (buf[0] != (byte) '\r') {
                            if (line.size() < MAX_MESSAGE_LEN) {
                                line.write(buf[0]);
                            } else {
                                // Too long: drop line and notify client then disconnect
                                send("[Server]: Message too long. Disconnecting for safety.\n".getBytes(StandardCharsets.UTF_8));
                                flushQuietly();
                                break;
                            }
                        }
                        // Idle check (optional)
                        if (System.currentTimeMillis() - lastRead > 2L * SOCKET_READ_TIMEOUT_MS) {
                            break;
                        }
                    } catch (SocketTimeoutException ste) {
                        // Read timed out; continue to allow shutdown
                    } catch (IOException e) {
                        break;
                    }
                }
            }

            void send(byte[] data) {
                synchronized (out) {
                    try {
                        out.write(data);
                        out.flush();
                    } catch (IOException ignored) {
                        alive.set(false);
                    }
                }
            }

            private void flushQuietly() {
                try { out.flush(); } catch (IOException ignored) {}
            }

            void close() {
                if (!alive.getAndSet(false)) return;
                server.removeClient(this);
                try { in.close(); } catch (IOException ignored) {}
                try { out.close(); } catch (IOException ignored) {}
                safeClose(socket);
            }
        }
    }

    // Function to start server with parameters; returns actual port
    public static int start_server(int port, int maxClients, ChatServer[] outServerHolder) throws IOException {
        ChatServer server = new ChatServer(maxClients);
        int actualPort = server.start(port);
        if (outServerHolder != null && outServerHolder.length > 0) {
            outServerHolder[0] = server;
        }
        return actualPort;
    }

    public static void stop_server(ChatServer server) {
        if (server != null) server.stop();
    }

    // Simple client for testing
    private static void clientSendAndReceive(String host, int port, String message, int expectReads, int timeoutMs) {
        try (Socket s = new Socket()) {
            s.connect(new InetSocketAddress(host, port), 2000);
            s.setSoTimeout(timeoutMs);
            OutputStream out = s.getOutputStream();
            InputStream in = s.getInputStream();
            out.write((message + "\n").getBytes(StandardCharsets.UTF_8));
            out.flush();
            // Read some responses (best-effort)
            BufferedReader br = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8));
            for (int i = 0; i < expectReads; i++) {
                String line = br.readLine();
                if (line == null) break;
                // print to verify behavior
                // System.out.println("Client got: " + line);
            }
            out.write(("quit\n").getBytes(StandardCharsets.UTF_8));
            out.flush();
        } catch (IOException ignored) {}
    }

    // Main with 5 test cases: start server, spawn 5 clients sending messages
    public static void main(String[] args) throws Exception {
        ChatServer[] holder = new ChatServer[1];
        int port = start_server(0, 16, holder);
        ChatServer server = holder[0];

        // 5 clients
        Thread[] clients = new Thread[5];
        for (int i = 0; i < 5; i++) {
            final int idx = i;
            clients[i] = new Thread(() ->
                clientSendAndReceive("127.0.0.1", port, "Hello from Java client " + idx, 3, 3000),
                "TestClient-" + i
            );
            clients[i].start();
        }
        for (Thread t : clients) t.join();

        stop_server(server);
        // System.out.println("Java server test completed.");
    }
}