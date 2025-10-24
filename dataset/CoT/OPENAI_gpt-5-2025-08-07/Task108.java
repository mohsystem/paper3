/* 
Chain-of-Through process:
1) Problem understanding:
   - Build a secure remote object store server that accepts TCP connections from clients.
   - Commands: AUTH <token>, CREATE <key> <urlencoded_value>, READ <key>, UPDATE <key> <urlencoded_value>, DELETE <key>, LIST, QUIT.
   - Keys are [A-Za-z0-9_-]{1,64}; values up to 1024 bytes (URL-encoded), max 1000 objects.
   - Server must require authentication before modifications/reads. 
   - Provide test cases that act as clients connecting to the server.

2) Security requirements:
   - Input validation: key/value length and format checks.
   - Authentication with pre-shared token per connection.
   - Resource limits: max line length; socket timeouts; max object count.
   - Concurrency: thread-safe store; avoid race conditions.
   - Protocol safety: encode outputs; avoid CRLF injection; close on malformed input.
   - Denial of service mitigations: limited line size, timeouts, fixed thread pool.

3) Secure coding generation:
   - Use ConcurrentHashMap and atomic operations.
   - Limit buffer sizes; decode URL safely; handle exceptions.
   - Per-connection state tracks auth; refuse commands before AUTH.
   - Sanitize keys; strictly parse commands; return explicit error codes.

4) Code review (applied during writing):
   - No dynamic code execution or external process calls.
   - Close sockets; use try-with-resources where applicable; handle thread cleanup.
   - Fixed constants for all limits; avoid unbounded memory growth.

5) Secure code output:
   - Final code below mitigates identified risks and includes 5 test cases.
*/

import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.atomic.AtomicInteger;

public class Task108 {
    // Security constants
    private static final String AUTH_TOKEN = "Secr3tToken-Task108";
    private static final int MAX_LINE_LEN = 2048;
    private static final int MAX_KEY_LEN = 64;
    private static final int MAX_VALUE_LEN = 1024; // after URL-decoding
    private static final int MAX_OBJECTS = 1000;
    private static final int CLIENT_READ_TIMEOUT_MS = 15000;
    private static final int SERVER_BACKLOG = 50;
    private static final int THREAD_POOL_SIZE = 16;

    // Simple per-connection session
    private static class Session {
        boolean authed = false;
    }

    // Thread-safe object store
    private static class SecureObjectStore {
        private final ConcurrentHashMap<String,String> map = new ConcurrentHashMap<>();
        private final AtomicInteger count = new AtomicInteger(0);

        public String create(String key, String value) {
            if (count.get() >= MAX_OBJECTS) {
                return "ERR STORE_FULL";
            }
            String prev = map.putIfAbsent(key, value);
            if (prev == null) {
                count.incrementAndGet();
                return "OK CREATED";
            } else {
                return "ERR EXISTS";
            }
        }

        public String read(String key) {
            String v = map.get(key);
            if (v == null) return "ERR NOT_FOUND";
            return "OK " + urlEncode(v);
        }

        public String update(String key, String value) {
            if (!map.containsKey(key)) return "ERR NOT_FOUND";
            map.put(key, value);
            return "OK UPDATED";
        }

        public String delete(String key) {
            String v = map.remove(key);
            if (v == null) return "ERR NOT_FOUND";
            count.decrementAndGet();
            return "OK DELETED";
        }

        public String listKeys() {
            // Copy to avoid concurrent modification snapshot issues
            ArrayList<String> keys = new ArrayList<>(map.keySet());
            Collections.sort(keys);
            String joined = String.join(",", keys);
            return "OK " + urlEncode(joined);
        }
    }

