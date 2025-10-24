import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.net.ServerSocketFactory;
import java.io.*;
import java.math.BigInteger;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.*;
import java.security.interfaces.RSAPublicKey;
import java.security.spec.*;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.*;
import java.util.regex.Pattern;

public class Task7 {

    // ---------- Constants ----------
    private static final String MAGIC = "ENC1";
    private static final int VERSION = 1;
    private static final int SALT_LEN = 16;
    private static final int IV_LEN = 12;
    private static final int TAG_LEN = 16;
    private static final int PBKDF2_ITER = 210_000;
    private static final int AES_KEY_LEN = 32; // 256-bit
    private static final Pattern USERNAME_RE = Pattern.compile("^[A-Za-z0-9_.-]{3,32}$");

    // ---------- Utilities ----------
    static byte[] randomBytes(int n) {
        byte[] b = new byte[n];
        new SecureRandom().nextBytes(b);
        return b;
    }

    static byte[] pbkdf2(char[] pass, byte[] salt, int iterations, int keyLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(pass, salt, iterations, keyLen * 8);
        SecretKeyFactory f = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return f.generateSecret(spec).getEncoded();
    }

    static byte[] aesGcmEncrypt(byte[] key, byte[] iv, byte[] plaintext) throws Exception {
        Cipher c = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(128, iv);
        SecretKeySpecFixed sk = new SecretKeySpecFixed(key, "AES");
        c.init(Cipher.ENCRYPT_MODE, sk, spec);
        return c.doFinal(plaintext); // ciphertext || tag
    }

    static byte[] aesGcmDecrypt(byte[] key, byte[] iv, byte[] ciphertextAndTag) throws Exception {
        Cipher c = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(128, iv);
        SecretKeySpecFixed sk = new SecretKeySpecFixed(key, "AES");
        c.init(Cipher.DECRYPT_MODE, sk, spec);
        return c.doFinal(ciphertextAndTag);
    }

    static boolean constantTimeEquals(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }

    static String urlEncode(String s) {
        try {
            return URLEncoder.encode(s, StandardCharsets.UTF_8.name());
        } catch (UnsupportedEncodingException e) {
            return "";
        }
    }

    static String urlDecode(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (UnsupportedEncodingException e) {
            return "";
        }
    }

    static Map<String, String> parseKv(String s) {
        Map<String, String> m = new LinkedHashMap<>();
        if (s == null || s.isEmpty()) return m;
        String[] parts = s.split("&");
        for (String p : parts) {
            int eq = p.indexOf('=');
            if (eq > 0) {
                String k = p.substring(0, eq);
                String v = p.substring(eq + 1);
                m.put(k, urlDecode(v));
            }
        }
        return m;
    }

    static String buildKv(Map<String, String> m) {
        StringBuilder sb = new StringBuilder();
        boolean first = true;
        for (Map.Entry<String, String> e : m.entrySet()) {
            if (!first) sb.append('&');
            sb.append(e.getKey()).append('=').append(urlEncode(e.getValue()));
            first = false;
        }
        return sb.toString();
    }

    static void sendFramed(OutputStream out, String s) throws IOException {
        byte[] b = s.getBytes(StandardCharsets.UTF_8);
        DataOutputStream dos = new DataOutputStream(out);
        dos.writeInt(b.length);
        dos.write(b);
        dos.flush();
    }

    static String recvFramed(InputStream in) throws IOException {
        DataInputStream dis = new DataInputStream(in);
        int len;
        try {
            len = dis.readInt();
        } catch (EOFException e) {
            return null;
        }
        if (len < 0 || len > 1_048_576) throw new IOException("invalid frame length");
        byte[] b = new byte[len];
        dis.readFully(b);
        return new String(b, StandardCharsets.UTF_8);
    }

