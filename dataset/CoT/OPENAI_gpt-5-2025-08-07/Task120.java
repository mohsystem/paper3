// Chain-of-Through Step 1: Problem understanding
// Purpose: Safely display user-supplied input on a web page by returning an HTML string with proper escaping.
// Inputs: A user-supplied String.
// Outputs: A complete HTML String with the user input safely embedded.
// Operations: HTML-escape user input and insert into a minimal HTML template.
//
// Chain-of-Through Step 2: Security requirements
// - Prevent XSS by escaping special HTML characters.
// - Avoid null-pointer issues by normalizing null input to empty.
// - Avoid unsafe concatenation by limiting to controlled template.
//
// Chain-of-Through Step 3: Secure coding generation
// - Implement robust HTML escaping.
// - Construct the HTML using a fixed template and the escaped input.
// - Include a restrictive CSP meta tag for defense-in-depth.
//
// Chain-of-Through Step 4: Code review
// - Escaping covers &, <, >, ", ', /.
// - No dynamic code execution, no untrusted HTML insertion.
// - Returns String; no external side effects beyond printing in tests.
//
// Chain-of-Through Step 5: Secure code output
// - Final code below.
public class Task120 {
    // Escapes special HTML characters to prevent XSS.
    public static String escapeHtml(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(Math.max(16, s.length() * 6));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#39;"); break;
                case '/': sb.append("&#47;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    // Returns a minimal HTML page containing the safely-escaped user input.
    public static String displayUserInputOnWebPage(String userInput) {
        String safe = escapeHtml(userInput);
        StringBuilder html = new StringBuilder(safe.length() + 256);
        html.append("<!doctype html><html lang=\"en\"><head>")
            .append("<meta charset=\"utf-8\">")
            .append("<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; style-src 'self' 'unsafe-inline';\">")
            .append("<title>Safe Echo</title></head><body>")
            .append("<h1>User Input</h1><p>")
            .append(safe)
            .append("</p></body></html>");
        return html.toString();
    }

    // 5 test cases
    public static void main(String[] args) {
        String[] tests = new String[] {
            "Hello, world!",
            "<script>alert('xss')</script>",
            "Tom & Jerry > Spike",
            "\"quoted\" and 'single'",
            "こんにちは <b>世界</b> & emojis 😊"
        };
        for (String t : tests) {
            String page = displayUserInputOnWebPage(t);
            System.out.println("----- Java Output -----");
            System.out.println(page);
        }
    }
}