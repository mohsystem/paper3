import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

public class Task108 {
    // Server implementation
    static class RemoteObjectServer {
        private final String secret;
        private final int port;
        private final AtomicInteger counter = new AtomicInteger(0);
        private final ConcurrentHashMap<String, String> store = new ConcurrentHashMap<>();
        private final AtomicBoolean running = new AtomicBoolean(false);
        private ServerSocket serverSocket;
        private final ExecutorService clientPool = Executors.newCachedThreadPool();

        public RemoteObjectServer(int port, String secret) {
            this.port = port;
            this.secret = secret;
        }

        public boolean start() {
            if (running.get()) return true;
            try {
                serverSocket = new ServerSocket();
                serverSocket.setReuseAddress(true);
                serverSocket.bind(new InetSocketAddress("127.0.0.1", port), 50);
                running.set(true);
            } catch (IOException e) {
                return false;
            }
            Thread acceptThread = new Thread(this::acceptLoop, "AcceptLoop");
            acceptThread.setDaemon(true);
            acceptThread.start();
            return true;
        }

        public void stop() {
            running.set(false);
            try {
                if (serverSocket != null && !serverSocket.isClosed()) {
                    serverSocket.close();
                }
            } catch (IOException ignored) {}
            clientPool.shutdownNow();
        }

        private void acceptLoop() {
            while (running.get()) {
                try {
                    Socket client = serverSocket.accept();
                    clientPool.execute(() -> handleClient(client));
                } catch (IOException e) {
                    if (!running.get()) break;
                }
            }
        }

        private static boolean isKeyValid(String k) {
            if (k == null || k.isEmpty() || k.length() > 32) return false;
            for (int i = 0; i < k.length(); i++) {
                char c = k.charAt(i);
                if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-')) return false;
            }
            return true;
        }

        private static boolean constantTimeEquals(String a, String b) {
            if (a == null || b == null) return false;
            byte[] x = a.getBytes(StandardCharsets.UTF_8);
            byte[] y = b.getBytes(StandardCharsets.UTF_8);
            if (x.length != y.length) {
                // compare lengths to avoid length leak in timing
                int diff = x.length ^ y.length;
                for (int i = 0; i < Math.min(x.length, y.length); i++) diff |= x[i] ^ y[i];
                return false;
            }
            int diff = 0;
            for (int i = 0; i < x.length; i++) diff |= x[i] ^ y[i];
            return diff == 0;
        }

        private static String readLineLimited(BufferedInputStream in, int maxLen, int timeoutMs) throws IOException {
            int read;
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            long deadline = System.currentTimeMillis() + timeoutMs;
            while (true) {
                try {
                    if (in.available() == 0) {
                        if (System.currentTimeMillis() > deadline) throw new SocketTimeoutException("timeout");
                        try { Thread.sleep(5); } catch (InterruptedException ignored) {}
                        continue;
                    }
                    read = in.read();
                } catch (InterruptedIOException e) {
                    throw e;
                }
                if (read == -1) {
                    if (baos.size() == 0) return null;
                    break;
                }
                if (read == '\n') break;
                if (read != '\r') {
                    if (baos.size() >= maxLen) throw new IOException("line too long");
                    baos.write(read);
                }
            }
            return baos.toString(StandardCharsets.UTF_8.name()).trim();
        }

        private void writeLine(OutputStream out, String s) {
            try {
                out.write((s + "\n").getBytes(StandardCharsets.UTF_8));
                out.flush();
            } catch (IOException ignored) {}
        }

