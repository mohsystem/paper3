import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.time.Instant;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.UUID;

/*
Chain-of-Through process in code generation:

1) Problem understanding:
   - Create a simple API-like handler that accepts user data (name, email, age) and stores it in a "database".
   - We simulate the database as an append-only JSON Lines file (users_db.jsonl) for single-file portability.
   - Input: HTTP-like parameters (method, path, headers, body JSON).
   - Output: HTTP-like response (status code, JSON message).

2) Security requirements:
   - Limit request body size to prevent resource exhaustion.
   - Validate and sanitize inputs (name, email, age).
   - Use only safe operations; no command execution or unsafe deserialization.
   - Use atomic append to file; avoid overwriting.
   - Basic content-type enforcement.

3) Secure coding generation:
   - Implement strict validators.
   - Implement minimal JSON parser for expected keys to avoid dependency risks.
   - Escape JSON output.
   - Handle errors gracefully, return proper status codes.

4) Code review:
   - Review all file operations for append-only usage.
   - Ensure no path traversal; fixed database file name.
   - Ensure validation is strict and failures produce 4xx.
   - Ensure all strings are bounded and sanitized.

5) Secure code output:
   - Final code integrates validations, safe I/O, and consistent responses.
*/

public class Task49 {

    private static final int MAX_BODY_SIZE = 4096;
    private static final String DB_PATH = "users_db.jsonl";

    public static class Response {
        public final int statusCode;
        public final String body;
        public Response(int code, String body) {
            this.statusCode = code;
            this.body = body;
        }
    }

    public static class User {
        public final String name;
        public final String email;
        public final int age;
        public final String id;
        public final long createdAt;

        public User(String name, String email, int age) {
            this.name = name;
            this.email = email;
            this.age = age;
            this.id = UUID.randomUUID().toString();
            this.createdAt = Instant.now().getEpochSecond();
        }
    }

