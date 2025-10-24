import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.Map;

public class Task7 {

    // Client: Build login request safely using percent-encoding
    public static String buildLoginRequest(String username, char[] password) {
        if (username == null || password == null) return "";
        String u = username.trim();
        if (u.isEmpty() || u.length() > 128) return "";
        if (password.length == 0 || password.length > 256) return "";
        try {
            String ue = URLEncoder.encode(u, StandardCharsets.UTF_8.name());
            String pe;
            // Convert char[] to String only for encoding; minimize lifetime.
            pe = URLEncoder.encode(new String(password), StandardCharsets.UTF_8.name());
            // Clear password char[] after use is caller's responsibility; here we return request
            return "u=" + ue + "&p=" + pe;
        } catch (Exception e) {
            return "";
        }
    }

    // Server: Process request and authenticate against filesystem DB
    public static String processRequest(String request, Path userDbPath) {
        if (request == null || request.length() == 0) return "ERR: Malformed request";
        Map<String, String> kv = parseQuery(request);
        String uEnc = kv.get("u");
        String pEnc = kv.get("p");
        if (uEnc == null || pEnc == null) return "ERR: Malformed request";
        try {
            String username = URLDecoder.decode(uEnc, StandardCharsets.UTF_8.name());
            String passwordStr = URLDecoder.decode(pEnc, StandardCharsets.UTF_8.name());
            if (username.isEmpty() || passwordStr.isEmpty()) return "ERR: Missing credentials";
            char[] password = passwordStr.toCharArray();
            try {
                UserRecord rec = lookupUser(userDbPath, username);
                if (rec == null) return "ERR: Invalid credentials";
                byte[] derived = kdf(password, rec.salt, rec.iterations);
                boolean ok = constantTimeEquals(derived, rec.hash);
                Arrays.fill(derived, (byte) 0);
                return ok ? "OK" : "ERR: Invalid credentials";
            } finally {
                Arrays.fill(password, '\0');
            }
        } catch (Exception e) {
            return "ERR: Server error";
        }
    }

    // User record type
    static class UserRecord {
        final String username;
        final int iterations;
        final byte[] salt;
        final byte[] hash;
        UserRecord(String u, int it, byte[] s, byte[] h) {
            this.username = u; this.iterations = it; this.salt = s; this.hash = h;
        }
    }

    // Lookup a user in the DB file "username:iterations:saltHex:hashHex"
    static UserRecord lookupUser(Path dbPath, String username) throws IOException {
        if (!Files.exists(dbPath)) return null;
        try (BufferedReader br = Files.newBufferedReader(dbPath, StandardCharsets.UTF_8)) {
            String line;
            while ((line = br.readLine()) != null) {
                line = line.trim();
                if (line.isEmpty() || line.startsWith("#")) continue;
                String[] parts = line.split(":");
                if (parts.length != 4) continue;
                if (!parts[0].equals(username)) continue;
                int iterations;
                try {
                    iterations = Integer.parseInt(parts[1]);
                } catch (NumberFormatException e) { continue; }
                byte[] salt = hexToBytes(parts[2]);
                byte[] hash = hexToBytes(parts[3]);
                if (salt == null || hash == null) continue;
                return new UserRecord(username, iterations, salt, hash);
            }
        }
        return null;
    }

