import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.time.ZonedDateTime;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public class Lse91 {

    // Configuration
    private static final int PBKDF2_ITERATIONS = 210_000;
    private static final int SALT_LENGTH = 16; // 128-bit salt
    private static final int KEY_LENGTH = 256; // 256-bit derived key
    private static final String DB_MAGIC = "USRDB1";
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_-]{3,32}$");

    // Secure random generator
    private static SecureRandom secureRandom() {
        try {
            return SecureRandom.getInstanceStrong();
        } catch (Exception e) {
            return new SecureRandom();
        }
    }

    // Password policy validation
    private static boolean isStrongPassword(char[] password) {
        if (password == null) return false;
        if (password.length < 12 || password.length > 128) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else if ("!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~".indexOf(c) >= 0) hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) sb.append(String.format("%02x", b));
        return sb.toString();
    }

    private static byte[] pbkdf2(char[] password, byte[] salt) throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, PBKDF2_ITERATIONS, KEY_LENGTH);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    private static List<String> readDb(Path dbPath) throws IOException {
        List<String> lines = new ArrayList<>();
        if (!Files.exists(dbPath)) return lines;
        try (BufferedReader br = Files.newBufferedReader(dbPath, StandardCharsets.UTF_8)) {
            String first = br.readLine();
            if (first == null || !DB_MAGIC.equals(first)) {
                throw new IOException("Invalid database header");
            }
            lines.add(first);
            String line;
            while ((line = br.readLine()) != null) {
                // Bound line length
                if (line.length() > 4096) throw new IOException("Corrupted database line");
                lines.add(line);
            }
        }
        return lines;
    }

    private static void writeDbAtomically(Path dbPath, List<String> lines) throws IOException {
        Path dir = dbPath.toAbsolutePath().getParent();
        if (dir == null) dir = Paths.get(".");
        Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
        FileAttribute<Set<PosixFilePermission>> attr = null;
        try {
            attr = PosixFilePermissions.asFileAttribute(perms);
        } catch (UnsupportedOperationException ignored) {
        }
        Path tmp = Files.createTempFile(dir, "usersdb_", ".tmp", attr);
        try (FileChannel ch = FileChannel.open(tmp, StandardOpenOption.WRITE);
             BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(Channels.newOutputStream(ch), StandardCharsets.UTF_8))) {
            for (String l : lines) {
                bw.write(l);
                bw.newLine();
            }
            bw.flush();
            ch.force(true);
        }
        try {
            Files.setPosixFilePermissions(tmp, perms);
        } catch (Exception ignored) {
        }
        try {
            Files.move(tmp, dbPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
        } catch (AtomicMoveNotSupportedException e) {
            Files.move(tmp, dbPath, StandardCopyOption.REPLACE_EXISTING);
        } finally {
            try {
                Files.deleteIfExists(tmp);
            } catch (Exception ignored) {
            }
        }
    }

    public static String register(String username, char[] password, Path dbPath) {
        Objects.requireNonNull(username, "username");
        Objects.requireNonNull(password, "password");
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            Arrays.fill(password, '\0');
            return "Invalid username format";
        }
        if (!isStrongPassword(password)) {
            Arrays.fill(password, '\0');
            return "Password does not meet strength requirements";
        }

        byte[] salt = new byte[SALT_LENGTH];
        secureRandom().nextBytes(salt);
        byte[] hash = null;
        try {
            hash = pbkdf2(password, salt);
        } catch (GeneralSecurityException e) {
            Arrays.fill(password, '\0');
            return "Internal error";
        } finally {
            Arrays.fill(password, '\0');
        }

        try {
            List<String> lines;
            if (Files.exists(dbPath)) lines = readDb(dbPath);
            else {
                lines = new ArrayList<>();
                lines.add(DB_MAGIC);
            }

            // Check duplicate username
            for (int i = 1; i < lines.size(); i++) {
                String line = lines.get(i);
                String[] parts = line.split("\\|", -1);
                if (parts.length >= 4) {
                    if (username.equals(parts[0])) {
                        return "Username already exists";
                    }
                }
            }

            String record = String.join("|",
                    username,
                    Integer.toString(PBKDF2_ITERATIONS),
                    toHex(salt),
                    toHex(hash));
            lines.add(record);
            writeDbAtomically(dbPath, lines);
            Arrays.fill(hash, (byte) 0);
            return "Registered";
        } catch (IOException e) {
            if (hash != null) Arrays.fill(hash, (byte) 0);
            return "Internal error";
        }
    }

    // Minimal HTTP server for /register (POST, x-www-form-urlencoded)
    public static class RegisterHandler implements HttpHandler {
        private final Path dbPath;

        public RegisterHandler(Path dbPath) {
            this.dbPath = dbPath;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendJson(exchange, 405, "{\"error\":\"Method not allowed\"}");
                    return;
                }
                List<String> cts = exchange.getRequestHeaders().get("Content-Type");
                if (cts == null || cts.stream().noneMatch(v -> v.toLowerCase(Locale.ROOT).startsWith("application/x-www-form-urlencoded"))) {
                    sendJson(exchange, 400, "{\"error\":\"Unsupported content type\"}");
                    return;
                }

                int max = 4096;
                InputStream is = exchange.getRequestBody();
                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                byte[] buf = new byte[1024];
                int total = 0, r;
                while ((r = is.read(buf)) != -1) {
                    total += r;
                    if (total > max) {
                        sendJson(exchange, 413, "{\"error\":\"Request too large\"}");
                        return;
                    }
                    bos.write(buf, 0, r);
                }
                String body = bos.toString(StandardCharsets.UTF_8);
                Map<String, String> form = parseForm(body);
                String username = form.getOrDefault("username", "");
                String passwordStr = form.getOrDefault("password", "");
                String result = register(username, passwordStr.toCharArray(), dbPath);
                if ("Registered".equals(result)) {
                    sendJson(exchange, 200, "{\"status\":\"ok\"}");
                } else if ("Username already exists".equals(result)) {
                    sendJson(exchange, 409, "{\"error\":\"Username already exists\"}");
                } else if ("Invalid username format".equals(result) || "Password does not meet strength requirements".equals(result)) {
                    sendJson(exchange, 400, "{\"error\":\"" + jsonEscape(result) + "\"}");
                } else {
                    sendJson(exchange, 500, "{\"error\":\"Internal error\"}");
                }
            } catch (Exception e) {
                sendJson(exchange, 500, "{\"error\":\"Internal error\"}");
            }
        }

        private static Map<String, String> parseForm(String body) throws UnsupportedEncodingException {
            Map<String, String> map = new HashMap<>();
            String[] pairs = body.split("&");
            for (String p : pairs) {
                if (p.isEmpty()) continue;
                String[] kv = p.split("=", 2);
                String k = URLDecoder.decode(kv[0], StandardCharsets.UTF_8);
                String v = kv.length > 1 ? URLDecoder.decode(kv[1], StandardCharsets.UTF_8) : "";
                map.put(k, v);
            }
            return map;
        }

        private static void sendJson(HttpExchange ex, int status, String json) throws IOException {
            byte[] data = json.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "application/json; charset=utf-8");
            ex.sendResponseHeaders(status, data.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(data);
            }
        }

        private static String jsonEscape(String s) {
            return s.replace("\\", "\\\\").replace("\"", "\\\"");
        }
    }

    public static HttpServer startServer(Path dbPath, int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/register", new RegisterHandler(dbPath));
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
        return server;
    }

    public static void main(String[] args) throws Exception {
        Path dbPath = Paths.get("users_java.db");
        if (!Files.exists(dbPath)) {
            List<String> init = new ArrayList<>();
            init.add(DB_MAGIC);
            writeDbAtomically(dbPath, init);
        }

        // 5 test cases
        String[] usernames = {
                "alice_01",
                "alice_01", // duplicate
                "bob",
                "X",
                "charlie-user"
        };
        String[] passwords = {
                "Str0ng!Password#1",
                "Anoth3r!Str0ngPass",
                "N0tStrong", // weak
                "Weak123!", // invalid username and weak
                "V3ry$trongPassw0rd!"
        };

        for (int i = 0; i < 5; i++) {
            String u = usernames[i];
            String p = passwords[i];
            String res = register(u, p.toCharArray(), dbPath);
            System.out.println(ZonedDateTime.now() + " Test " + (i + 1) + " register(" + u + "): " + res);
        }

        // Start server (commented to avoid blocking CI); uncomment to run a local server
        // HttpServer server = startServer(dbPath, 8080);
        // System.out.println("Server running on http://127.0.0.1:8080/register");
        // Runtime.getRuntime().addShutdownHook(new Thread(server::stop));
    }
}