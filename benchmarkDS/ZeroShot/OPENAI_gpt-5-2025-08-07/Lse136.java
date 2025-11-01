import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.*;

public class Lse136 {
    private static final String SALT = "u$er-salt::";
    private static final String EXPECTED_USERNAME = "admin";
    private static final String EXPECTED_PASSWORD = "s3cureP@ss";
    private static final int MAX_USERNAME_LEN = 64;
    private static final int MAX_PASSWORD_LEN = 256;

    private static byte[] sha256(String s) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            return md.digest(s.getBytes(StandardCharsets.UTF_8));
        } catch (NoSuchAlgorithmException e) {
            // SHA-256 should always be available; in the unlikely event it's not, fail securely.
            return new byte[0];
        }
    }

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() == 0 || username.length() > MAX_USERNAME_LEN) return false;
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            if (!(c == '_' || c == '-' || c == '.' ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9'))) {
                return false;
            }
        }
        return true;
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> map = new LinkedHashMap<>();
        if (query == null || query.isEmpty()) return map;
        String[] pairs = query.split("&");
        for (String pair : pairs) {
            if (pair.isEmpty()) continue;
            String key;
            String val;
            int idx = pair.indexOf('=');
            if (idx >= 0) {
                key = pair.substring(0, idx);
                val = pair.substring(idx + 1);
            } else {
                key = pair;
                val = "";
            }
            try {
                key = java.net.URLDecoder.decode(key, "UTF-8");
                val = java.net.URLDecoder.decode(val, "UTF-8");
            } catch (Exception e) {
                // Invalid encoding -> ignore pair
                continue;
            }
            if (!map.containsKey(key)) {
                // Enforce limits
                if (key != null && key.length() <= 64 && val != null && val.length() <= 512) {
                    map.put(key, val);
                }
            }
        }
        return map;
    }

    public static String check_mod(String url) {
        if (url == null) return "false";
        int q = url.indexOf('?');
        String path = (q >= 0) ? url.substring(0, q) : url;
        if (!"/check_mod".equals(path)) {
            return "false";
        }
        String query = (q >= 0 && q + 1 < url.length()) ? url.substring(q + 1) : "";
        Map<String, String> params = parseQuery(query);

        String username = params.get("username");
        String password = params.get("password");
        if (username == null || password == null) return "false";

        if (!isValidUsername(username)) return "false";
        if (password.length() == 0 || password.length() > MAX_PASSWORD_LEN) return "false";

        // Constant-time compare username
        byte[] u1 = username.getBytes(StandardCharsets.UTF_8);
        byte[] u2 = EXPECTED_USERNAME.getBytes(StandardCharsets.UTF_8);
        if (!MessageDigest.isEqual(u1, u2)) return "false";

        // Hash password with salt and compare in constant time
        byte[] expectedHash = sha256(SALT + EXPECTED_PASSWORD);
        byte[] providedHash = sha256(SALT + password);
        if (!MessageDigest.isEqual(expectedHash, providedHash)) return "false";

        return "true";
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "/check_mod?username=admin&password=s3cureP%40ss",
            "/check_mod?username=admin&password=wrong",
            "/check_mod?username=Admin&password=s3cureP%40ss",
            "/check_mod?username=admin",
            "/other?username=admin&password=s3cureP%40ss"
        };
        for (String t : tests) {
            System.out.println(check_mod(t));
        }
    }
}