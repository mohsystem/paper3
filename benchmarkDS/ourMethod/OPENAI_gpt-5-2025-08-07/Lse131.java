import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class Lse131 {
    // Environment holding a template
    public static final class Environment {
        private final String template;
        public Environment(String template) {
            this.template = template;
        }
        public String getTemplate() {
            return template;
        }
    }

    // Request holding the "name" parameter
    public static final class Request {
        private final String name;
        public Request(String name) {
            this.name = name;
        }
        public String getName() {
            return name;
        }
    }

    // HTML escape to prevent XSS
    private static String htmlEscape(String input) {
        if (input == null) return "";
        StringBuilder sb = new StringBuilder(Math.min(4096, input.length() + 16));
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#x27;"); break;
                default: sb.append(c);
            }
            if (sb.length() >= 4096) break; // enforce max size
        }
        return sb.toString();
    }

    // Validate and sanitize name: allowed chars [A-Za-z0-9 _.-], length 1..100
    private static String sanitizeName(String input) {
        if (input == null) return "Guest";
        String trimmed = input.trim();
        if (trimmed.isEmpty() || trimmed.length() > 100) return "Guest";
        for (int i = 0; i < trimmed.length(); i++) {
            char c = trimmed.charAt(i);
            boolean allowed = (c >= 'A' && c <= 'Z')
                    || (c >= 'a' && c <= 'z')
                    || (c >= '0' && c <= '9')
                    || c == ' ' || c == '_' || c == '-' || c == '.';
            if (!allowed) return "Guest";
        }
        return trimmed;
    }

    // Safely get template from env, enforce length limits
    private static String getTemplate(Environment env) {
        String tpl = (env != null && env.getTemplate() != null) ? env.getTemplate() : "Hello, {{name}}!";
        if (tpl.length() > 2000) {
            // Fail closed to a safe default if template is excessively large
            return "Hello, {{name}}!";
        }
        return tpl;
    }

    // Render by replacing all occurrences of {{name}} with escaped, sanitized name
    public static String render_response_from_env(Environment env, Request request) {
        String safeName = sanitizeName(request != null ? request.getName() : null);
        String escapedName = htmlEscape(safeName);
        String template = getTemplate(env);

        String token = "{{name}}";
        StringBuilder out = new StringBuilder(Math.min(4096, template.length() + escapedName.length() + 16));
        int i = 0;
        int tokenLen = token.length();

        while (i < template.length()) {
            if (i <= template.length() - tokenLen && template.regionMatches(false, i, token, 0, tokenLen)) {
                out.append(escapedName);
                i += tokenLen;
            } else {
                out.append(template.charAt(i));
                i++;
            }
            if (out.length() >= 4096) {
                // Enforce upper bound on response size
                break;
            }
        }

        // Ensure UTF-8-safe (string in Java is already UTF-16, but enforce size)
        String result = out.toString();
        if (result.length() > 4096) {
            result = result.substring(0, 4096);
        }
        return result;
    }

    public static String home(Environment env, Request request) {
        return render_response_from_env(env, request);
    }

    public static void main(String[] args) {
        List<String> outputs = new ArrayList<>();

        // Test 1: Normal name
        outputs.add(home(new Environment("Welcome, {{name}}."), new Request("Alice")));

        // Test 2: Name with potentially dangerous characters (should be escaped)
        outputs.add(home(new Environment("<h1>Hello, {{name}}</h1>"), new Request("<script>alert(1)</script>")));

        // Test 3: Too long name -> fallback to Guest
        StringBuilder longName = new StringBuilder();
        for (int k = 0; k < 150; k++) longName.append('A');
        outputs.add(home(new Environment("Hi {{name}}!"), new Request(longName.toString())));

        // Test 4: Invalid characters in name -> fallback to Guest
        outputs.add(home(new Environment("User: {{name}}"), new Request("Bob!!@@##")));

        // Test 5: Missing template -> default template
        outputs.add(home(new Environment(null), new Request("Charlie")));

        // Print results
        for (int idx = 0; idx < outputs.size(); idx++) {
            System.out.println("Test " + (idx + 1) + ": " + outputs.get(idx));
        }
    }
}