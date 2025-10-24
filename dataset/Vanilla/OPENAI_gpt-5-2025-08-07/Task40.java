import java.util.*;
import java.security.SecureRandom;

public class Task40 {
    // Simple in-memory session and user settings store with CSRF protection
    static class Session {
        String sid;
        String csrf;
        String username = "default_user";
        String email = "user@example.com";
    }

    static class Server {
        private final Map<String, Session> sessions = new HashMap<>();
        private final SecureRandom random = new SecureRandom();

        private String toHex(byte[] bytes) {
            StringBuilder sb = new StringBuilder(bytes.length * 2);
            for (byte b : bytes) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        }

        private String newToken(int sizeBytes) {
            byte[] b = new byte[sizeBytes];
            random.nextBytes(b);
            return toHex(b);
        }

        private String parseSidFromCookie(String cookie) {
            if (cookie == null) return null;
            String[] parts = cookie.split(";");
            for (String part : parts) {
                String p = part.trim();
                if (p.startsWith("SID=")) {
                    return p.substring(4).trim();
                }
            }
            return null;
        }

        private Session getOrCreateSession(String sid) {
            if (sid != null && sessions.containsKey(sid)) {
                return sessions.get(sid);
            }
            Session s = new Session();
            s.sid = newToken(16);
            sessions.put(s.sid, s);
            return s;
        }

        public String handle(String method, String path, Map<String,String> headers, Map<String,String> form) {
            StringBuilder resp = new StringBuilder();
            try {
                String cookie = headers != null ? headers.get("Cookie") : null;
                String sid = parseSidFromCookie(cookie);
                Session session = getOrCreateSession(sid);

                if (!"/settings".equals(path)) {
                    resp.append("HTTP/1.1 404 Not Found\n");
                    resp.append("Content-Type: text/plain; charset=utf-8\n\n");
                    resp.append("Not Found");
                    return resp.toString();
                }

                if ("GET".equalsIgnoreCase(method)) {
                    session.csrf = newToken(16);
                    String body = ""
                        + "<!doctype html><html><head><title>Settings</title></head><body>"
                        + "<h1>User Settings</h1>"
                        + "<form method=\"POST\" action=\"/settings\">"
                        + "<input type=\"hidden\" name=\"csrf_token\" value=\"" + session.csrf + "\"/>"
                        + "<label>Username: <input type=\"text\" name=\"username\" value=\"" + session.username + "\"/></label><br/>"
                        + "<label>Email: <input type=\"email\" name=\"email\" value=\"" + session.email + "\"/></label><br/>"
                        + "<button type=\"submit\">Update</button>"
                        + "</form>"
                        + "</body></html>";

                    resp.append("HTTP/1.1 200 OK\n");
                    resp.append("Set-Cookie: SID=").append(session.sid).append("; HttpOnly; SameSite=Strict\n");
                    resp.append("Content-Type: text/html; charset=utf-8\n\n");
                    resp.append(body);
                    return resp.toString();
                } else if ("POST".equalsIgnoreCase(method)) {
                    String token = form != null ? form.getOrDefault("csrf_token", "") : "";
                    if (token != null && token.equals(session.csrf)) {
                        if (form != null) {
                            session.username = form.getOrDefault("username", session.username);
                            session.email = form.getOrDefault("email", session.email);
                        }
                        // Rotate CSRF token after successful POST
                        session.csrf = newToken(16);
                        String body = "Settings updated successfully for " + session.username + " (" + session.email + ")";
                        resp.append("HTTP/1.1 200 OK\n");
                        resp.append("Set-Cookie: SID=").append(session.sid).append("; HttpOnly; SameSite=Strict\n");
                        resp.append("Content-Type: text/plain; charset=utf-8\n\n");
                        resp.append(body);
                        return resp.toString();
                    } else {
                        resp.append("HTTP/1.1 403 Forbidden\n");
                        resp.append("Set-Cookie: SID=").append(session.sid).append("; HttpOnly; SameSite=Strict\n");
                        resp.append("Content-Type: text/plain; charset=utf-8\n\n");
                        resp.append("Forbidden: invalid CSRF token");
                        return resp.toString();
                    }
                } else {
                    resp.append("HTTP/1.1 405 Method Not Allowed\n");
                    resp.append("Allow: GET, POST\n");
                    resp.append("Content-Type: text/plain; charset=utf-8\n\n");
                    resp.append("Method Not Allowed");
                    return resp.toString();
                }
            } catch (Exception e) {
                resp.setLength(0);
                resp.append("HTTP/1.1 500 Internal Server Error\n");
                resp.append("Content-Type: text/plain; charset=utf-8\n\n");
                resp.append("Internal Server Error");
                return resp.toString();
            }
        }
    }