        private void handleClient(Socket client) {
            try (Socket c = client) {
                c.setSoTimeout(10000);
                c.setTcpNoDelay(true);
                BufferedInputStream in = new BufferedInputStream(c.getInputStream());
                OutputStream out = c.getOutputStream();
                writeLine(out, "WELCOME");
                String line = readLineLimited(in, 1024, 10000);
                if (line == null || !line.startsWith("TOKEN ")) {
                    writeLine(out, "ERR AUTH");
                    return;
                }
                String sent = line.substring(6).trim();
                if (!constantTimeEquals(sent, secret)) {
                    writeLine(out, "ERR AUTH");
                    return;
                }
                writeLine(out, "OK AUTH");
                while (true) {
                    String cmdLine = readLineLimited(in, 1024, 15000);
                    if (cmdLine == null) break;
                    if (cmdLine.isEmpty()) continue;
                    String[] parts = cmdLine.split(" ");
                    String cmd = parts[0].toUpperCase(Locale.ROOT);
                    switch (cmd) {
                        case "QUIT":
                            writeLine(out, "BYE");
                            return;
                        case "PING":
                            writeLine(out, "PONG");
                            break;
                        case "GETCNT":
                            writeLine(out, "COUNTER " + counter.get());
                            break;
                        case "INCR":
                            if (parts.length != 2) { writeLine(out, "ERR ARG"); break; }
                            try {
                                int n = Integer.parseInt(parts[1]);
                                if (n < 0 || n > 1_000_000) { writeLine(out, "ERR RANGE"); break; }
                                int v = counter.addAndGet(n);
                                writeLine(out, "OK " + v);
                            } catch (NumberFormatException e) { writeLine(out, "ERR ARG"); }
                            break;
                        case "DECR":
                            if (parts.length != 2) { writeLine(out, "ERR ARG"); break; }
                            try {
                                int n = Integer.parseInt(parts[1]);
                                if (n < 0 || n > 1_000_000) { writeLine(out, "ERR RANGE"); break; }
                                while (true) {
                                    int cur = counter.get();
                                    int next = cur - n;
                                    if (next < 0) { writeLine(out, "ERR RANGE"); break; }
                                    if (counter.compareAndSet(cur, next)) { writeLine(out, "OK " + next); break; }
                                }
                            } catch (NumberFormatException e) { writeLine(out, "ERR ARG"); }
                            break;
                        case "RESET":
                            counter.set(0);
                            writeLine(out, "OK 0");
                            break;
                        case "PUT":
                            if (parts.length != 3) { writeLine(out, "ERR ARG"); break; }
                            String key = parts[1];
                            String val = parts[2];
                            if (!isKeyValid(key) || val.length() > 256) { writeLine(out, "ERR ARG"); break; }
                            store.put(key, val);
                            writeLine(out, "OK");
                            break;
                        case "GET":
                            if (parts.length != 2) { writeLine(out, "ERR ARG"); break; }
                            key = parts[1];
                            if (!isKeyValid(key)) { writeLine(out, "ERR ARG"); break; }
                            String v = store.get(key);
                            if (v == null) writeLine(out, "ERR NOT_FOUND");
                            else writeLine(out, "VALUE " + v);
                            break;
                        case "DEL":
                            if (parts.length != 2) { writeLine(out, "ERR ARG"); break; }
                            key = parts[1];
                            if (!isKeyValid(key)) { writeLine(out, "ERR ARG"); break; }
                            store.remove(key);
                            writeLine(out, "OK");
                            break;
                        case "LIST":
                            ArrayList<String> keys = new ArrayList<>(store.keySet());
                            Collections.sort(keys);
                            StringBuilder sb = new StringBuilder();
                            for (int i = 0; i < keys.size(); i++) {
                                if (i > 0) sb.append(",");
                                sb.append(keys.get(i));
                            }
                            writeLine(out, "KEYS " + sb);
                            break;
                        default:
                            writeLine(out, "ERR CMD");
                    }
                }
            } catch (IOException ignored) {
            }
        }
    }

    // Client helper
    public static List<String> clientSession(String host, int port, String token, List<String> commands) {
        List<String> responses = new ArrayList<>();
        try (Socket s = new Socket()) {
            s.connect(new InetSocketAddress(host, port), 3000);
            s.setSoTimeout(5000);
            BufferedReader br = new BufferedReader(new InputStreamReader(s.getInputStream(), StandardCharsets.UTF_8));
            BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), StandardCharsets.UTF_8));
            String welcome = br.readLine();
            responses.add(welcome == null ? "" : welcome);
            bw.write("TOKEN " + token + "\n");
            bw.flush();
            String auth = br.readLine();
            responses.add(auth == null ? "" : auth);
            if (auth == null || !auth.startsWith("OK")) return responses;
            for (String cmd : commands) {
                bw.write(cmd + "\n");
                bw.flush();
                String resp = br.readLine();
                responses.add(resp == null ? "" : resp);
            }
        } catch (IOException e) {
            responses.add("ERR IO");
        }
        return responses;
    }

    public static void main(String[] args) throws Exception {
        int port = 50508;
        String secret = System.getenv().getOrDefault("REMOTE_SECRET", "s3cure-Secret-Token");
        RemoteObjectServer server = new RemoteObjectServer(port, secret);
        if (!server.start()) {
            System.out.println("Failed to start server");
            return;
        }
        Thread.sleep(200); // allow server to start

        // Test case 1: Auth and PING
        List<String> out1 = clientSession("127.0.0.1", port, secret, Arrays.asList("PING", "QUIT"));
        System.out.println("Test1: " + out1);

        // Test case 2: Counter ops
        List<String> out2 = clientSession("127.0.0.1", port, secret, Arrays.asList("GETCNT", "INCR 5", "GETCNT", "QUIT"));
        System.out.println("Test2: " + out2);

        // Test case 3: KV put/get
        List<String> out3 = clientSession("127.0.0.1", port, secret, Arrays.asList("PUT foo bar", "GET foo", "QUIT"));
        System.out.println("Test3: " + out3);

        // Test case 4: delete and missing
        List<String> out4 = clientSession("127.0.0.1", port, secret, Arrays.asList("DEL foo", "GET foo", "QUIT"));
        System.out.println("Test4: " + out4);

        // Test case 5: Invalid token
        List<String> out5 = clientSession("127.0.0.1", port, "wrong-token", Arrays.asList("PING", "QUIT"));
        System.out.println("Test5: " + out5);

        server.stop();
    }
}