    // ---------- Secure key spec wrapper (immutable) ----------
    static final class SecretKeySpecFixed implements Key, SecretKey {
        private final byte[] key;
        private final String alg;
        SecretKeySpecFixed(byte[] k, String alg) {
            this.key = Arrays.copyOf(k, k.length);
            this.alg = alg;
        }
        public String getAlgorithm() { return alg; }
        public String getFormat() { return "RAW"; }
        public byte[] getEncoded() { return Arrays.copyOf(key, key.length); }
        protected void finalize() throws Throwable { Arrays.fill(key, (byte)0); super.finalize(); }
    }

    // ---------- User DB ----------
    static final class UserRecord {
        final byte[] salt;
        final byte[] hash;
        UserRecord(byte[] salt, byte[] hash) {
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
        }
    }

    static final class UserStore {
        private final Path baseDir;
        private final Path dbFile;
        private final char[] encPass;
        private final Object lock = new Object();

        UserStore(Path baseDir, char[] encPass) throws IOException {
            this.baseDir = secureBaseDir(baseDir);
            this.dbFile = secureResolve(this.baseDir, "users.enc");
            this.encPass = Arrays.copyOf(encPass, encPass.length);
            Files.createDirectories(this.baseDir);
        }

        private static Path secureBaseDir(Path base) throws IOException {
            Path p = base.toAbsolutePath().normalize();
            if (!Files.exists(p)) Files.createDirectories(p);
            if (Files.isSymbolicLink(p)) throw new IOException("base dir is symlink");
            return p;
        }

        private static Path secureResolve(Path base, String name) throws IOException {
            if (!name.matches("^[A-Za-z0-9_.-]+\\.[A-Za-z0-9_.-]+$")) {
                throw new IOException("invalid file name");
            }
            Path p = base.resolve(name).toAbsolutePath().normalize();
            if (!p.startsWith(base)) throw new IOException("path traversal");
            return p;
        }

        private Map<String, UserRecord> readAll() throws Exception {
            synchronized (lock) {
                if (!Files.exists(dbFile)) return new LinkedHashMap<>();
                try (InputStream in = Files.newInputStream(dbFile, StandardOpenOption.READ)) {
                    byte[] all = in.readAllBytes();
                    ByteBuffer bb = ByteBuffer.wrap(all);
                    byte[] magic = new byte[4];
                    bb.get(magic);
                    if (!Arrays.equals(magic, MAGIC.getBytes(StandardCharsets.US_ASCII))) throw new IOException("bad magic");
                    int ver = bb.getInt();
                    if (ver != VERSION) throw new IOException("bad version");
                    byte[] salt = new byte[SALT_LEN];
                    bb.get(salt);
                    byte[] iv = new byte[IV_LEN];
                    bb.get(iv);
                    int remaining = bb.remaining();
                    if (remaining < TAG_LEN) throw new IOException("truncated");
                    byte[] ciphertext = new byte[remaining];
                    bb.get(ciphertext);
                    byte[] key = pbkdf2(encPass, salt, PBKDF2_ITER, AES_KEY_LEN);
                    byte[] plaintext = aesGcmDecrypt(key, iv, ciphertext);
                    Arrays.fill(key, (byte)0);
                    String content = new String(plaintext, StandardCharsets.UTF_8);
                    return parseUsers(content);
                }
            }
        }

