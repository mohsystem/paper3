import java.nio.charset.StandardCharsets;
import java.util.*;

public class Task130 {

    // Response model
    public static final class Response {
        public final int status;
        public final Map<String, String> headers;
        public final String body;

        public Response(int status, Map<String, String> headers, String body) {
            this.status = status;
            this.headers = Collections.unmodifiableMap(new LinkedHashMap<>(headers));
            this.body = body == null ? "" : body;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("Status: ").append(status).append("\n");
            sb.append("Headers:\n");
            for (Map.Entry<String, String> e : headers.entrySet()) {
                sb.append("  ").append(e.getKey()).append(": ").append(e.getValue()).append("\n");
            }
            sb.append("Body: ").append(body).append("\n");
            return sb.toString();
        }
    }

    // Configuration for CORS and API
    public static final class ApiConfig {
        private final Set<String> allowedOrigins;
        private final Set<String> allowedMethods;
        private final Set<String> allowedHeaders;
        private final int maxAgeSeconds;
        private final boolean allowCredentials;

        public ApiConfig(Set<String> allowedOrigins,
                         Set<String> allowedMethods,
                         Set<String> allowedHeaders,
                         int maxAgeSeconds,
                         boolean allowCredentials) {
            this.allowedOrigins = new HashSet<>(allowedOrigins);
            this.allowedMethods = new HashSet<>();
            for (String m : allowedMethods) this.allowedMethods.add(m.toUpperCase(Locale.ROOT));
            Set<String> lower = new HashSet<>();
            for (String h : allowedHeaders) lower.add(h.toLowerCase(Locale.ROOT));
            this.allowedHeaders = lower;
            this.maxAgeSeconds = Math.max(0, maxAgeSeconds);
            this.allowCredentials = allowCredentials;
        }
    }

    private static final ApiConfig CONFIG = new ApiConfig(
            new HashSet<>(Arrays.asList("https://example.com", "https://app.example.com")),
            new HashSet<>(Arrays.asList("GET", "POST", "OPTIONS")),
            new HashSet<>(Arrays.asList("Content-Type", "X-Requested-With")),
            600,
            false
    );

    // Helper: case-insensitive header retrieval
    public static String getHeader(Map<String, String> headers, String key) {
        if (headers == null || key == null) return null;
        for (Map.Entry<String, String> e : headers.entrySet()) {
            if (e.getKey() != null && e.getKey().equalsIgnoreCase(key)) {
                return e.getValue();
            }
        }
        return null;
    }

    private static boolean isValidToken(String s, int maxLen) {
        if (s == null) return false;
        String t = s.trim();
        if (t.isEmpty() || t.length() > maxLen) return false;
        return true;
    }

    // The core API request processor with CORS
    public static Response processRequest(String method, String path, Map<String, String> headers, String body) {
        Map<String, String> outHeaders = new LinkedHashMap<>();
        String safeMethod = method == null ? "" : method.trim().toUpperCase(Locale.ROOT);
        String safePath = path == null ? "" : path.trim();

        if (!isValidToken(safeMethod, 16) || !isValidToken(safePath, 128) || !safePath.startsWith("/")) {
            return new Response(400, outHeaders, "Bad Request");
        }

        String origin = getHeader(headers, "Origin");
        if (origin != null) {
            origin = origin.trim();
            if (origin.length() > 200) {
                return new Response(400, outHeaders, "Invalid Origin");
            }
        }

        boolean isPreflight = "OPTIONS".equals(safeMethod) && origin != null && getHeader(headers, "Access-Control-Request-Method") != null;

        if (isPreflight) {
            return handlePreflight(headers, origin, outHeaders);
        } else {
            return handleActualRequest(safeMethod, safePath, headers, origin, outHeaders);
        }
    }

