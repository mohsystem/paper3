import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class Task48 {

    // Simple secure chat server that broadcasts messages to all connected clients.
    public static class ChatServer {
        private final int requestedPort;
        private volatile boolean running = false;
        private ServerSocket serverSocket;
        private Thread acceptThread;
        private final CopyOnWriteArrayList<ClientHandler> clients = new CopyOnWriteArrayList<>();
        private final AtomicInteger clientCounter = new AtomicInteger(0);

        public ChatServer(int port) {
            this.requestedPort = port;
        }

        public synchronized boolean start() {
            if (running) return true;
            try {
                serverSocket = new ServerSocket();
                // Security: reuse address to avoid TIME_WAIT issues, set backlog reasonable
                serverSocket.setReuseAddress(true);
                serverSocket.bind(new InetSocketAddress("127.0.0.1", requestedPort), 50);
                serverSocket.setSoTimeout(500);
                running = true;
            } catch (IOException e) {
                return false;
            }
            acceptThread = new Thread(this::acceptLoop, "ChatServer-AcceptLoop");
            acceptThread.setDaemon(true);
            acceptThread.start();
            return true;
        }

        public int getPort() {
            if (serverSocket == null) return -1;
            return serverSocket.getLocalPort();
        }

        private void acceptLoop() {
            while (running) {
                try {
                    Socket s = serverSocket.accept();
                    configureSocket(s);
                    ClientHandler handler = new ClientHandler(s, "Client-" + clientCounter.incrementAndGet());
                    clients.add(handler);
                    new Thread(handler, "Handler-" + handler.name).start();
                } catch (SocketTimeoutException ste) {
                    // loop to check running flag
                } catch (IOException e) {
                    if (running) {
                        // continue unless stopping
                    }
                }
            }
        }

        private void configureSocket(Socket s) throws SocketException {
            s.setTcpNoDelay(true);
            s.setSoTimeout(500);
            s.setKeepAlive(true);
        }

        private String sanitize(String s) {
            if (s == null) return "";
            // Trim and limit length
            s = s.replaceAll("[\\r\\n]", " ").trim();
            if (s.length() > 1024) {
                s = s.substring(0, 1024);
            }
            return s;
        }

        public void broadcast(String msg, ClientHandler from) {
            String sanitized = sanitize(msg);
            String payload = from != null ? from.name + ": " + sanitized : sanitized;
            for (ClientHandler c : clients) {
                c.send(payload);
            }
        }

        public synchronized void stop() {
            running = false;
            try {
                if (serverSocket != null) serverSocket.close();
            } catch (IOException ignored) {}
            for (ClientHandler ch : clients) {
                ch.close();
            }
            clients.clear();
            if (acceptThread != null) {
                try { acceptThread.join(1000); } catch (InterruptedException ignored) {}
            }
        }

        private class ClientHandler implements Runnable {
            private final Socket socket;
            private final String name;
            private volatile boolean active = true;
            private BufferedReader in;
            private BufferedWriter out;

            ClientHandler(Socket socket, String name) {
                this.socket = socket;
                this.name = name;
                try {
                    this.in = new BufferedReader(new InputStreamReader(socket.getInputStream(), java.nio.charset.StandardCharsets.UTF_8));
                    this.out = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), java.nio.charset.StandardCharsets.UTF_8));
                } catch (IOException e) {
                    active = false;
                    close();
                }
            }

            @Override
            public void run() {
                char[] buf = new char[1024];
                StringBuilder lineBuf = new StringBuilder();
                while (active) {
                    try {
                        int ch;
                        try {
                            ch = in.read();
                        } catch (SocketTimeoutException ste) {
                            continue;
                        }
                        if (ch == -1) {
                            break;
                        }
                        if (ch == '\n') {
                            String line = lineBuf.toString();
                            lineBuf.setLength(0);
                            if (!line.isEmpty()) {
                                broadcast(line, this);
                            }
                        } else if (ch != '\r') {
                            if (lineBuf.length() < 1024) {
                                lineBuf.append((char) ch);
                            }
                        }
                    } catch (IOException e) {
                        break;
                    }
                }
                close();
            }

            void send(String message) {
                if (!active) return;
                try {
                    out.write(message);
                    out.write("\n");
                    out.flush();
                } catch (IOException e) {
                    close();
                }
            }

            void close() {
                active = false;
                try { if (in != null) in.close(); } catch (IOException ignored) {}
                try { if (out != null) out.close(); } catch (IOException ignored) {}
                try { if (socket != null && !socket.isClosed()) socket.close(); } catch (IOException ignored) {}
                clients.remove(this);
            }
        }
    }

    // Simple chat client for tests
    public static class ChatClient implements Closeable {
        private final String name;
        private Socket socket;
        private BufferedReader in;
        private BufferedWriter out;
        private final List<String> received = Collections.synchronizedList(new ArrayList<>());
        private volatile boolean running = false;
        private Thread readerThread;

        public ChatClient(String name) {
            this.name = name;
        }

        public boolean connect(String host, int port) {
            try {
                socket = new Socket();
                socket.setReuseAddress(true);
                socket.connect(new InetSocketAddress(host, port), 1000);
                socket.setSoTimeout(500);
                socket.setTcpNoDelay(true);
                in = new BufferedReader(new InputStreamReader(socket.getInputStream(), java.nio.charset.StandardCharsets.UTF_8));
                out = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), java.nio.charset.StandardCharsets.UTF_8));
                running = true;
                readerThread = new Thread(this::readLoop, "Reader-" + name);
                readerThread.setDaemon(true);
                readerThread.start();
                return true;
            } catch (IOException e) {
                return false;
            }
        }

        private void readLoop() {
            String line;
            while (running) {
                try {
                    line = in.readLine();
                    if (line == null) break;
                    received.add(line);
                } catch (SocketTimeoutException ste) {
                    // continue
                } catch (IOException e) {
                    break;
                }
            }
            running = false;
        }

        public boolean send(String message) {
            if (!running) return false;
            String sanitized = message == null ? "" : message.replaceAll("[\\r\\n]", " ");
            if (sanitized.length() > 1024) sanitized = sanitized.substring(0, 1024);
            try {
                out.write(sanitized);
                out.write("\n");
                out.flush();
                return true;
            } catch (IOException e) {
                return false;
            }
        }

        public List<String> getReceivedSnapshot() {
            synchronized (received) {
                return new ArrayList<>(received);
            }
        }

        @Override
        public void close() {
            running = false;
            try { if (in != null) in.close(); } catch (IOException ignored) {}
            try { if (out != null) out.close(); } catch (IOException ignored) {}
            try { if (socket != null && !socket.isClosed()) socket.close(); } catch (IOException ignored) {}
            if (readerThread != null) {
                try { readerThread.join(1000); } catch (InterruptedException ignored) {}
            }
        }
    }

    private static void sleepQuiet(long ms) {
        try { Thread.sleep(ms); } catch (InterruptedException ignored) {}
    }

    // Test utilities
    private static boolean waitFor(CallableBoolean cond, long timeoutMs, long stepMs) {
        long deadline = System.currentTimeMillis() + timeoutMs;
        while (System.currentTimeMillis() < deadline) {
            if (cond.get()) return true;
            sleepQuiet(stepMs);
        }
        return cond.get();
    }

    interface CallableBoolean { boolean get(); }

    public static void main(String[] args) {
        ChatServer server = new ChatServer(0);
        if (!server.start()) {
            System.out.println("Server failed to start");
            return;
        }
        int port = server.getPort();
        System.out.println("Server listening on " + port);

        // Test 1: Single client receives its own broadcast
        ChatClient c1 = new ChatClient("T1-C1");
        c1.connect("127.0.0.1", port);
        c1.send("Hello World");
        waitFor(() -> c1.getReceivedSnapshot().size() >= 1, 2000, 50);
        System.out.println("Test1 received: " + c1.getReceivedSnapshot());

        // Test 2: Two clients, one sends, the other receives
        ChatClient c2 = new ChatClient("T2-C2");
        c2.connect("127.0.0.1", port);
        c1.send("Ping");
        waitFor(() -> c2.getReceivedSnapshot().stream().anyMatch(s -> s.contains("Ping")), 2000, 50);
        System.out.println("Test2 C2 received: " + c2.getReceivedSnapshot());

        // Test 3: Multiple messages from multiple clients
        c2.send("Pong");
        c1.send("Another");
        waitFor(() -> c1.getReceivedSnapshot().size() >= 3 && c2.getReceivedSnapshot().size() >= 2, 3000, 50);
        System.out.println("Test3 C1 received: " + c1.getReceivedSnapshot());
        System.out.println("Test3 C2 received: " + c2.getReceivedSnapshot());

        // Test 4: Client disconnect, others continue
        c1.close();
        c2.send("AfterC1Left");
        waitFor(() -> c2.getReceivedSnapshot().stream().anyMatch(s -> s.contains("AfterC1Left")), 2000, 50);
        System.out.println("Test4 C2 received after C1 left: " + c2.getReceivedSnapshot());

        // Test 5: Late joiner receives subsequent messages only
        ChatClient c3 = new ChatClient("T5-C3");
        c3.connect("127.0.0.1", port);
        c2.send("WelcomeNew");
        waitFor(() -> c3.getReceivedSnapshot().stream().anyMatch(s -> s.contains("WelcomeNew")), 2000, 50);
        System.out.println("Test5 C3 received: " + c3.getReceivedSnapshot());

        // Cleanup
        c2.close();
        c3.close();
        server.stop();
        System.out.println("Done");
    }
}