    // Validate name: 1..100 chars, allowed letters, digits, spaces, hyphen, apostrophe, underscore, and dot.
    private static boolean validateName(String name) {
        if (name == null) return false;
        String trimmed = name.trim();
        if (trimmed.length() < 1 || trimmed.length() > 100) return false;
        for (int i = 0; i < trimmed.length(); i++) {
            char c = trimmed.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == ' ' || c == '-' || c == '\'' || c == '_' || c == '.')) {
                return false;
            }
        }
        return true;
    }

    // Basic email validation: simple pattern check without complex regex to avoid ReDoS.
    private static boolean validateEmail(String email) {
        if (email == null) return false;
        if (email.length() < 3 || email.length() > 254) return false;
        int at = email.indexOf('@');
        if (at <= 0 || at != email.lastIndexOf('@')) return false;
        String local = email.substring(0, at);
        String domain = email.substring(at + 1);
        if (local.length() == 0 || domain.length() < 3) return false;
        if (!domain.contains(".")) return false;
        String allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._%+-";
        for (int i = 0; i < local.length(); i++) {
            if (allowed.indexOf(local.charAt(i)) < 0) return false;
        }
        String allowedDomain = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-";
        for (int i = 0; i < domain.length(); i++) {
            if (allowedDomain.indexOf(domain.charAt(i)) < 0) return false;
        }
        if (domain.startsWith("-") || domain.endsWith("-") || domain.contains("..")) return false;
        String[] parts = domain.split("\\.");
        if (parts.length < 2) return false;
        String tld = parts[parts.length - 1];
        if (tld.length() < 2 || tld.length() > 24) return false;
        for (int i = 0; i < tld.length(); i++) {
            char c = tld.charAt(i);
            if (!Character.isLetter(c)) return false;
        }
        return true;
    }

    private static boolean validateAge(int age) {
        return age >= 0 && age <= 150;
    }

    private static String sanitize(String s) {
        if (s == null) return "";
        // Remove CR/LF to avoid log injection; trim excessive spaces
        return s.replace("\r", "").replace("\n", "").trim();
    }

    private static String jsonEscape(String s) {
        StringBuilder sb = new StringBuilder(s.length() + 16);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '\\': sb.append("\\\\"); break;
                case '"': sb.append("\\\""); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x", (int)c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    private static String userToJson(User u) {
        return "{\"id\":\"" + jsonEscape(u.id) + "\"," +
               "\"name\":\"" + jsonEscape(u.name) + "\"," +
               "\"email\":\"" + jsonEscape(u.email) + "\"," +
               "\"age\":" + u.age + "," +
               "\"createdAt\":" + u.createdAt + "}";
    }

    private static String extractJsonString(String json, String key) throws IllegalArgumentException {
        String k = "\"" + key + "\"";
        int idx = indexOfKey(json, k);
        if (idx < 0) throw new IllegalArgumentException("Missing key: " + key);
        int colon = skipWs(json, idx + k.length());
        if (colon >= json.length() || json.charAt(colon) != ':') throw new IllegalArgumentException("Invalid JSON near: " + key);
        int valStart = skipWs(json, colon + 1);
        if (valStart >= json.length() || json.charAt(valStart) != '"') throw new IllegalArgumentException("Expected string for: " + key);
        StringBuilder sb = new StringBuilder();
        int i = valStart + 1;
        boolean esc = false;
        while (i < json.length()) {
            char c = json.charAt(i++);
            if (esc) {
                switch (c) {
                    case '"': sb.append('"'); break;
                    case '\\': sb.append('\\'); break;
                    case '/': sb.append('/'); break;
                    case 'b': sb.append('\b'); break;
                    case 'f': sb.append('\f'); break;
                    case 'n': sb.append('\n'); break;
                    case 'r': sb.append('\r'); break;
                    case 't': sb.append('\t'); break;
                    case 'u':
                        if (i + 4 <= json.length()) {
                            String hex = json.substring(i, i + 4);
                            try {
                                char uc = (char) Integer.parseInt(hex, 16);
                                sb.append(uc);
                                i += 4;
                            } catch (NumberFormatException e) {
                                throw new IllegalArgumentException("Invalid unicode escape");
                            }
                        } else {
                            throw new IllegalArgumentException("Invalid unicode escape length");
                        }
                        break;
                    default:
                        throw new IllegalArgumentException("Invalid escape");
                }
                esc = false;
            } else {
                if (c == '\\') esc = true;
                else if (c == '"') break;
                else sb.append(c);
            }
        }
        return sb.toString();
    }

    private static int extractJsonInt(String json, String key) throws IllegalArgumentException {
        String k = "\"" + key + "\"";
        int idx = indexOfKey(json, k);
        if (idx < 0) throw new IllegalArgumentException("Missing key: " + key);
        int colon = skipWs(json, idx + k.length());
        if (colon >= json.length() || json.charAt(colon) != ':') throw new IllegalArgumentException("Invalid JSON near: " + key);
        int i = skipWs(json, colon + 1);
        int j = i;
        boolean neg = false;
        if (j < json.length() && json.charAt(j) == '-') {
            neg = true;
            j++;
        }
        int start = j;
        while (j < json.length() && Character.isDigit(json.charAt(j))) j++;
        if (start == j) throw new IllegalArgumentException("Expected number for: " + key);
        try {
            int val = Integer.parseInt(json.substring(i, j));
            return val;
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid number for: " + key);
        }
    }

    private static int indexOfKey(String json, String quotedKey) {
        // Find first occurrence of "key" that is not inside a string value
        boolean inStr = false;
        boolean esc = false;
        for (int i = 0; i <= json.length() - quotedKey.length(); i++) {
            char c = json.charAt(i);
            if (inStr) {
                if (esc) {
                    esc = false;
                } else if (c == '\\') {
                    esc = true;
                } else if (c == '"') {
                    inStr = false;
                }
            } else {
                if (c == '"') {
                    // possible beginning of a key
                    if (json.regionMatches(i, quotedKey, 0, quotedKey.length())) {
                        return i + quotedKey.length();
                    } else {
                        inStr = true; // it's a string but not our key
                    }
                }
            }
        }
        return -1;
    }

    private static int skipWs(String s, int idx) {
        int i = idx;
        while (i < s.length()) {
            char c = s.charAt(i);
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') i++;
            else break;
        }
        return i;
    }

    private static User parseUserJson(String body) throws IllegalArgumentException {
        String trimmed = body == null ? "" : body.trim();
        if (trimmed.length() < 2 || trimmed.charAt(0) != '{' || trimmed.charAt(trimmed.length() - 1) != '}') {
            throw new IllegalArgumentException("Invalid JSON");
        }
        String name = sanitize(extractJsonString(trimmed, "name"));
        String email = sanitize(extractJsonString(trimmed, "email"));
        int age = extractJsonInt(trimmed, "age");
        return new User(name, email, age);
    }

    private static boolean dbAppendUser(User u, String dbPath) {
        String line = userToJson(u) + System.lineSeparator();
        try {
            // Ensure file exists; create if not.
            Files.write(Paths.get(dbPath), line.getBytes(StandardCharsets.UTF_8),
                    StandardOpenOption.CREATE, StandardOpenOption.APPEND);
            return true;
        } catch (IOException e) {
            return false;
        }
    }

    public static Response handleRequest(String method, String path, Map<String, String> headers, String body) {
        try {
            if (method == null || path == null) {
                return new Response(400, "{\"error\":\"Bad Request\"}");
            }
            if (!"POST".equals(method.toUpperCase(Locale.ROOT))) {
                return new Response(405, "{\"error\":\"Method Not Allowed\"}");
            }
            if (!"/users".equals(path)) {
                return new Response(404, "{\"error\":\"Not Found\"}");
            }
            if (body == null) body = "";
            if (body.getBytes(StandardCharsets.UTF_8).length > MAX_BODY_SIZE) {
                return new Response(413, "{\"error\":\"Payload Too Large\"}");
            }
            String ct = headers != null ? headers.getOrDefault("Content-Type", headers.getOrDefault("content-type", "")) : "";
            if (ct == null) ct = "";
            if (!ct.toLowerCase(Locale.ROOT).startsWith("application/json")) {
                return new Response(415, "{\"error\":\"Unsupported Media Type\"}");
            }
            User u = parseUserJson(body);
            if (!validateName(u.name)) {
                return new Response(400, "{\"error\":\"Invalid name\"}");
            }
            if (!validateEmail(u.email)) {
                return new Response(400, "{\"error\":\"Invalid email\"}");
            }
            if (!validateAge(u.age)) {
                return new Response(400, "{\"error\":\"Invalid age\"}");
            }
            boolean ok = dbAppendUser(u, DB_PATH);
            if (!ok) {
                return new Response(500, "{\"error\":\"Internal Server Error\"}");
            }
            String resp = "{\"message\":\"User created\",\"id\":\"" + jsonEscape(u.id) + "\"}";
            return new Response(201, resp);
        } catch (IllegalArgumentException ex) {
            return new Response(400, "{\"error\":\"" + jsonEscape(ex.getMessage()) + "\"}");
        } catch (Exception ex) {
            return new Response(500, "{\"error\":\"Internal Server Error\"}");
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        Map<String, String> headers = new HashMap<>();
        headers.put("Content-Type", "application/json");

        String[] bodies = new String[] {
            "{\"name\":\"Alice Smith\",\"email\":\"alice@example.com\",\"age\":30}",
            "{\"name\":\"Bob\",\"email\":\"bob[at]example.com\",\"age\":25}", // invalid email
            "{\"email\":\"charlie@example.com\",\"age\":40}", // missing name
            "{\"name\":\"Diana\",\"email\":\"diana@example.com\",\"age\":-5}", // invalid age
            "{\"name\":\"Edward\",\"email\":\"edward@example.com\",\"age\":28}" // valid
        };

        Response r1 = handleRequest("POST", "/users", headers, bodies[0]);
        System.out.println(r1.statusCode + " " + r1.body);

        Response r2 = handleRequest("POST", "/users", headers, bodies[1]);
        System.out.println(r2.statusCode + " " + r2.body);

        Response r3 = handleRequest("POST", "/users", headers, bodies[2]);
        System.out.println(r3.statusCode + " " + r3.body);

        Response r4 = handleRequest("POST", "/users", headers, bodies[3]);
        System.out.println(r4.statusCode + " " + r4.body);

        // Wrong method test
        Response r5 = handleRequest("GET", "/users", headers, bodies[4]);
        System.out.println(r5.statusCode + " " + r5.body);
    }
}