    private static Response handlePreflight(Map<String, String> headers, String origin, Map<String, String> outHeaders) {
        if (!CONFIG.allowedOrigins.contains(origin)) {
            return new Response(403, outHeaders, "CORS preflight failed: origin not allowed");
        }

        String reqMethod = getHeader(headers, "Access-Control-Request-Method");
        if (reqMethod == null) {
            return new Response(400, outHeaders, "CORS preflight failed: missing ACRM");
        }
        reqMethod = reqMethod.trim().toUpperCase(Locale.ROOT);
        if (!CONFIG.allowedMethods.contains(reqMethod)) {
            return new Response(403, outHeaders, "CORS preflight failed: method not allowed");
        }

        String acrh = getHeader(headers, "Access-Control-Request-Headers");
        if (acrh != null) {
            String[] parts = acrh.split(",");
            for (String p : parts) {
                String h = p.trim().toLowerCase(Locale.ROOT);
                if (h.isEmpty()) continue;
                if (!CONFIG.allowedHeaders.contains(h)) {
                    return new Response(403, outHeaders, "CORS preflight failed: header not allowed -> " + h);
                }
            }
        }

        setCorsHeaders(outHeaders, origin);
        outHeaders.put("Access-Control-Allow-Methods", String.join(", ", sortStrings(CONFIG.allowedMethods)));
        outHeaders.put("Access-Control-Allow-Headers", String.join(", ", sortStrings(CONFIG.allowedHeaders)));
        outHeaders.put("Access-Control-Max-Age", Integer.toString(CONFIG.maxAgeSeconds));
        return new Response(204, outHeaders, "");
    }

    private static Response handleActualRequest(String method, String path, Map<String, String> headers, String origin, Map<String, String> outHeaders) {
        boolean originProvided = origin != null;

        if (originProvided) {
            if (!CONFIG.allowedOrigins.contains(origin)) {
                return new Response(403, outHeaders, "CORS error: origin not allowed");
            }
            setCorsHeaders(outHeaders, origin);
        }

        if ("GET".equals(method) && "/data".equals(path)) {
            String respBody = "{\"message\":\"ok\",\"ts\":" + System.currentTimeMillis() + "}";
            outHeaders.put("Content-Type", "application/json; charset=UTF-8");
            return new Response(200, outHeaders, respBody);
        } else if (!"/data".equals(path)) {
            return new Response(404, outHeaders, "Not Found");
        } else {
            return new Response(405, outHeaders, "Method Not Allowed");
        }
    }

    private static void setCorsHeaders(Map<String, String> outHeaders, String origin) {
        outHeaders.put("Vary", "Origin");
        outHeaders.put("Access-Control-Allow-Origin", origin);
        if (CONFIG.allowCredentials) {
            outHeaders.put("Access-Control-Allow-Credentials", "true");
        }
        outHeaders.put("Access-Control-Expose-Headers", "Content-Type");
    }

    private static List<String> sortStrings(Collection<String> c) {
        List<String> list = new ArrayList<>(c);
        list.sort(String.CASE_INSENSITIVE_ORDER);
        return list;
    }

    // Helper for tests to create headers map safely
    public static Map<String, String> headersOf(String... kv) {
        Map<String, String> map = new LinkedHashMap<>();
        if (kv == null) return map;
        for (int i = 0; i + 1 < kv.length; i += 2) {
            String k = kv[i] == null ? "" : kv[i];
            String v = kv[i + 1] == null ? "" : kv[i + 1];
            if (k.length() <= 64 && v.length() <= 512) {
                map.put(k, v);
            }
        }
        return map;
    }

    public static void main(String[] args) {
        // Test case 1: Allowed origin, GET /data
        Response r1 = processRequest(
                "GET",
                "/data",
                headersOf("Origin", "https://example.com"),
                null
        );
        System.out.println("Test 1:\n" + r1);

        // Test case 2: Disallowed origin
        Response r2 = processRequest(
                "GET",
                "/data",
                headersOf("Origin", "https://evil.com"),
                null
        );
        System.out.println("Test 2:\n" + r2);

        // Test case 3: Preflight allowed
        Response r3 = processRequest(
                "OPTIONS",
                "/data",
                headersOf(
                        "Origin", "https://app.example.com",
                        "Access-Control-Request-Method", "GET",
                        "Access-Control-Request-Headers", "Content-Type, X-Requested-With"
                ),
                null
        );
        System.out.println("Test 3:\n" + r3);

        // Test case 4: Preflight with disallowed header
        Response r4 = processRequest(
                "OPTIONS",
                "/data",
                headersOf(
                        "Origin", "https://example.com",
                        "Access-Control-Request-Method", "GET",
                        "Access-Control-Request-Headers", "X-Secret"
                ),
                null
        );
        System.out.println("Test 4:\n" + r4);

        // Test case 5: No Origin header, GET /data
        Response r5 = processRequest(
                "GET",
                "/data",
                headersOf("User-Agent", "TestClient/1.0"),
                null
        );
        System.out.println("Test 5:\n" + r5);
    }
}