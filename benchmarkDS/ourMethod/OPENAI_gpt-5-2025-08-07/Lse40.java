import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse40 {

    public static String htmlEscape(String input) {
        if (input == null) return "N/A";
        StringBuilder sb = new StringBuilder(input.length() + 16);
        for (char c : input.toCharArray()) {
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#x27;"); break;
                case '/': sb.append("&#x2F;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    public static String last4OfSSN(String ssn) {
        if (ssn == null) return "N/A";
        StringBuilder digits = new StringBuilder();
        for (int i = 0; i < ssn.length(); i++) {
            char c = ssn.charAt(i);
            if (Character.isDigit(c)) {
                digits.append(c);
                if (digits.length() > 32) break; // basic sanity cap
            }
        }
        if (digits.length() < 4) return "N/A";
        return digits.substring(digits.length() - 4);
    }

    // The route handler function for /info
    public static String info(Map<String, String> session) {
        if (session == null) {
            return httpResponse(400, "text/plain; charset=UTF-8", "Bad Request");
        }
        String username = htmlEscape(session.get("username"));
        String email = htmlEscape(session.get("email"));
        String ssnLast4 = htmlEscape(last4OfSSN(session.get("ssn")));

        String body = ""
            + "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Info</title></head><body>"
            + "<h1>User Info</h1>"
            + "<ul>"
            + "<li>Username: " + username + "</li>"
            + "<li>Email: " + email + "</li>"
            + "<li>SSN Last 4: " + ssnLast4 + "</li>"
            + "</ul>"
            + "</body></html>";

        return httpResponse(200, "text/html; charset=UTF-8", body);
    }

    public static String handleRequest(String method, String path, Map<String, String> session) {
        if (method == null || path == null) {
            return httpResponse(400, "text/plain; charset=UTF-8", "Bad Request");
        }
        if (!"GET".equals(method)) {
            return httpResponse(405, "text/plain; charset=UTF-8", "Method Not Allowed");
        }
        if ("/info".equals(path)) {
            return info(session);
        }
        return httpResponse(404, "text/plain; charset=UTF-8", "Not Found");
    }

    private static String httpResponse(int statusCode, String contentType, String body) {
        // Simple HTTP-like response string for demonstration/testing
        String reason;
        switch (statusCode) {
            case 200: reason = "OK"; break;
            case 400: reason = "Bad Request"; break;
            case 404: reason = "Not Found"; break;
            case 405: reason = "Method Not Allowed"; break;
            default: reason = "OK";
        }
        byte[] bodyBytes = body.getBytes(StandardCharsets.UTF_8);
        return "HTTP/1.1 " + statusCode + " " + reason + "\r\n"
                + "Content-Type: " + contentType + "\r\n"
                + "Content-Length: " + bodyBytes.length + "\r\n"
                + "Connection: close\r\n"
                + "\r\n"
                + body;
    }

    public static void main(String[] args) {
        // Test case 1: Valid session data
        Map<String, String> session1 = new HashMap<>();
        session1.put("username", "alice");
        session1.put("email", "alice@example.com");
        session1.put("ssn", "123-45-6789");
        System.out.println(handleRequest("GET", "/info", session1));

        // Test case 2: SSN with different formatting (only last 4 should show)
        Map<String, String> session2 = new HashMap<>();
        session2.put("username", "bob");
        session2.put("email", "bob@example.com");
        session2.put("ssn", "0000 1111 2222 3333 4444 5555 6666 7777 8888 9999"); // last 4 = 9999
        System.out.println(handleRequest("GET", "/info", session2));

        // Test case 3: Malicious content in username (ensure HTML-escaped)
        Map<String, String> session3 = new HashMap<>();
        session3.put("username", "<script>alert('xss')</script>");
        session3.put("email", "evil@example.com");
        session3.put("ssn", "111223333");
        System.out.println(handleRequest("GET", "/info", session3));

        // Test case 4: Missing SSN -> N/A
        Map<String, String> session4 = new HashMap<>();
        session4.put("username", "charlie");
        session4.put("email", "charlie@example.com");
        System.out.println(handleRequest("GET", "/info", session4));

        // Test case 5: Wrong path -> 404 and Wrong method -> 405
        Map<String, String> session5 = new HashMap<>();
        session5.put("username", "dana");
        session5.put("email", "dana@example.com");
        session5.put("ssn", "444-55-1234");
        System.out.println(handleRequest("GET", "/unknown", session5));
        System.out.println(handleRequest("POST", "/info", session5));
    }
}