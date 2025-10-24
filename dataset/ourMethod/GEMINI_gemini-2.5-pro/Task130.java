import java.util.*;

public class Task130 {

    // Whitelist of allowed origins. Using a wildcard (*) is discouraged for security reasons.
    private static final Set<String> ALLOWED_ORIGINS = new HashSet<>(Arrays.asList(
        "https://example.com",
        "https://trusted.site.org"
    ));
    
    // Define allowed methods and headers for preflight requests.
    private static final String ALLOWED_METHODS = "GET, POST, OPTIONS";
    private static final String ALLOWED_HEADERS = "Content-Type, Authorization";
    private static final String MAX_AGE = "86400"; // 24 hours

    // Represents a simplified HTTP response
    public static class Response {
        public final int statusCode;
        public final Map<String, String> headers;
        public final String body;

        public Response(int statusCode, Map<String, String> headers, String body) {
            this.statusCode = statusCode;
            this.headers = Collections.unmodifiableMap(new HashMap<>(headers));
            this.body = body;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append("Status: ").append(statusCode).append("\n");
            sb.append("Headers:\n");
            if (headers.isEmpty()) {
                sb.append("  (none)\n");
            } else {
                headers.forEach((key, value) -> sb.append("  ").append(key).append(": ").append(value).append("\n"));
            }
            sb.append("Body:\n").append(body);
            return sb.toString();
        }
    }
    
    /**
     * Handles a request by applying Cross-Origin Resource Sharing (CORS) rules.
     * This function simulates a server-side API endpoint's CORS logic.
     *
     * @param method The HTTP method (e.g., "GET", "OPTIONS"). Must not be null.
     * @param originHeader The value of the "Origin" header from the request. Can be null.
     * @return A Response object containing status, headers, and body.
     */
    public static Response handleCorsRequest(String method, String originHeader) {
        Map<String, String> headers = new LinkedHashMap<>(); // Use LinkedHashMap to preserve order for printing
        
        // Rule #3: Validate input. The origin must be from the predefined whitelist.
        boolean isOriginAllowed = originHeader != null && ALLOWED_ORIGINS.contains(originHeader);

        if (!isOriginAllowed) {
            // If the origin is not allowed, or no origin header was provided,
            // do not add any CORS headers and return an error.
            return new Response(403, Collections.emptyMap(), "{\"error\": \"CORS policy does not allow this origin.\"}");
        }
        
        // The origin is allowed, so add the primary CORS response header.
        headers.put("Access-Control-Allow-Origin", originHeader);

        // Handle preflight (OPTIONS) request
        if ("OPTIONS".equalsIgnoreCase(method)) {
            headers.put("Access-Control-Allow-Methods", ALLOWED_METHODS);
            headers.put("Access-Control-Allow-Headers", ALLOWED_HEADERS);
            headers.put("Access-Control-Max-Age", MAX_AGE);
            return new Response(204, headers, ""); // Preflight response has status 204 No Content
        }

        // Handle actual API requests (e.g., GET, POST)
        if ("GET".equalsIgnoreCase(method) || "POST".equalsIgnoreCase(method)) {
            headers.put("Content-Type", "application/json");
            return new Response(200, headers, "{\"data\": \"some protected data\"}");
        }

        // If the method is not OPTIONS, GET, or POST, it's not allowed.
        // The Access-Control-Allow-Origin header is still sent because the origin was valid.
        return new Response(405, headers, "{\"error\": \"Method not allowed.\"}");
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: Valid GET request from an allowed origin ---");
        Response res1 = handleCorsRequest("GET", "https://example.com");
        System.out.println(res1);
        System.out.println("\n----------------------------------------------------------\n");

        System.out.println("--- Test Case 2: Valid Preflight (OPTIONS) request from an allowed origin ---");
        Response res2 = handleCorsRequest("OPTIONS", "https://trusted.site.org");
        System.out.println(res2);
        System.out.println("\n----------------------------------------------------------\n");

        System.out.println("--- Test Case 3: Request from a disallowed origin ---");
        Response res3 = handleCorsRequest("GET", "https://malicious.com");
        System.out.println(res3);
        System.out.println("\n----------------------------------------------------------\n");
        
        System.out.println("--- Test Case 4: Request with a missing Origin header ---");
        Response res4 = handleCorsRequest("GET", null);
        System.out.println(res4);
        System.out.println("\n----------------------------------------------------------\n");

        System.out.println("--- Test Case 5: Request with a disallowed method (PUT) from an allowed origin ---");
        Response res5 = handleCorsRequest("PUT", "https://example.com");
        System.out.println(res5);
        System.out.println("\n----------------------------------------------------------\n");
    }
}