    private static final Server server = new Server();

    // Public API: accepts inputs as parameters and returns response as string
    public static String processRequest(String method, String path, Map<String,String> headers, Map<String,String> form) {
        return server.handle(method, path, headers, form);
    }

    // Helpers for tests
    private static String extractSetCookieSid(String response) {
        String marker = "Set-Cookie: SID=";
        int idx = response.indexOf(marker);
        if (idx < 0) return null;
        int start = idx + marker.length();
        int end = response.indexOf("\n", start);
        String line = end >= 0 ? response.substring(start, end) : response.substring(start);
        int semicolon = line.indexOf(';');
        if (semicolon >= 0) line = line.substring(0, semicolon);
        return line.trim();
    }

    private static String extractCsrfToken(String response) {
        String name = "name=\"csrf_token\"";
        int n = response.indexOf(name);
        if (n < 0) return null;
        String valMarker = "value=\"";
        int v = response.indexOf(valMarker, n);
        if (v < 0) return null;
        int start = v + valMarker.length();
        int end = response.indexOf("\"", start);
        if (end < 0) return null;
        return response.substring(start, end);
    }

    public static void main(String[] args) {
        // Test 1: GET form, no cookie (server issues SID and CSRF token)
        Map<String,String> headers1 = new HashMap<>();
        String resp1 = processRequest("GET", "/settings", headers1, null);
        System.out.println("=== Test 1: GET /settings (no cookie) ===");
        System.out.println(resp1);

        String sid = extractSetCookieSid(resp1);
        String csrf1 = extractCsrfToken(resp1);
        Map<String,String> headersWithCookie = new HashMap<>();
        headersWithCookie.put("Cookie", "SID=" + sid);

        // Test 2: POST with correct CSRF token (should succeed)
        Map<String,String> form2 = new HashMap<>();
        form2.put("csrf_token", csrf1);
        form2.put("username", "alice");
        form2.put("email", "alice@example.com");
        String resp2 = processRequest("POST", "/settings", headersWithCookie, form2);
        System.out.println("=== Test 2: POST /settings (valid CSRF) ===");
        System.out.println(resp2);

        // Test 3: POST with missing CSRF token (should fail)
        Map<String,String> form3 = new HashMap<>();
        form3.put("username", "mallory");
        form3.put("email", "mallory@example.com");
        String resp3 = processRequest("POST", "/settings", headersWithCookie, form3);
        System.out.println("=== Test 3: POST /settings (missing CSRF) ===");
        System.out.println(resp3);

        // Test 4: POST with invalid CSRF token (should fail)
        Map<String,String> form4 = new HashMap<>();
        form4.put("csrf_token", "invalidtoken");
        form4.put("username", "bob");
        form4.put("email", "bob@example.com");
        String resp4 = processRequest("POST", "/settings", headersWithCookie, form4);
        System.out.println("=== Test 4: POST /settings (invalid CSRF) ===");
        System.out.println(resp4);

        // Test 5: GET to get a fresh token, then POST with new token (should succeed)
        String resp5a = processRequest("GET", "/settings", headersWithCookie, null);
        String csrfNew = extractCsrfToken(resp5a);
        Map<String,String> form5 = new HashMap<>();
        form5.put("csrf_token", csrfNew);
        form5.put("username", "charlie");
        form5.put("email", "charlie@example.com");
        String resp5 = processRequest("POST", "/settings", headersWithCookie, form5);
        System.out.println("=== Test 5: POST /settings after fresh GET (valid CSRF) ===");
        System.out.println(resp5);
    }
}