import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UncheckedIOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

public class Task90 {
    // Chat server implementation
    public static final class ChatServer {
        private final AtomicBoolean running = new AtomicBoolean(false);
        private final CopyOnWriteArrayList<ClientHandler> clients = new CopyOnWriteArrayList<>();
        private ServerSocket serverSocket;
        private Thread acceptThread;
        private final AtomicInteger clientIdGen = new AtomicInteger(1);

        public synchronized void start(int port) throws IOException {
            if (running.get()) {
                throw new IllegalStateException("Server already running");
            }
            serverSocket = new ServerSocket(port);
            // Set a timeout on accept to allow graceful shutdown checks
            serverSocket.setSoTimeout(1000);
            running.set(true);
            acceptThread = new Thread(this::acceptLoop, "AcceptThread");
            acceptThread.setDaemon(true);
            acceptThread.start();
        }

        public synchronized void stop() {
            running.set(false);
            // Close server socket to break accept
            if (serverSocket != null && !serverSocket.isClosed()) {
                try {
                    serverSocket.close();
                } catch (IOException ignored) { }
            }
            // Close all clients
            for (ClientHandler ch : clients) {
                ch.close();
            }
            clients.clear();
            // Join accept thread
            if (acceptThread != null) {
                try {
                    acceptThread.join(3000);
                } catch (InterruptedException ignored) { }
            }
        }

        public synchronized int getPort() {
            if (serverSocket == null) {
                return -1;
            }
            return serverSocket.getLocalPort();
        }

        private void acceptLoop() {
            while (running.get()) {
                try {
                    Socket s = serverSocket.accept();
                    configureSocket(s);
                    String clientName = "client-" + clientIdGen.getAndIncrement();
                    ClientHandler handler = new ClientHandler(this, s, clientName);
                    clients.add(handler);
                    handler.start();
                    broadcast("[server] " + clientName + " joined");
                } catch (SocketTimeoutException ste) {
                    // periodic timeout to check running flag
                } catch (IOException ioe) {
                    if (running.get()) {
                        // Unexpected error while accepting
                        // Close and continue to avoid tight loop
                        try { Thread.sleep(50); } catch (InterruptedException ignored) {}
                    }
                }
            }
        }

        private void configureSocket(Socket s) throws IOException {
            s.setTcpNoDelay(true);
            s.setSoTimeout(15000); // read timeout
        }

        private static String sanitizeLine(String line, int maxLen) {
            if (line == null) return "";
            if (line.length() > maxLen) {
                line = line.substring(0, maxLen);
            }
            StringBuilder sb = new StringBuilder(line.length());
            for (int i = 0; i < line.length(); i++) {
                char c = line.charAt(i);
                if (c == '\n' || c == '\r') {
                    continue;
                }
                // Allow printable ASCII and common Unicode letters; otherwise replace with '?'
                if ((c >= 32 && c <= 126) || Character.isLetterOrDigit(c) || Character.isSpaceChar(c) || c == '\t') {
                    sb.append(c);
                } else {
                    sb.append('?');
                }
            }
            return sb.toString();
        }

        void onClientMessage(ClientHandler from, String message) {
            String clean = sanitizeLine(message, 1024);
            if (clean.isEmpty()) return;
            broadcast(from.getName() + ": " + clean);
        }

        void onClientClosed(ClientHandler ch) {
            clients.remove(ch);
            broadcast("[server] " + ch.getName() + " left");
        }

        private void broadcast(String msg) {
            String toSend = msg + "\n";
            List<ClientHandler> bad = new ArrayList<>();
            for (ClientHandler ch : clients) {
                try {
                    ch.send(toSend);
                } catch (UncheckedIOException e) {
                    bad.add(ch);
                }
            }
            // Cleanup bad clients
            for (ClientHandler ch : bad) {
                ch.close();
                clients.remove(ch);
            }
        }

        private static final class ClientHandler {
            private final ChatServer server;
            private final Socket socket;
            private final String name;
            private final AtomicBoolean open = new AtomicBoolean(true);
            private Thread thread;

            ClientHandler(ChatServer server, Socket socket, String name) {
                this.server = server;
                this.socket = socket;
                this.name = name;
            }

