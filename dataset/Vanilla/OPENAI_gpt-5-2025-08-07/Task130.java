import java.util.*;

public class Task130 {

    public static class Response {
        public int status;
        public Map<String, String> headers = new LinkedHashMap<>();
        public String body;

        public Response(int status, Map<String, String> headers, String body) {
            this.status = status;
            this.headers = headers;
            this.body = body;
        }
    }

    private static final Set<String> ALLOWED_ORIGINS = new HashSet<>(Arrays.asList(
            "https://example.com",
            "https://app.local"
    ));
    private static final Set<String> ALLOWED_METHODS = new HashSet<>(Arrays.asList("GET", "POST"));
    private static final String ALLOWED_HEADERS_DEFAULT = "Content-Type, Authorization, X-Requested-With";

    private static boolean isOriginAllowed(String origin) {
        return origin != null && ALLOWED_ORIGINS.contains(origin);
    }

    private static String getHeaderIgnoreCase(Map<String, String> headers, String key) {
        if (headers == null) return null;
        for (Map.Entry<String, String> e : headers.entrySet()) {
            if (e.getKey() != null && e.getKey().equalsIgnoreCase(key)) return e.getValue();
        }
        return null;
    }

    public static Response handleRequest(String method, String origin, String path, Map<String, String> requestHeaders) {
        method = method == null ? "" : method.toUpperCase(Locale.ROOT);
        Map<String, String> respHeaders = new LinkedHashMap<>();

        if ("OPTIONS".equals(method)) {
            if (!isOriginAllowed(origin)) {
                return new Response(403, respHeaders, "Forbidden: Origin not allowed");
            }
            String reqMethod = getHeaderIgnoreCase(requestHeaders, "Access-Control-Request-Method");
            if (reqMethod == null) {
                return new Response(400, respHeaders, "Bad Request: Missing Access-Control-Request-Method");
            }
            reqMethod = reqMethod.toUpperCase(Locale.ROOT);
            if (!ALLOWED_METHODS.contains(reqMethod)) {
                return new Response(405, respHeaders, "Method Not Allowed for CORS preflight");
            }
            String reqHeaders = getHeaderIgnoreCase(requestHeaders, "Access-Control-Request-Headers");
            respHeaders.put("Access-Control-Allow-Origin", origin);
            respHeaders.put("Vary", "Origin");
            respHeaders.put("Access-Control-Allow-Methods", String.join(", ", ALLOWED_METHODS));
            respHeaders.put("Access-Control-Allow-Headers", reqHeaders != null && !reqHeaders.isEmpty() ? reqHeaders : ALLOWED_HEADERS_DEFAULT);
            respHeaders.put("Access-Control-Max-Age", "600");
            return new Response(204, respHeaders, "");
        }

        if (!"/hello".equals(path)) {
            return new Response(404, respHeaders, "Not Found");
        }

        if (!ALLOWED_METHODS.contains(method)) {
            return new Response(405, respHeaders, "Method Not Allowed");
        }

        if (origin != null) {
            if (!isOriginAllowed(origin)) {
                return new Response(403, respHeaders, "Forbidden: Origin not allowed");
            }
            respHeaders.put("Access-Control-Allow-Origin", origin);
            respHeaders.put("Vary", "Origin");
            respHeaders.put("Access-Control-Expose-Headers", "X-RateLimit-Remaining");
        }
        respHeaders.put("Content-Type", "application/json");
        respHeaders.put("X-RateLimit-Remaining", "42");
        String body = "{\"message\":\"Hello from CORS-enabled API\",\"method\":\"" + method + "\"}";
        return new Response(200, respHeaders, body);
    }

    public static void main(String[] args) {
        List<Map<String, String>> tests = new ArrayList<>();

        // Test 1: GET /hello from allowed origin
        tests.add(new HashMap<String, String>() {{
            put("METHOD", "GET");
            put("ORIGIN", "https://example.com");
            put("PATH", "/hello");
        }});

        // Test 2: OPTIONS preflight with allowed origin and headers
        tests.add(new HashMap<String, String>() {{
            put("METHOD", "OPTIONS");
            put("ORIGIN", "https://example.com");
            put("PATH", "/hello");
            put("Access-Control-Request-Method", "POST");
            put("Access-Control-Request-Headers", "Content-Type, X-Test");
        }});

        // Test 3: GET from disallowed origin
        tests.add(new HashMap<String, String>() {{
            put("METHOD", "GET");
            put("ORIGIN", "https://evil.com");
            put("PATH", "/hello");
        }});

        // Test 4: POST to /hello from allowed origin
        tests.add(new HashMap<String, String>() {{
            put("METHOD", "POST");
            put("ORIGIN", "https://app.local");
            put("PATH", "/hello");
        }});

        // Test 5: OPTIONS with disallowed requested method
        tests.add(new HashMap<String, String>() {{
            put("METHOD", "OPTIONS");
            put("ORIGIN", "https://example.com");
            put("PATH", "/hello");
            put("Access-Control-Request-Method", "PUT");
        }});

        int i = 1;
        for (Map<String, String> t : tests) {
            String method = t.getOrDefault("METHOD", "GET");
            String origin = t.get("ORIGIN");
            String path = t.getOrDefault("PATH", "/hello");
            Map<String, String> reqHeaders = new HashMap<>();
            for (Map.Entry<String, String> e : t.entrySet()) {
                String k = e.getKey();
                if (!k.equals("METHOD") && !k.equals("ORIGIN") && !k.equals("PATH")) {
                    reqHeaders.put(k, e.getValue());
                }
            }
            Response r = handleRequest(method, origin, path, reqHeaders);
            System.out.println("=== Java Test " + (i++) + " ===");
            System.out.println("Status: " + r.status);
            for (Map.Entry<String, String> h : r.headers.entrySet()) {
                System.out.println(h.getKey() + ": " + h.getValue());
            }
            System.out.println("Body: " + r.body);
        }
    }
}