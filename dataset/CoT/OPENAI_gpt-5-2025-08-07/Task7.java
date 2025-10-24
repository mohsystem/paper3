import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;

public class Task7 {

    // 1) Client: build login request
    public static String buildLoginRequest(String username, String password) {
        if (username == null || password == null) return "";
        if (username.length() > 128 || password.length() > 256) return "";
        String u = escape(username);
        String p = escape(password);
        return "op=login;user=" + u + ";pass=" + p;
    }

    // 2) Server: process request
    public static String processRequest(String userStorePath, String request) {
        if (request == null || request.length() > 4096) return "ERR:invalid request size";
        Map<String, String> kv = parseKV(request);
        String op = kv.get("op");
        if (op == null || !op.equals("login")) return "ERR:unsupported op";
        String user = kv.get("user");
        String pass = kv.get("pass");
        if (user == null || pass == null) return "ERR:missing fields";
        String unescUser = unescape(user);
        String unescPass = unescape(pass);
        if (!isValidUsername(unescUser)) return "ERR:invalid username";
        try {
            boolean ok = authenticate(userStorePath, unescUser, unescPass);
            return ok ? "OK:Welcome " + unescUser : "ERR:authentication failed";
        } catch (IOException e) {
            return "ERR:server io error";
        }
    }

    // 3) Server admin: create or update a user in store with salted+iterative SHA-256
    public static boolean createOrUpdateUser(String userStorePath, String username, String password) {
        if (!isValidUsername(username)) return false;
        try {
            Files.createDirectories(Paths.get(userStorePath).getParent() == null ? Paths.get(".") : Paths.get(userStorePath).getParent());
        } catch (Exception ignored) {}
        int iterations = 10000;
        byte[] salt = secureRandomBytes(16);
        String saltHex = toHex(salt);
        String hash = hashPassword(password, salt, iterations);
        Map<String, String> lines = new LinkedHashMap<>();
        // Load existing
        try {
            if (Files.exists(Paths.get(userStorePath))) {
                List<String> existing = Files.readAllLines(Paths.get(userStorePath), StandardCharsets.UTF_8);
                for (String line : existing) {
                    if (line.trim().isEmpty()) continue;
                    int idx = line.indexOf(':');
                    if (idx <= 0) continue;
                    String u = line.substring(0, idx);
                    lines.put(u, line);
                }
            }
        } catch (IOException ignored) {}
        String record = username + ":" + saltHex + ":" + iterations + ":" + hash;
        lines.put(username, record);
        // Write atomically
        Path tmp = Paths.get(userStorePath + ".tmp");
        try (BufferedWriter bw = Files.newBufferedWriter(tmp, StandardCharsets.UTF_8)) {
            for (String k : lines.keySet()) {
                bw.write(lines.get(k));
                bw.newLine();
            }
        } catch (IOException e) {
            return false;
        }
        try {
            Files.move(tmp, Paths.get(userStorePath), StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
        } catch (IOException e) {
            try { Files.deleteIfExists(tmp); } catch (IOException ignored) {}
            return false;
        }
        return true;
    }

    // 4) Authentication against filesystem
    public static boolean authenticate(String userStorePath, String username, String password) throws IOException {
        Path p = Paths.get(userStorePath);
        if (!Files.exists(p)) return false;
        try (BufferedReader br = Files.newBufferedReader(p, StandardCharsets.UTF_8)) {
            String line;
            while ((line = br.readLine()) != null) {
                if (line.isEmpty()) continue;
                String[] parts = line.split(":", -1);
                if (parts.length != 4) continue;
                String u = parts[0];
                if (!u.equals(username)) continue;
                String saltHex = parts[1];
                String iterStr = parts[2];
                String storedHash = parts[3];
                int iterations;
                try { iterations = Integer.parseInt(iterStr); } catch (Exception e) { return false; }
                if (iterations < 1000 || iterations > 200000) return false;
                byte[] salt = fromHex(saltHex);
                String computed = hashPassword(password, salt, iterations);
                return MessageDigest.isEqual(storedHash.getBytes(StandardCharsets.US_ASCII), computed.getBytes(StandardCharsets.US_ASCII));
            }
        }
        return false;
    }

    // Utilities

    private static boolean isValidUsername(String u) {
        if (u == null || u.isEmpty() || u.length() > 64) return false;
        for (int i = 0; i < u.length(); i++) {
            char c = u.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.')) return false;
        }
        return true;
    }

    private static Map<String, String> parseKV(String s) {
        Map<String, String> map = new HashMap<>();
        StringBuilder key = new StringBuilder();
        StringBuilder val = new StringBuilder();
        boolean inKey = true;
        boolean esc = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (esc) {
                if (inKey) key.append(c); else val.append(c);
                esc = false;
            } else {
                if (c == '\\') {
                    esc = true;
                } else if (c == '=' && inKey) {
                    inKey = false;
                } else if (c == ';') {
                    if (key.length() > 0) {
                        map.put(key.toString(), val.toString());
                    }
                    key.setLength(0);
                    val.setLength(0);
                    inKey = true;
                } else {
                    if (inKey) key.append(c); else val.append(c);
                }
            }
        }
        if (key.length() > 0) {
            map.put(key.toString(), val.toString());
        }
        return map;
    }

