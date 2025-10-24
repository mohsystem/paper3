import java.util.*;

public class Task130 {

    // Response structure
    public static class Response {
        public final int status;
        public final Map<String, String> headers;
        public final String body;

        public Response(int status, Map<String, String> headers, String body) {
            this.status = status;
            this.headers = Collections.unmodifiableMap(new LinkedHashMap<>(headers));
            this.body = body;
        }
    }

    // Security: strict allowlists
    private static final Set<String> ALLOWED_ORIGINS = new HashSet<>(Arrays.asList(
            "https://trustedorigin.com",
            "http://localhost:3000"
    ));
    private static final Set<String> ALLOWED_METHODS = new HashSet<>(Arrays.asList("GET", "POST"));
    private static final Set<String> ALLOWED_HEADERS = new HashSet<>(Arrays.asList(
            "content-type", "authorization", "x-requested-with"
    ));

    // Utility: safe lower-case trim
    private static String normalizeToken(String s) {
        if (s == null) return "";
        return s.trim().toLowerCase(Locale.ROOT);
    }

    private static boolean isOriginAllowed(String origin) {
        if (origin == null) return false;
        // Exact match only to avoid header reflection
        return ALLOWED_ORIGINS.contains(origin.trim());
    }

    private static Set<String> parseHeaderList(String headerList) {
        Set<String> out = new LinkedHashSet<>();
        if (headerList == null) return out;
        String[] parts = headerList.split(",");
        for (String p : parts) {
            String t = normalizeToken(p);
            if (!t.isEmpty()) out.add(t);
        }
        return out;
    }

    private static String joinCSV(Collection<String> items) {
        StringBuilder sb = new StringBuilder();
        boolean first = true;
        for (String it : items) {
            if (!first) sb.append(", ");
            sb.append(it);
            first = false;
        }
        return sb.toString();
    }

    // Core: handle API with CORS
    public static Response handleRequest(
            String method,
            String path,
            String origin,
            String accessControlRequestMethod,
            String accessControlRequestHeaders
    ) {
        String m = method == null ? "" : method.trim().toUpperCase(Locale.ROOT);
        String p = path == null ? "" : path.trim();

        Map<String, String> headers = new LinkedHashMap<>();
        headers.put("X-Content-Type-Options", "nosniff");
        headers.put("X-Frame-Options", "DENY");
        headers.put("Referrer-Policy", "no-referrer");
        headers.put("Cache-Control", "no-store");

        boolean originAllowed = isOriginAllowed(origin);

        if ("OPTIONS".equals(m)) {
            // Preflight
            if (!originAllowed) {
                return new Response(403, headers, "");
            }
            String reqMethod = accessControlRequestMethod == null ? "" :
                    accessControlRequestMethod.trim().toUpperCase(Locale.ROOT);
            if (!ALLOWED_METHODS.contains(reqMethod)) {
                return new Response(405, headers, "");
            }
            Set<String> requested = parseHeaderList(accessControlRequestHeaders);
            List<String> allowedBack = new ArrayList<>();
            if (!requested.isEmpty()) {
                for (String h : requested) {
                    if (ALLOWED_HEADERS.contains(h)) {
                        allowedBack.add(h);
                    }
                }
            } else {
                allowedBack.addAll(ALLOWED_HEADERS);
            }
            // CORS headers
            headers.put("Access-Control-Allow-Origin", origin.trim());
            headers.put("Vary", "Origin");
            headers.put("Access-Control-Allow-Methods", joinCSV(ALLOWED_METHODS));
            if (!allowedBack.isEmpty()) {
                headers.put("Access-Control-Allow-Headers", joinCSV(allowedBack));
            }
            headers.put("Access-Control-Max-Age", "600");
            // Do not set Allow-Credentials unless needed; omitting prevents inadvertent exposure
            return new Response(204, headers, "");
        }

        // Actual request
        if (!"/api/data".equals(p)) {
            return new Response(404, headers, "{\"error\":\"not found\"}");
        }
        if (!ALLOWED_METHODS.contains(m)) {
            return new Response(405, headers, "{\"error\":\"method not allowed\"}");
        }

        if (originAllowed) {
            headers.put("Access-Control-Allow-Origin", origin.trim());
            headers.put("Vary", "Origin");
            headers.put("Access-Control-Expose-Headers", "Content-Type, Content-Length");
            // Not setting Allow-Credentials by default for safety
        }
        headers.put("Content-Type", "application/json; charset=utf-8");

        String body = "{\"message\":\"Hello from CORS protected API\",\"path\":\"" + p + "\"}";
        return new Response(200, headers, body);
    }

    // Test helper: pretty print a response (for demonstration)
    private static void printResponse(String title, Response r) {
        System.out.println("== " + title + " ==");
        System.out.println("Status: " + r.status);
        for (Map.Entry<String, String> e : r.headers.entrySet()) {
            System.out.println(e.getKey() + ": " + e.getValue());
        }
        System.out.println("Body: " + r.body);
        System.out.println();
    }

    public static void main(String[] args) {
        // 5 test cases
        Response t1 = handleRequest(
                "OPTIONS",
                "/api/data",
                "https://trustedorigin.com",
                "GET",
                "Content-Type, X-Requested-With"
        );
        Response t2 = handleRequest(
                "OPTIONS",
                "/api/data",
                "https://evil.com",
                "GET",
                "Content-Type"
        );
        Response t3 = handleRequest(
                "GET",
                "/api/data",
                "http://localhost:3000",
                null,
                null
        );
        Response t4 = handleRequest(
                "OPTIONS",
                "/api/data",
                "http://localhost:3000",
                "DELETE",
                "Content-Type"
        );
        Response t5 = handleRequest(
                "GET",
                "/api/data",
                "https://unknown.com",
                null,
                null
        );

        printResponse("Test1 Allowed Origin Preflight", t1);
        printResponse("Test2 Disallowed Origin Preflight", t2);
        printResponse("Test3 Allowed Origin Actual GET", t3);
        printResponse("Test4 Not Allowed Method Preflight", t4);
        printResponse("Test5 Disallowed Origin Actual GET", t5);
    }
}