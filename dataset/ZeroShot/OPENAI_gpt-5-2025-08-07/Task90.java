import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Task90 {

    // Configuration constants
    private static final int DEFAULT_MAX_CLIENTS = 100;
    private static final int MAX_MSG_LEN = 512;
    private static final int CLIENT_SO_TIMEOUT_MS = 60000; // 60s
    private static final int ACCEPT_SO_TIMEOUT_MS = 1000;  // 1s

    public static class ChatServer {
        private final int port;
        private final int maxClients;
        private volatile boolean running = false;
        private ServerSocket serverSocket = null;
        private final List<ClientHandler> clients = new CopyOnWriteArrayList<>();
        private ExecutorService pool = null;
        private Thread acceptThread = null;

        public ChatServer(int port, int maxClients) {
            this.port = port;
            this.maxClients = Math.max(1, maxClients);
        }

        public boolean start() {
            if (!isValidPort(this.port)) {
                return false;
            }
            if (running) {
                return true;
            }
            try {
                serverSocket = new ServerSocket();
                serverSocket.setReuseAddress(true);
                serverSocket.bind(new InetSocketAddress("0.0.0.0", this.port), Math.min(this.maxClients, 50));
                serverSocket.setSoTimeout(ACCEPT_SO_TIMEOUT_MS);
                running = true;
                pool = Executors.newFixedThreadPool(Math.min(this.maxClients + 2, this.maxClients + 10));
                acceptThread = new Thread(this::acceptLoop, "chat-accept-loop");
                acceptThread.setDaemon(true);
                acceptThread.start();
                return true;
            } catch (IOException e) {
                stop();
                return false;
            }
        }

        public void stop() {
            running = false;
            // Close server socket
            if (serverSocket != null) {
                try {
                    serverSocket.close();
                } catch (IOException ignored) {
                } finally {
                    serverSocket = null;
                }
            }
            // Close clients
            for (ClientHandler ch : clients) {
                ch.close();
            }
            clients.clear();
            // Shutdown thread pool
            if (pool != null) {
                pool.shutdownNow();
                pool = null;
            }
            // Join accept thread
            if (acceptThread != null) {
                try {
                    acceptThread.join(2000);
                } catch (InterruptedException ignored) {
                    Thread.currentThread().interrupt();
                } finally {
                    acceptThread = null;
                }
            }
        }

        private void acceptLoop() {
            while (running) {
                try {
                    Socket client = serverSocket.accept();
                    if (client == null) {
                        continue;
                    }
                    if (clients.size() >= maxClients) {
                        safeClose(client);
                        continue;
                    }
                    // Configure client socket
                    try {
                        client.setSoTimeout(CLIENT_SO_TIMEOUT_MS);
                        client.setTcpNoDelay(true);
                        client.setKeepAlive(true);
                    } catch (SocketException ignored) {
                    }

                    ClientHandler handler = new ClientHandler(client, this);
                    clients.add(handler);
                    if (pool != null) {
                        pool.execute(handler);
                    } else {
                        // Fallback single thread
                        new Thread(handler, "client-handler").start();
                    }
                } catch (SocketTimeoutException ignored) {
                    // periodic wake to check running flag
                } catch (IOException e) {
                    if (running) {
                        // continue loop; log suppressed for secure minimal output
                    }
                }
            }
        }

        void removeClient(ClientHandler handler) {
            clients.remove(handler);
        }

        void broadcast(String message, ClientHandler sender) {
            String sanitized = sanitizeMessage(message, MAX_MSG_LEN);
            if (sanitized.isEmpty()) {
                return;
            }
            for (ClientHandler ch : clients) {
                if (ch != sender) {
                    ch.sendLine(sanitized);
                }
            }
        }

        private static void safeClose(Socket s) {
            if (s != null) {
                try {
                    s.close();
                } catch (IOException ignored) {
                }
            }
        }

        public static boolean isValidPort(int port) {
            return port >= 1024 && port <= 65535;
        }

        public static String sanitizeMessage(String input, int maxLen) {
            if (input == null) return "";
            StringBuilder sb = new StringBuilder(Math.min(maxLen, input.length()));
            int count = 0;
            for (int i = 0; i < input.length() && count < maxLen; ) {
                int cp = input.codePointAt(i);
                i += Character.charCount(cp);
                // Allow printable unicode except control characters; allow basic whitespace space and tab
                if (cp == 0x0A || cp == 0x0D) {
                    // normalize CR/LF to single space
                    if (count == 0 || (sb.charAt(sb.length() - 1) != ' ')) {
                        sb.append(' ');
                        count++;
                    }
                } else if (cp == 0x09 || (cp >= 0x20 && !Character.isISOControl(cp))) {
                    sb.appendCodePoint(cp);
                    count++;
                }
                // else drop control characters
            }
            // trim leading/trailing whitespace
            String out = sb.toString().trim();
            if (out.length() > maxLen) {
                out = out.substring(0, maxLen);
            }
            return out;
        }

        private static class ClientHandler implements Runnable {
            private final Socket socket;
            private final ChatServer server;
            private BufferedReader reader = null;
            private BufferedWriter writer = null;
            private volatile boolean open = true;

            ClientHandler(Socket socket, ChatServer server) {
                this.socket = socket;
                this.server = server;
            }

            @Override
            public void run() {
                try (Socket s = socket) {
                    InputStream in = new BufferedInputStream(s.getInputStream());
                    OutputStream out = new BufferedOutputStream(s.getOutputStream());
                    reader = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8));
                    writer = new BufferedWriter(new OutputStreamWriter(out, StandardCharsets.UTF_8));

                    sendLine("Welcome to Secure Chat Server");

                    String line;
                    while (open && (line = reader.readLine()) != null) {
                        if (line.isEmpty()) {
                            continue;
                        }
                        String msg = ChatServer.sanitizeMessage(line, MAX_MSG_LEN);
                        if (msg.isEmpty()) {
                            continue;
                        }
                        server.broadcast(msg, this);
                    }
                } catch (IOException ignored) {
                } finally {
                    close();
                    server.removeClient(this);
                }
            }

            void sendLine(String msg) {
                BufferedWriter w = this.writer;
                if (w == null) return;
                synchronized (w) {
                    try {
                        w.write(msg);
                        w.write('\n');
                        w.flush();
                    } catch (IOException e) {
                        close();
                    }
                }
            }

            void close() {
                open = false;
                try {
                    if (reader != null) reader.close();
                } catch (IOException ignored) {
                } finally {
                    reader = null;
                }
                try {
                    if (writer != null) writer.close();
                } catch (IOException ignored) {
                } finally {
                    writer = null;
                }
                if (!socket.isClosed()) {
                    try {
                        socket.close();
                    } catch (IOException ignored) {
                    }
                }
            }
        }
    }

    // 5 basic test cases for utility functions
    private static void runTests() {
        int passed = 0;
        // Test 1: sanitize removes control characters
        String t1 = ChatServer.sanitizeMessage("Hello\u0000World", MAX_MSG_LEN);
        if ("HelloWorld".equals(t1)) passed++;

        // Test 2: sanitize limits length
        String longStr = "a".repeat(600);
        String t2 = ChatServer.sanitizeMessage(longStr, 128);
        if (t2.length() == 128) passed++;

        // Test 3: valid port
        if (ChatServer.isValidPort(8080)) passed++;

        // Test 4: invalid port (too high)
        if (!ChatServer.isValidPort(70000)) passed++;

        // Test 5: invalid port (privileged)
        if (!ChatServer.isValidPort(1023)) passed++;

        System.out.println("Tests passed: " + passed + "/5");
    }

    // Optional: run server if port is provided as arg
    public static void main(String[] args) {
        runTests();
        if (args.length >= 1) {
            try {
                int port = Integer.parseInt(args[0]);
                ChatServer server = new ChatServer(port, DEFAULT_MAX_CLIENTS);
                if (server.start()) {
                    System.out.println("Secure Chat Server running on port " + port + ". Press Ctrl+C to stop.");
                    // Keep main thread alive
                    try {
                        Thread.sleep(Long.MAX_VALUE);
                    } catch (InterruptedException ignored) {
                        Thread.currentThread().interrupt();
                    } finally {
                        server.stop();
                    }
                } else {
                    System.out.println("Failed to start server. Invalid port or system error.");
                }
            } catch (NumberFormatException e) {
                System.out.println("Invalid port argument.");
            }
        }
    }
}