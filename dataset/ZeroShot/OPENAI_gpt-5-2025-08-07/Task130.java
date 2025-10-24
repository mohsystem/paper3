import java.util.*;

public class Task130 {
    public static class Response {
        public int status;
        public Map<String, String> headers = new LinkedHashMap<>();
        public String body;
        public Response(int status, Map<String, String> headers, String body) {
            this.status = status;
            if (headers != null) this.headers.putAll(headers);
            this.body = body;
        }
    }

    private static final Set<String> ALLOWED_ORIGINS = new HashSet<>(Arrays.asList(
            "https://example.com",
            "https://app.example.com"
    ));
    private static final Set<String> ALLOWED_METHODS = new HashSet<>(Arrays.asList(
            "GET", "POST", "OPTIONS"
    ));
    private static final Set<String> ALLOWED_HEADERS = new HashSet<>(Arrays.asList(
            "content-type", "authorization", "x-requested-with"
    ));
    private static final String EXPOSE_HEADERS = "X-Response-Time";
    private static final int MAX_AGE = 600;
    private static final int MAX_HEADER_LIST_LEN = 1024;
    private static final int MAX_HEADERS = 50;

    public static Response handleRequest(String method,
                                         String origin,
                                         String accessControlRequestMethod,
                                         String accessControlRequestHeaders,
                                         String path,
                                         String requestBody) {
        String m = method == null ? "" : method.trim().toUpperCase(Locale.ROOT);
        String o = origin == null ? null : origin.trim();
        boolean hasOrigin = o != null && !o.isEmpty();
        Map<String, String> headers = new LinkedHashMap<>();
        // Vary headers for caches
        if ("OPTIONS".equals(m)) {
            headers.put("Vary", "Origin, Access-Control-Request-Method, Access-Control-Request-Headers");
        } else {
            headers.put("Vary", "Origin");
        }

        // Validate origin
        boolean originAllowed = hasOrigin && ALLOWED_ORIGINS.contains(o);

        // Handle preflight
        if ("OPTIONS".equals(m)) {
            if (!originAllowed) {
                return new Response(403, headers, "");
            }
            String reqMethod = accessControlRequestMethod == null ? "" : accessControlRequestMethod.trim().toUpperCase(Locale.ROOT);
            if (!ALLOWED_METHODS.contains(reqMethod) || "OPTIONS".equals(reqMethod)) {
                return new Response(403, headers, "");
            }
            List<String> requestedHeaders = parseHeaderList(accessControlRequestHeaders);
            if (requestedHeaders == null) {
                return new Response(400, headers, "");
            }
            // Ensure requested headers are subset of allowed
            List<String> allowedEcho = new ArrayList<>();
            for (String h : requestedHeaders) {
                String lower = h.toLowerCase(Locale.ROOT);
                if (!ALLOWED_HEADERS.contains(lower)) {
                    return new Response(403, headers, "");
                }
                // echo normalized original-casing from requested (keep as provided but safe)
                allowedEcho.add(h);
            }
            headers.put("Access-Control-Allow-Origin", o);
            headers.put("Access-Control-Allow-Credentials", "true");
            headers.put("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            if (!allowedEcho.isEmpty()) {
                headers.put("Access-Control-Allow-Headers", String.join(", ", allowedEcho));
            } else {
                headers.put("Access-Control-Allow-Headers", "");
            }
            headers.put("Access-Control-Max-Age", Integer.toString(MAX_AGE));
            return new Response(204, headers, "");
        }

        // Actual request
        if (hasOrigin && !originAllowed) {
            return new Response(403, headers, "{\"error\":\"CORS origin denied\"}");
        }

        // Implement one API endpoint
        if ("/api/data".equals(path)) {
            if ("GET".equals(m)) {
                String body = "{\"message\":\"Hello\",\"ts\":" + System.currentTimeMillis() + "}";
                headers.put("Content-Type", "application/json; charset=utf-8");
                headers.put("X-Response-Time", Long.toString(1)); // placeholder
                headers.put("Cache-Control", "no-store");
                if (originAllowed) {
                    headers.put("Access-Control-Allow-Origin", o);
                    headers.put("Access-Control-Allow-Credentials", "true");
                    headers.put("Access-Control-Expose-Headers", EXPOSE_HEADERS);
                }
                return new Response(200, headers, body);
            } else if ("POST".equals(m)) {
                // For demo, echo securely limited body length
                String safeBody = requestBody == null ? "" : requestBody;
                if (safeBody.length() > 1024) safeBody = safeBody.substring(0, 1024);
                String body = "{\"received\":" + safeJsonString(safeBody) + "}";
                headers.put("Content-Type", "application/json; charset=utf-8");
                headers.put("X-Response-Time", Long.toString(1));
                headers.put("Cache-Control", "no-store");
                if (originAllowed) {
                    headers.put("Access-Control-Allow-Origin", o);
                    headers.put("Access-Control-Allow-Credentials", "true");
                    headers.put("Access-Control-Expose-Headers", EXPOSE_HEADERS);
                }
                return new Response(200, headers, body);
            } else {
                return new Response(405, headers, "");
            }
        } else {
            return new Response(404, headers, "");
        }
    }

    private static List<String> parseHeaderList(String list) {
        if (list == null) return new ArrayList<>();
        String s = list.trim();
        if (s.length() > MAX_HEADER_LIST_LEN) return null;
        String[] parts = s.split(",");
        List<String> out = new ArrayList<>();
        for (String raw : parts) {
            if (out.size() >= MAX_HEADERS) break;
            String t = raw.trim();
            if (t.isEmpty()) continue;
            if (!isToken(t)) return null;
            out.add(t);
        }
        return out;
    }

    private static boolean isToken(String s) {
        if (s.length() == 0 || s.length() > 100) return false;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '-';
            if (!ok) return false;
        }
        return true;
    }

    private static String safeJsonString(String s) {
        StringBuilder sb = new StringBuilder();
        sb.append('"');
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
        sb.append('"');
        return sb.toString();
    }

    public static void main(String[] args) {
        // 1) Preflight allowed
        Response r1 = handleRequest(
                "OPTIONS",
                "https://example.com",
                "GET",
                "Content-Type, X-Requested-With",
                "/api/data",
                null
        );
        printResult("Test1 Preflight OK", r1);

        // 2) Actual GET allowed origin
        Response r2 = handleRequest(
                "GET",
                "https://app.example.com",
                null,
                null,
                "/api/data",
                null
        );
        printResult("Test2 GET with CORS", r2);

        // 3) Actual POST disallowed origin
        Response r3 = handleRequest(
                "POST",
                "https://evil.com",
                null,
                null,
                "/api/data",
                "{\"a\":1}"
        );
        printResult("Test3 POST disallowed origin", r3);

        // 4) Actual GET no Origin (same-origin or curl)
        Response r4 = handleRequest(
                "GET",
                null,
                null,
                null,
                "/api/data",
                null
        );
        printResult("Test4 GET no origin", r4);

        // 5) Preflight bad method
        Response r5 = handleRequest(
                "OPTIONS",
                "https://example.com",
                "DELETE",
                "Authorization",
                "/api/data",
                null
        );
        printResult("Test5 Preflight bad method", r5);
    }

    private static void printResult(String label, Response r) {
        System.out.println("=== " + label + " ===");
        System.out.println("Status: " + r.status);
        for (Map.Entry<String, String> e : r.headers.entrySet()) {
            System.out.println(e.getKey() + ": " + e.getValue());
        }
        if (r.body != null && !r.body.isEmpty()) {
            System.out.println("Body: " + r.body);
        }
    }
}