    // Create user DB file from in-memory map username -> password chars
    public static void createUserDb(Path dbPath, Map<String, char[]> userPasswords, int iterations) {
        SecureRandom rng = new SecureRandom();
        try (BufferedWriter bw = Files.newBufferedWriter(dbPath, StandardCharsets.UTF_8)) {
            for (Map.Entry<String, char[]> e : userPasswords.entrySet()) {
                String user = e.getKey().trim();
                if (user.isEmpty()) continue;
                char[] pwd = e.getValue();
                byte[] salt = new byte[16];
                rng.nextBytes(salt);
                byte[] hash = kdf(pwd, salt, iterations);
                String line = user + ":" + iterations + ":" + bytesToHex(salt) + ":" + bytesToHex(hash);
                bw.write(line);
                bw.newLine();
                Arrays.fill(hash, (byte)0);
            }
        } catch (IOException ex) {
            throw new UncheckedIOException(ex);
        } finally {
            // wipe passwords
            for (char[] v : userPasswords.values()) Arrays.fill(v, '\0');
        }
        try {
            // Set file permissions to owner read/write only (best-effort)
            Files.setPosixFilePermissions(dbPath, java.util.EnumSet.of(
                java.nio.file.attribute.PosixFilePermission.OWNER_READ,
                java.nio.file.attribute.PosixFilePermission.OWNER_WRITE
            ));
        } catch (Exception ignore) {}
    }

    // Iterated salted SHA-256 KDF: H = SHA256(salt || password); repeat hashing iterations-1 times
    static byte[] kdf(char[] password, byte[] salt, int iterations) {
        if (iterations < 1) iterations = 1;
        byte[] pwdBytes = new String(password).getBytes(StandardCharsets.UTF_8);
        try {
            MessageDigest md = sha256();
            md.update(salt);
            md.update(pwdBytes);
            byte[] h = md.digest();
            for (int i = 1; i < iterations; i++) {
                md.reset();
                h = md.digest(h);
            }
            return h;
        } catch (Exception e) {
            return new byte[32];
        } finally {
            Arrays.fill(pwdBytes, (byte)0);
        }
    }

    static MessageDigest sha256() {
        try {
            return MessageDigest.getInstance("SHA-256");
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int r = 0;
        for (int i = 0; i < a.length; i++) {
            r |= (a[i] ^ b[i]);
        }
        return r == 0;
    }

    static String bytesToHex(byte[] data) {
        char[] out = new char[data.length * 2];
        final char[] hex = "0123456789abcdef".toCharArray();
        for (int i = 0; i < data.length; i++) {
            int v = data[i] & 0xFF;
            out[i*2] = hex[v >>> 4];
            out[i*2+1] = hex[v & 0x0F];
        }
        return new String(out);
    }

    static byte[] hexToBytes(String hex) {
        if ((hex.length() & 1) == 1) return null;
        int n = hex.length() / 2;
        byte[] out = new byte[n];
        for (int i = 0; i < n; i++) {
            int hi = Character.digit(hex.charAt(2*i), 16);
            int lo = Character.digit(hex.charAt(2*i+1), 16);
            if (hi < 0 || lo < 0) return null;
            out[i] = (byte)((hi << 4) | lo);
        }
        return out;
    }

    static Map<String,String> parseQuery(String q) {
        Map<String,String> map = new LinkedHashMap<>();
        String[] parts = q.split("&");
        for (String p : parts) {
            int idx = p.indexOf('=');
            if (idx <= 0) continue;
            String k = p.substring(0, idx);
            String v = p.substring(idx+1);
            map.put(k, v);
        }
        return map;
    }

    public static void main(String[] args) {
        try {
            Path db = Files.createTempFile("userdb_java_", ".txt");
            Map<String, char[]> users = new LinkedHashMap<>();
            users.put("alice", "Password123!".toCharArray());
            users.put("bob", "S3cur3Pa$$".toCharArray());
            users.put("charlie", "letmein".toCharArray());
            int iterations = 100000;
            createUserDb(db, users, iterations);

            // 5 test cases
            String[] testsU = {"alice", "alice", "dave", "bob", "bob"};
            String[] testsP = {"Password123!", "wrong", "doesntmatter", "", "S3cur3Pa$$"};
            for (int i = 0; i < 5; i++) {
                String req = buildLoginRequest(testsU[i], testsP[i].toCharArray());
                String resp = processRequest(req, db);
                System.out.println("Test " + (i+1) + " -> " + resp);
            }

            Files.deleteIfExists(db);
        } catch (IOException e) {
            System.out.println("ERR: " + e.getMessage());
        }
    }
}