    private static String escape(String s) {
        StringBuilder sb = new StringBuilder(s.length() + 8);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '\\' || c == ';' || c == '=') {
                sb.append('\\');
            }
            sb.append(c);
        }
        return sb.toString();
    }

    private static String unescape(String s) {
        StringBuilder sb = new StringBuilder(s.length());
        boolean esc = false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (esc) {
                sb.append(c);
                esc = false;
            } else {
                if (c == '\\') esc = true;
                else sb.append(c);
            }
        }
        return sb.toString();
    }

    private static String hashPassword(String password, byte[] salt, int iterations) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] h = md.digest(password.getBytes(StandardCharsets.UTF_8));
            for (int i = 0; i < iterations; i++) {
                md.reset();
                md.update(h);
                md.update(salt);
                h = md.digest();
            }
            return toHex(h);
        } catch (Exception e) {
            return "";
        }
    }

    private static byte[] secureRandomBytes(int len) {
        byte[] b = new byte[len];
        new SecureRandom().nextBytes(b);
        return b;
    }

    private static String toHex(byte[] bytes) {
        char[] hex = "0123456789abcdef".toCharArray();
        char[] out = new char[bytes.length * 2];
        for (int i = 0; i < bytes.length; i++) {
            int v = bytes[i] & 0xFF;
            out[i * 2] = hex[v >>> 4];
            out[i * 2 + 1] = hex[v & 0x0F];
        }
        return new String(out);
    }

    private static byte[] fromHex(String s) {
        if ((s.length() & 1) == 1) return new byte[0];
        byte[] out = new byte[s.length() / 2];
        for (int i = 0; i < out.length; i++) {
            int hi = Character.digit(s.charAt(2 * i), 16);
            int lo = Character.digit(s.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) return new byte[0];
            out[i] = (byte) ((hi << 4) | lo);
        }
        return out;
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String store = "users_java.db";
        // Prepare user store
        createOrUpdateUser(store, "alice", "Wonder@123");
        createOrUpdateUser(store, "bob", "Builder#456");

        String[] reqs = new String[5];
        reqs[0] = buildLoginRequest("alice", "Wonder@123");
        reqs[1] = buildLoginRequest("alice", "wrongpass");
        reqs[2] = buildLoginRequest("charlie", "Anything1!");
        reqs[3] = buildLoginRequest("bob", "Builder#456");
        reqs[4] = buildLoginRequest("bad;user", "str@nge=pass"); // invalid username

        for (int i = 0; i < reqs.length; i++) {
            String resp = processRequest(store, reqs[i]);
            System.out.println("Case " + (i + 1) + ": " + resp);
        }
    }
}