            String getName() {
                return name;
            }

            void start() {
                thread = new Thread(this::run, "Handler-" + name);
                thread.setDaemon(true);
                thread.start();
            }

            void run() {
                try (BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream(), StandardCharsets.UTF_8))) {
                    String line;
                    while (open.get() && (line = reader.readLine()) != null) {
                        if (line.equalsIgnoreCase("/quit")) {
                            break;
                        }
                        server.onClientMessage(this, line);
                    }
                } catch (SocketTimeoutException ste) {
                    // timeout -> close
                } catch (IOException ignored) {
                } finally {
                    close();
                    server.onClientClosed(this);
                }
            }

            void send(String msg) {
                if (!open.get()) return;
                try {
                    BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.UTF_8));
                    writer.write(msg);
                    writer.flush();
                } catch (IOException ioe) {
                    throw new UncheckedIOException(ioe);
                }
            }

            void close() {
                if (open.compareAndSet(true, false)) {
                    try {
                        socket.close();
                    } catch (IOException ignored) {}
                }
            }
        }
    }

    // Simple test clients to exercise the server
    private static final class TestClient implements Runnable {
        private final int id;
        private final String host;
        private final int port;
        private final CountDownLatch readyLatch;
        private final CountDownLatch startLatch;
        private final int expectedMessages;
        private volatile int received = 0;

        TestClient(int id, String host, int port, CountDownLatch readyLatch, CountDownLatch startLatch, int expectedMessages) {
            this.id = id;
            this.host = host;
            this.port = port;
            this.readyLatch = readyLatch;
            this.startLatch = startLatch;
            this.expectedMessages = expectedMessages;
        }

        public int getReceived() {
            return received;
        }

        @Override
        public void run() {
            try (Socket s = new Socket(host, port)) {
                s.setSoTimeout(15000);
                BufferedReader r = new BufferedReader(new InputStreamReader(s.getInputStream(), StandardCharsets.UTF_8));
                BufferedWriter w = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), StandardCharsets.UTF_8));
                readyLatch.countDown();
                startLatch.await();
                // Send one message
                String msg = "Hello from test client " + id;
                w.write(msg);
                w.write("\n");
                w.flush();
                // Expect expectedMessages broadcasts (including own message and join/leave lines)
                long deadline = System.currentTimeMillis() + 5000;
                List<String> inbox = new ArrayList<>();
                while (System.currentTimeMillis() < deadline && inbox.size() < expectedMessages) {
                    String line;
                    try {
                        line = r.readLine();
                    } catch (SocketTimeoutException ste) {
                        break;
                    }
                    if (line == null) break;
                    inbox.add(line);
                }
                received = inbox.size();
                // Graceful quit
                w.write("/quit\n");
                w.flush();
            } catch (Exception e) {
                // fail closed
            }
        }
    }

    public static void main(String[] args) throws Exception {
        ChatServer server = new ChatServer();
        server.start(0); // bind to ephemeral port
        int port = server.getPort();
        String host = InetAddress.getLoopbackAddress().getHostAddress();

        final int clients = 5;
        CountDownLatch ready = new CountDownLatch(clients);
        CountDownLatch start = new CountDownLatch(1);
        Thread[] threads = new Thread[clients];
        TestClient[] tc = new TestClient[clients];

        // For broadcasts: we expect at least 1 join line per client and 1 message per client.
        // During the window, some join/leave messages may be observed. We'll conservatively expect at least 'clients' messages.
        int expectedMin = clients; // each client should receive all chat messages (one per client)

        for (int i = 0; i < clients; i++) {
            tc[i] = new TestClient(i + 1, host, port, ready, start, expectedMin);
            threads[i] = new Thread(tc[i], "TestClient-" + (i + 1));
            threads[i].start();
        }

        // Wait for all clients to connect
        ready.await();
        // Release them to send messages
        start.countDown();

        // Join clients
        for (int i = 0; i < clients; i++) {
            threads[i].join();
        }

        // Print test results
        for (int i = 0; i < clients; i++) {
            System.out.println("Client " + (i + 1) + " received " + tc[i].getReceived() + " lines");
        }

        server.stop();
        System.out.println("Server stopped.");
    }
}