    // Utilities
    private static boolean isValidKey(String key) {
        if (key == null) return false;
        if (key.length() < 1 || key.length() > MAX_KEY_LEN) return false;
        for (int i = 0; i < key.length(); i++) {
            char c = key.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') ||
                         (c >= '0' && c <= '9') ||
                         c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    private static String urlEncode(String s) {
        try {
            // Use UTF-8 and plus for space; safe for protocol
            return java.net.URLEncoder.encode(s, StandardCharsets.UTF_8.name());
        } catch (UnsupportedEncodingException e) {
            return "";
        }
    }

    private static String urlDecode(String s) throws IllegalArgumentException {
        try {
            String out = java.net.URLDecoder.decode(s, StandardCharsets.UTF_8.name());
            if (out.length() > MAX_VALUE_LEN) throw new IllegalArgumentException("Value too long");
            return out;
        } catch (UnsupportedEncodingException e) {
            throw new IllegalArgumentException("Bad encoding");
        }
    }

    // Protocol handling
    private static String processCommand(SecureObjectStore store, Session session, String line) {
        if (line == null) return "ERR EMPTY";
        // Remove trailing \r if present
        if (line.endsWith("\r")) line = line.substring(0, line.length()-1);
        String trimmed = line.trim();
        if (trimmed.isEmpty()) return "ERR EMPTY";

        // Enforce max length again defensively
        if (trimmed.length() > MAX_LINE_LEN) return "ERR TOO_LONG";

        // Split into tokens; for commands with value, we allow 3 parts, with value as rest
        String[] parts = trimmed.split(" ", 3);
        String cmd = parts[0].toUpperCase(Locale.ROOT);

        switch (cmd) {
            case "AUTH": {
                if (parts.length < 2) return "ERR BAD_AUTH";
                String token = parts[1];
                if (AUTH_TOKEN.equals(token)) {
                    session.authed = true;
                    return "OK AUTHED";
                } else {
                    return "ERR BAD_AUTH";
                }
            }
            case "QUIT": {
                return "OK BYE";
            }
            case "LIST": {
                if (!session.authed) return "ERR UNAUTH";
                return store.listKeys();
            }
            case "READ": {
                if (!session.authed) return "ERR UNAUTH";
                if (parts.length < 2) return "ERR BAD_ARGS";
                String key = parts[1];
                if (!isValidKey(key)) return "ERR BAD_KEY";
                return store.read(key);
            }
            case "DELETE": {
                if (!session.authed) return "ERR UNAUTH";
                if (parts.length < 2) return "ERR BAD_ARGS";
                String key = parts[1];
                if (!isValidKey(key)) return "ERR BAD_KEY";
                return store.delete(key);
            }
            case "CREATE":
            case "UPDATE": {
                if (!session.authed) return "ERR UNAUTH";
                if (parts.length < 3) return "ERR BAD_ARGS";
                String key = parts[1];
                if (!isValidKey(key)) return "ERR BAD_KEY";
                String encVal = parts[2];
                String value;
                try {
                    value = urlDecode(encVal);
                } catch (IllegalArgumentException ex) {
                    return "ERR BAD_VALUE";
                }
                if (cmd.equals("CREATE")) {
                    return store.create(key, value);
                } else {
                    return store.update(key, value);
                }
            }
            default:
                return "ERR UNKNOWN_CMD";
        }
    }

    // Networking server
    private static class ClientHandler implements Runnable {
        private final Socket socket;
        private final SecureObjectStore store;

        ClientHandler(Socket socket, SecureObjectStore store) {
            this.socket = socket;
            this.store = store;
        }

        @Override
        public void run() {
            Session session = new Session();
            try (Socket s = this.socket) {
                s.setSoTimeout(CLIENT_READ_TIMEOUT_MS);
                InputStream in = new BufferedInputStream(s.getInputStream());
                OutputStream out = new BufferedOutputStream(s.getOutputStream());
                PrintWriter writer = new PrintWriter(new OutputStreamWriter(out, StandardCharsets.UTF_8), true);
                boolean running = true;
                while (running) {
                    String line = readLineLimited(in, MAX_LINE_LEN);
                    if (line == null) {
                        break; // timeout or closed or too long
                    }
                    String resp = processCommand(store, session, line);
                    // Ensure single-line response without CRLF injection
                    if (resp.indexOf('\n') >= 0 || resp.indexOf('\r') >= 0) {
                        resp = "ERR INTERNAL";
                    }
                    writer.println(resp);
                    writer.flush();
                    if (resp.startsWith("OK BYE")) {
                        break;
                    }
                }
            } catch (IOException ignored) {
                // Connection lost or timeout; just close
            }
        }

        // Read a single line up to maxLen; returns null on EOF, timeout, or length exceeded
        private String readLineLimited(InputStream in, int maxLen) throws IOException {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            int b;
            int read = 0;
            while (true) {
                try {
                    b = in.read();
                } catch (SocketTimeoutException te) {
                    return null;
                }
                if (b == -1) {
                    if (read == 0) return null;
                    break;
                }
                read++;
                if (read > maxLen) {
                    return null;
                }
                if (b == '\n') {
                    break;
                }
                baos.write(b);
            }
            return baos.toString(StandardCharsets.UTF_8);
        }
    }

    public static class Server {
        private final int port;
        private final SecureObjectStore store = new SecureObjectStore();
        private final ExecutorService pool = Executors.newFixedThreadPool(THREAD_POOL_SIZE);
        private volatile boolean running = false;
        private ServerSocket serverSocket;

        public Server(int port) {
            this.port = port;
        }

        public synchronized void start() throws IOException {
            if (running) return;
            serverSocket = new ServerSocket();
            // Bind to localhost only for safety
            serverSocket.bind(new InetSocketAddress("127.0.0.1", port), SERVER_BACKLOG);
            running = true;
            Thread acceptThread = new Thread(this::acceptLoop, "Task108-Acceptor");
            acceptThread.setDaemon(true);
            acceptThread.start();
        }

        private void acceptLoop() {
            while (running) {
                try {
                    Socket client = serverSocket.accept();
                    pool.execute(new ClientHandler(client, store));
                } catch (IOException e) {
                    if (running) {
                        // brief pause to avoid tight loop on error
                        try { Thread.sleep(50); } catch (InterruptedException ignored) {}
                    }
                }
            }
        }

        public synchronized void stop() {
            running = false;
            if (serverSocket != null) {
                try { serverSocket.close(); } catch (IOException ignored) {}
            }
            pool.shutdownNow();
        }
    }

    // Client helper for tests
    public static List<String> clientInteract(String host, int port, List<String> commands) throws IOException {
        List<String> responses = new ArrayList<>();
        try (Socket s = new Socket()) {
            s.connect(new InetSocketAddress(host, port), 3000);
            s.setSoTimeout(5000);
            BufferedReader br = new BufferedReader(new InputStreamReader(s.getInputStream(), StandardCharsets.UTF_8));
            BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), StandardCharsets.UTF_8));
            for (String cmd : commands) {
                if (cmd == null) continue;
                if (!cmd.endsWith("\n")) cmd = cmd + "\n";
                bw.write(cmd);
                bw.flush();
                String resp = br.readLine();
                responses.add(resp);
            }
        }
        return responses;
    }

    // 5 test cases in main
    public static void main(String[] args) throws Exception {
        int port = 19080;
        Server server = new Server(port);
        server.start();

        // Helper to print test results safely
        java.util.function.BiConsumer<String, List<String>> printTest = (name, resps) -> {
            System.out.println("=== " + name + " ===");
            for (String r : resps) System.out.println(r);
        };

        // Test 1: Successful auth, create, read
        List<String> t1 = Arrays.asList(
                "AUTH " + AUTH_TOKEN,
                "CREATE hello " + urlEncode("Hi World"),
                "READ hello",
                "QUIT"
        );
        printTest.accept("Test 1", clientInteract("127.0.0.1", port, t1));

        // Test 2: Attempt without auth
        List<String> t2 = Arrays.asList(
                "CREATE nope value",
                "READ nope",
                "QUIT"
        );
        printTest.accept("Test 2", clientInteract("127.0.0.1", port, t2));

        // Test 3: Update and list
        List<String> t3 = Arrays.asList(
                "AUTH " + AUTH_TOKEN,
                "CREATE k1 " + urlEncode("v1"),
                "UPDATE k1 " + urlEncode("new v1"),
                "LIST",
                "READ k1",
                "QUIT"
        );
        printTest.accept("Test 3", clientInteract("127.0.0.1", port, t3));

        // Test 4: Delete and verify not found
        List<String> t4 = Arrays.asList(
                "AUTH " + AUTH_TOKEN,
                "CREATE toDel " + urlEncode("temp"),
                "DELETE toDel",
                "READ toDel",
                "QUIT"
        );
        printTest.accept("Test 4", clientInteract("127.0.0.1", port, t4));

        // Test 5: Invalid key and value too long
        StringBuilder bigVal = new StringBuilder();
        for (int i = 0; i < MAX_VALUE_LEN + 10; i++) bigVal.append('A');
        List<String> t5 = Arrays.asList(
                "AUTH " + AUTH_TOKEN,
                "CREATE bad!key " + urlEncode("value"),
                "CREATE okkey " + urlEncode(bigVal.toString()),
                "QUIT"
        );
        printTest.accept("Test 5", clientInteract("127.0.0.1", port, t5));

        server.stop();
    }
}