        private void writeAll(Map<String, UserRecord> data) throws Exception {
            synchronized (lock) {
                String content = serializeUsers(data);
                byte[] salt = randomBytes(SALT_LEN);
                byte[] iv = randomBytes(IV_LEN);
                byte[] key = pbkdf2(encPass, salt, PBKDF2_ITER, AES_KEY_LEN);
                byte[] ctAndTag = aesGcmEncrypt(key, iv, content.getBytes(StandardCharsets.UTF_8));
                Arrays.fill(key, (byte)0);

                ByteArrayOutputStream bos = new ByteArrayOutputStream();
                bos.write(MAGIC.getBytes(StandardCharsets.US_ASCII));
                bos.write(ByteBuffer.allocate(4).putInt(VERSION).array());
                bos.write(salt);
                bos.write(iv);
                bos.write(ctAndTag);
                byte[] outBytes = bos.toByteArray();

                Path tmp = dbFile.resolveSibling("users.enc.tmp." + UUID.randomUUID());
                try (OutputStream os = Files.newOutputStream(tmp, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
                    os.write(outBytes);
                    os.flush();
                    if (os instanceof FileOutputStream) {
                        ((FileOutputStream) os).getFD().sync();
                    }
                }
                try {
                    Files.move(tmp, dbFile, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
                } finally {
                    try { Files.deleteIfExists(tmp); } catch (Exception ignore) {}
                }
            }
        }

        private static Map<String, UserRecord> parseUsers(String s) throws IOException {
            Map<String, UserRecord> m = new LinkedHashMap<>();
            if (s.isEmpty()) return m;
            String[] lines = s.split("\n");
            for (String line : lines) {
                if (line.isBlank()) continue;
                String[] parts = line.split(":");
                if (parts.length != 3) throw new IOException("bad user line");
                String user = parts[0];
                if (!USERNAME_RE.matcher(user).matches()) throw new IOException("bad username in db");
                byte[] salt = Base64.getDecoder().decode(parts[1]);
                byte[] hash = Base64.getDecoder().decode(parts[2]);
                m.put(user, new UserRecord(salt, hash));
            }
            return m;
        }

        private static String serializeUsers(Map<String, UserRecord> m) {
            StringBuilder sb = new StringBuilder();
            for (Map.Entry<String, UserRecord> e : m.entrySet()) {
                String user = e.getKey();
                UserRecord ur = e.getValue();
                sb.append(user).append(":")
                        .append(Base64.getEncoder().encodeToString(ur.salt)).append(":")
                        .append(Base64.getEncoder().encodeToString(ur.hash)).append("\n");
            }
            return sb.toString();
        }

        boolean addUser(String username, char[] password) throws Exception {
            if (!USERNAME_RE.matcher(username).matches()) return false;
            if (!isStrongPassword(password)) return false;
            Map<String, UserRecord> all = readAll();
            if (all.containsKey(username)) return false;
            byte[] salt = randomBytes(SALT_LEN);
            byte[] hash = pbkdf2(password, salt, PBKDF2_ITER, 32);
            all.put(username, new UserRecord(salt, hash));
            writeAll(all);
            return true;
        }

        boolean verifyLogin(String username, char[] password) throws Exception {
            if (!USERNAME_RE.matcher(username).matches()) return false;
            Map<String, UserRecord> all = readAll();
            UserRecord ur = all.get(username);
            if (ur == null) return false;
            byte[] calc = pbkdf2(password, ur.salt, PBKDF2_ITER, ur.hash.length);
            boolean ok = constantTimeEquals(calc, ur.hash);
            Arrays.fill(calc, (byte)0);
            return ok;
        }

        private boolean isStrongPassword(char[] pw) {
            if (pw == null || pw.length < 12) return false;
            boolean hasU=false, hasL=false, hasD=false, hasS=false;
            for (char c : pw) {
                if (Character.isUpperCase(c)) hasU=true;
                else if (Character.isLowerCase(c)) hasL=true;
                else if (Character.isDigit(c)) hasD=true;
                else hasS=true;
            }
            return hasU && hasL && hasD && hasS;
        }
    }

    // ---------- Server ----------
    static final class Server implements AutoCloseable {
        private final int port;
        private final UserStore store;
        private volatile boolean running = true;
        private final KeyPair rsaKey;
        private final ExecutorService pool = Executors.newCachedThreadPool();
        private ServerSocket serverSocket;

        Server(int port, UserStore store) throws Exception {
            this.port = port;
            this.store = store;
            KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA");
            kpg.initialize(2048, new SecureRandom());
            this.rsaKey = kpg.generateKeyPair();
        }

        void startAsync(int maxClients) {
            pool.submit(() -> {
                int handled = 0;
                try (ServerSocket ss = ServerSocketFactory.getDefault().createServerSocket()) {
                    this.serverSocket = ss;
                    ss.bind(new InetSocketAddress(InetAddress.getByName("127.0.0.1"), port));
                    ss.setReuseAddress(true);
                    while (running && handled < maxClients) {
                        try {
                            Socket s = ss.accept();
                            handled++;
                            pool.submit(() -> handleClient(s));
                        } catch (IOException e) {
                            if (running) e.printStackTrace();
                        }
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                } finally {
                    running = false;
                }
            });
        }

        private void handleClient(Socket s) {
            try (Socket sock = s) {
                sock.setSoTimeout(5000);
                OutputStream out = sock.getOutputStream();
                InputStream in = sock.getInputStream();

                // Send public key info
                Map<String, String> hello = new LinkedHashMap<>();
                hello.put("op", "pubkey");
                hello.put("algo", "rsa-oaep-sha256");
                hello.put("host", "localhost");
                hello.put("pk_der_b64", Base64.getEncoder().encodeToString(rsaKey.getPublic().getEncoded()));
                sendFramed(out, buildKv(hello));

                // Receive login
                String reqStr = recvFramed(in);
                if (reqStr == null) return;
                Map<String, String> req = parseKv(reqStr);
                String op = req.getOrDefault("op", "");
                Map<String, String> resp = new LinkedHashMap<>();
                resp.put("op", "login_result");

                if (!"login".equals(op)) {
                    resp.put("status", "error");
                    resp.put("msg", "unsupported op");
                    sendFramed(out, buildKv(resp));
                    return;
                }
                String username = req.getOrDefault("username", "");
                String pwdEncB64 = req.getOrDefault("pwdenc_b64", "");
                byte[] enc = Base64.getDecoder().decode(pwdEncB64);
                String password;
                try {
                    Cipher c = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
                    OAEPParameterSpec oaep = new OAEPParameterSpec("SHA-256", "MGF1",
                            MGF1ParameterSpec.SHA256, PSource.PSpecified.DEFAULT);
                    c.init(Cipher.DECRYPT_MODE, rsaKey.getPrivate(), oaep);
                    byte[] dec = c.doFinal(enc);
                    password = new String(dec, StandardCharsets.UTF_8);
                } catch (GeneralSecurityException e) {
                    resp.put("status", "error");
                    resp.put("msg", "decryption failed");
                    sendFramed(out, buildKv(resp));
                    return;
                }
                boolean ok;
                try {
                    ok = store.verifyLogin(username, password.toCharArray());
                } catch (Exception ex) {
                    resp.put("status", "error");
                    resp.put("msg", "auth error");
                    sendFramed(out, buildKv(resp));
                    return;
                }
                if (ok) {
                    resp.put("status", "ok");
                    resp.put("msg", "welcome");
                } else {
                    resp.put("status", "error");
                    resp.put("msg", "invalid credentials");
                }
                sendFramed(out, buildKv(resp));
            } catch (IOException e) {
                // swallow
            }
        }

        @Override
        public void close() {
            running = false;
            try { if (serverSocket != null) serverSocket.close(); } catch (IOException ignore) {}
            pool.shutdownNow();
        }
    }

    // ---------- Client ----------
    static final class Client {
        private final String host;
        private final int port;

        Client(String host, int port) {
            this.host = host;
            this.port = port;
        }

        String login(String username, char[] password) throws Exception {
            try (Socket sock = new Socket()) {
                sock.connect(new InetSocketAddress(InetAddress.getByName(host), port), 3000);
                sock.setSoTimeout(5000);
                OutputStream out = sock.getOutputStream();
                InputStream in = sock.getInputStream();

                // Receive server public key
                String helloStr = recvFramed(in);
                if (helloStr == null) return "no hello";
                Map<String, String> hello = parseKv(helloStr);
                if (!"pubkey".equals(hello.get("op"))) return "bad hello";
                if (!"rsa-oaep-sha256".equals(hello.get("algo"))) return "bad algo";
                if (!"localhost".equals(hello.get("host"))) return "bad host";
                byte[] pkDer = Base64.getDecoder().decode(hello.getOrDefault("pk_der_b64", ""));

                KeyFactory kf = KeyFactory.getInstance("RSA");
                X509EncodedKeySpec spec = new X509EncodedKeySpec(pkDer);
                PublicKey pk = kf.generatePublic(spec);
                if (!(pk instanceof RSAPublicKey)) return "not RSA";

                // Encrypt password using OAEP-SHA256
                Cipher c = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
                OAEPParameterSpec oaep = new OAEPParameterSpec("SHA-256", "MGF1",
                        MGF1ParameterSpec.SHA256, PSource.PSpecified.DEFAULT);
                c.init(Cipher.ENCRYPT_MODE, pk, oaep);
                byte[] enc = c.doFinal(new String(password).getBytes(StandardCharsets.UTF_8));

                Map<String, String> req = new LinkedHashMap<>();
                req.put("op", "login");
                req.put("username", username);
                req.put("pwdenc_b64", Base64.getEncoder().encodeToString(enc));
                sendFramed(out, buildKv(req));

                String respStr = recvFramed(in);
                if (respStr == null) return "no resp";
                Map<String, String> resp = parseKv(respStr);
                if (!"login_result".equals(resp.get("op"))) return "bad resp";
                return resp.getOrDefault("status", "error") + ":" + resp.getOrDefault("msg", "");
            }
        }
    }

    // ---------- Test Main ----------
    public static void main(String[] args) throws Exception {
        // Setup
        Path base = Paths.get("data_java");
        String passEnv = System.getenv("USERS_PASSPHRASE");
        char[] encPass = (passEnv != null && passEnv.length() >= 16)
                ? passEnv.toCharArray()
                : Base64.getEncoder().encodeToString(randomBytes(24)).toCharArray(); // ephemeral

        UserStore store = new UserStore(base, encPass);

        // Create two test users with random strong passwords
        String user1 = "alice";
        String user2 = "bob";
        char[] pwd1 = genStrongPassword();
        char[] pwd2 = genStrongPassword();
        store.addUser(user1, pwd1);
        store.addUser(user2, pwd2);

        Server server = new Server(55555, store);
        server.startAsync(5);

        // Give server a moment to bind
        Thread.sleep(200);

        Client client = new Client("127.0.0.1", 55555);

        // 5 Test cases
        // 1) Correct login for alice
        String r1 = client.login(user1, pwd1);
        System.out.println("Test1: " + r1);

        // 2) Wrong password for alice
        String r2 = client.login(user1, "WrongPass123!".toCharArray());
        System.out.println("Test2: " + r2);

        // 3) Non-existent user
        String r3 = client.login("charlie", "SomePass123!".toCharArray());
        System.out.println("Test3: " + r3);

        // 4) Correct login for bob
        String r4 = client.login(user2, pwd2);
        System.out.println("Test4: " + r4);

        // 5) Invalid username attempt with traversal-like string
        String r5 = client.login("../etc/passwd", "AAaa11!!AAaa".toCharArray());
        System.out.println("Test5: " + r5);

        server.close();
        Arrays.fill(pwd1, '\0');
        Arrays.fill(pwd2, '\0');
        Arrays.fill(encPass, '\0');
    }

    private static char[] genStrongPassword() {
        String upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        String lower = "abcdefghijklmnopqrstuvwxyz";
        String digits = "0123456789";
        String special = "!@#$%^&*()-_=+[]{}:,.?/|~";
        SecureRandom rnd = new SecureRandom();
        StringBuilder sb = new StringBuilder();
        sb.append(upper.charAt(rnd.nextInt(upper.length())));
        sb.append(lower.charAt(rnd.nextInt(lower.length())));
        sb.append(digits.charAt(rnd.nextInt(digits.length())));
        sb.append(special.charAt(rnd.nextInt(special.length())));
        String pool = upper + lower + digits + special;
        for (int i = 0; i < 12; i++) {
            sb.append(pool.charAt(rnd.nextInt(pool.length())));
        }
        return sb.toString().toCharArray();
    }
}