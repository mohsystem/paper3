// Chain-of-Through process in code generation:
// 1) Problem understanding:
//    - Build a simple "web-like" application with a single route "/info" for GET requests.
//    - When accessed, function info() is called, reading username/email/ssn from a "session" map,
//      and returns an HTML page containing username, email, and last 4 digits of the SSN.
//    - If not present, store safe defaults in the session to ensure values exist.
// 2) Security requirements:
//    - Prevent XSS via HTML escaping of username and email.
//    - Do not expose full SSN; only last 4 digits of numeric characters of SSN should be shown.
//    - Never log or echo sensitive full SSN.
// 3) Secure coding generation:
//    - Implement HTML escaping.
//    - Sanitize SSN to digits-only and compute last 4 with masking if fewer than 4 digits.
// 4) Code review:
//    - Ensure no logging of SSN.
//    - Ensure proper escaping order (& first).
//    - Avoid null handling issues; set defaults if missing.
// 5) Secure code output:
//    - Finalized functions: info(session) and handle(method, path, session) returning strings.
//    - Include main with 5 test cases simulating requests to /info.

import java.util.*;

public class Lse40 {

    // HTML escape utility
    private static String escapeHtml(String s) {
        if (s == null) return "";
        String out = s;
        out = out.replace("&", "&amp;");
        out = out.replace("<", "&lt;");
        out = out.replace(">", "&gt;");
        out = out.replace("\"", "&quot;");
        out = out.replace("'", "&#x27;");
        return out;
    }

    // Extract last 4 digits from SSN; digits-only, mask if fewer than 4.
    private static String last4Safe(String ssn) {
        if (ssn == null) ssn = "";
        StringBuilder digits = new StringBuilder();
        for (int i = 0; i < ssn.length(); i++) {
            char c = ssn.charAt(i);
            if (c >= '0' && c <= '9') digits.append(c);
        }
        String d = digits.toString();
        if (d.length() >= 4) {
            return d.substring(d.length() - 4);
        } else {
            int pad = 4 - d.length();
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < pad; i++) sb.append('*');
            sb.append(d);
            return sb.toString();
        }
    }

    // The info() function: reads/writes session and returns safe HTML.
    public static String info(Map<String, String> session) {
        if (session == null) session = new HashMap<>();
        // Ensure session has required fields; set safe defaults if missing
        session.putIfAbsent("username", "Jane Doe");
        session.putIfAbsent("email", "jane.doe@example.com");
        session.putIfAbsent("ssn", "123-45-6789");

        String username = escapeHtml(session.get("username"));
        String email = escapeHtml(session.get("email"));
        String ssnLast4 = last4Safe(session.get("ssn"));

        // Build response HTML
        StringBuilder html = new StringBuilder();
        html.append("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>User Info</title></head><body>");
        html.append("<h1>User Information</h1>");
        html.append("<ul>");
        html.append("<li>Username: ").append(username).append("</li>");
        html.append("<li>Email: ").append(email).append("</li>");
        html.append("<li>SSN (last 4): ").append(ssnLast4).append("</li>");
        html.append("</ul>");
        html.append("</body></html>");
        return html.toString();
    }

    // Simulated router: only supports GET /info
    public static String handle(String method, String path, Map<String, String> session) {
        if (method == null || path == null) return "400 Bad Request";
        if ("GET".equalsIgnoreCase(method) && "/info".equals(path)) {
            return info(session);
        }
        return "404 Not Found";
    }

    // 5 test cases
    public static void main(String[] args) {
        // Test 1: Empty session -> defaults
        Map<String, String> s1 = new HashMap<>();
        System.out.println(handle("GET", "/info", s1));
        System.out.println("-----");

        // Test 2: Provided clean data
        Map<String, String> s2 = new HashMap<>();
        s2.put("username", "Alice");
        s2.put("email", "alice@example.com");
        s2.put("ssn", "987654321");
        System.out.println(handle("GET", "/info", s2));
        System.out.println("-----");

        // Test 3: XSS attempt in username; short SSN
        Map<String, String> s3 = new HashMap<>();
        s3.put("username", "<script>alert(1)</script>");
        s3.put("email", "bob@example.com");
        s3.put("ssn", "12");
        System.out.println(handle("GET", "/info", s3));
        System.out.println("-----");

        // Test 4: SSN with non-digits and spaces
        Map<String, String> s4 = new HashMap<>();
        s4.put("username", "Charlie");
        s4.put("email", "charlie+tag@example.co.uk");
        s4.put("ssn", "  999-88  7777 ");
        System.out.println(handle("GET", "/info", s4));
        System.out.println("-----");

        // Test 5: Missing ssn; present username/email
        Map<String, String> s5 = new HashMap<>();
        s5.put("username", "Dana");
        s5.put("email", "dana@example.org");
        System.out.println(handle("GET", "/info", s5)); // will fill default ssn
    }
}