import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicBoolean;

public class Task108 {
    // Validation helpers
    private static boolean isValidIdOrKey(String s) {
        if (s == null) return false;
        if (s.length() < 1 || s.length() > 64) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_')) return false;
        }
        return true;
    }

    private static boolean isValidValue(String s) {
        if (s == null) return false;
        if (s.length() > 256) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            // Allow visible ASCII plus common punctuation, deny whitespace besides space
            if (c == ' ') continue;
            if (c < 33 || c > 126) return false;
        }
        return true;
    }

    // Object Store
    private static final class ObjectStore {
        private final ConcurrentHashMap<String, ConcurrentHashMap<String, String>> store = new ConcurrentHashMap<>();

        public String newObject(String id) {
            if (!isValidIdOrKey(id)) return "ERR invalid_id";
            ConcurrentHashMap<String, String> prev = store.putIfAbsent(id, new ConcurrentHashMap<>());
            return (prev == null) ? "OK" : "ERR exists";
        }

        public String putField(String id, String key, String value) {
            if (!isValidIdOrKey(id)) return "ERR invalid_id";
            if (!isValidIdOrKey(key)) return "ERR invalid_key";
            if (!isValidValue(value)) return "ERR invalid_value";
            ConcurrentHashMap<String, String> obj = store.get(id);
            if (obj == null) return "ERR not_found";
            obj.put(key, value);
            return "OK";
        }

        public String getField(String id, String key) {
            if (!isValidIdOrKey(id)) return "ERR invalid_id";
            if (!isValidIdOrKey(key)) return "ERR invalid_key";
            ConcurrentHashMap<String, String> obj = store.get(id);
            if (obj == null) return "ERR not_found";
            String v = obj.get(key);
            if (v == null) return "ERR not_found";
            return "OK " + v;
        }

        public String delField(String id, String key) {
            if (!isValidIdOrKey(id)) return "ERR invalid_id";
            if (!isValidIdOrKey(key)) return "ERR invalid_key";
            ConcurrentHashMap<String, String> obj = store.get(id);
            if (obj == null) return "ERR not_found";
            String prev = obj.remove(key);
            return (prev == null) ? "ERR not_found" : "OK";
        }

        public String keys(String id) {
            if (!isValidIdOrKey(id)) return "ERR invalid_id";
            ConcurrentHashMap<String, String> obj = store.get(id);
            if (obj == null) return "ERR not_found";
            List<String> ks = new ArrayList<>(obj.keySet());
            Collections.sort(ks);
            return "OK " + String.join(",", ks);
        }

        public String drop(String id) {
            if (!isValidIdOrKey(id)) return "ERR invalid_id";
            ConcurrentHashMap<String, String> prev = store.remove(id);
            return (prev == null) ? "ERR not_found" : "OK";
        }
    }

    // Protocol processing
    private static String processCommand(String line, ObjectStore store) {
        if (line == null) return "ERR empty";
        line = line.trim();
        if (line.isEmpty()) return "ERR empty";
        String[] parts = line.split(" ");
        String cmd = parts[0].toUpperCase(Locale.ROOT);
        try {
            switch (cmd) {
                case "PING":
                    return "OK PONG";
                case "NEW":
                    if (parts.length != 2) return "ERR usage";
                    return store.newObject(parts[1]);
                case "PUT":
                    if (parts.length != 4) return "ERR usage";
                    return store.putField(parts[1], parts[2], parts[3]);
                case "GET":
                    if (parts.length != 3) return "ERR usage";
                    return store.getField(parts[1], parts[2]);
                case "DEL":
                    if (parts.length != 3) return "ERR usage";
                    return store.delField(parts[1], parts[2]);
                case "KEYS":
                    if (parts.length != 2) return "ERR usage";
                    return store.keys(parts[1]);
                case "DROP":
                    if (parts.length != 2) return "ERR usage";
                    return store.drop(parts[1]);
                case "QUIT":
                    return "OK BYE";
                default:
                    return "ERR unknown";
            }
        } catch (Exception e) {
            return "ERR error";
        }
    }

    // Server
    private static final class Server implements Closeable {
        private final ObjectStore store = new ObjectStore();
        private final ServerSocket serverSocket;
        private final ExecutorService pool = Executors.newCachedThreadPool(r -> {
            Thread t = new Thread(r, "client-handler");
            t.setDaemon(true);
            return t;
        });
        private final AtomicBoolean running = new AtomicBoolean(true);

        public Server(int port) throws IOException {
            ServerSocket ss = new ServerSocket();
            ss.setReuseAddress(true);
            ss.bind(new InetSocketAddress("127.0.0.1", port));
            this.serverSocket = ss;
        }

        public int getPort() {
            return serverSocket.getLocalPort();
        }

        public void start() {
            Thread t = new Thread(this::acceptLoop, "server-accept");
            t.setDaemon(true);
            t.start();
        }

        private void acceptLoop() {
            while (running.get()) {
                try {
                    Socket client = serverSocket.accept();
                    client.setSoTimeout(10000);
                    pool.execute(() -> handleClient(client));
                } catch (SocketException se) {
                    break; // likely closed
                } catch (IOException e) {
                    // ignore and continue
                }
            }
        }

        private static String readLineLimited(InputStream in, int maxLen) throws IOException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            int b;
            while (baos.size() < maxLen && (b = in.read()) != -1) {
                if (b == '\n') break;
                if (b == '\r') continue;
                baos.write(b);
            }
            if (baos.size() >= maxLen) return null; // too long
            if (baos.size() == 0 && b == -1) return null;
            return baos.toString(StandardCharsets.UTF_8);
        }

        private void handleClient(Socket client) {
            try (Socket c = client;
                 InputStream in = c.getInputStream();
                 OutputStream out = c.getOutputStream()) {
                while (true) {
                    String line = readLineLimited(in, 1024);
                    if (line == null) break;
                    String resp = processCommand(line, store);
                    out.write((resp + "\n").getBytes(StandardCharsets.UTF_8));
                    out.flush();
                    if ("OK BYE".equals(resp)) break;
                }
            } catch (IOException ignored) {
            }
        }

        @Override
        public void close() throws IOException {
            running.set(false);
            serverSocket.close();
            pool.shutdownNow();
        }
    }

    // Simple client helper
    private static List<String> sendCommands(String host, int port, List<String> commands) throws IOException {
        List<String> responses = new ArrayList<>();
        try (Socket s = new Socket()) {
            s.connect(new InetSocketAddress(host, port), 3000);
            s.setSoTimeout(3000);
            try (InputStream in = s.getInputStream();
                 OutputStream out = s.getOutputStream()) {
                for (String cmd : commands) {
                    out.write((cmd + "\n").getBytes(StandardCharsets.UTF_8));
                    out.flush();
                    String resp = readResp(in, 1024);
                    responses.add(resp == null ? "ERR no_response" : resp);
                }
                out.write(("QUIT\n").getBytes(StandardCharsets.UTF_8));
                out.flush();
                readResp(in, 1024);
            }
        }
        return responses;
    }

    private static String readResp(InputStream in, int maxLen) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        int b;
        while (baos.size() < maxLen && (b = in.read()) != -1) {
            if (b == '\n') break;
            if (b == '\r') continue;
            baos.write(b);
        }
        if (baos.size() == 0 && b == -1) return null;
        if (baos.size() >= maxLen) return "ERR too_long";
        return baos.toString(StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        try (Server server = new Server(0)) {
            server.start();
            int port = server.getPort();

            // 5 test cases
            List<String> t1 = sendCommands("127.0.0.1", port, Collections.singletonList("NEW objA"));
            System.out.println("Test1: " + t1.get(0));

            List<String> t2 = sendCommands("127.0.0.1", port, Collections.singletonList("PUT objA key1 v1"));
            System.out.println("Test2: " + t2.get(0));

            List<String> t3 = sendCommands("127.0.0.1", port, Collections.singletonList("GET objA key1"));
            System.out.println("Test3: " + t3.get(0));

            List<String> t4 = sendCommands("127.0.0.1", port, Collections.singletonList("KEYS objA"));
            System.out.println("Test4: " + t4.get(0));

            List<String> t5 = sendCommands("127.0.0.1", port, Arrays.asList("DEL objA key1", "GET objA key1"));
            System.out.println("Test5a: " + t5.get(0));
            System.out.println("Test5b: " + t5.get(1));
        } catch (IOException e) {
            System.out.println("Server error");
        }
    }
}