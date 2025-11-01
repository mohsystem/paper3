import java.util.*;
import java.nio.charset.StandardCharsets;

public class Lse129 {

    public static final class Request {
        private final Map<String, String> params;

        public Request(Map<String, String> params) {
            this.params = params == null ? new HashMap<>() : new HashMap<>(params);
        }

        public String getParam(String key) {
            if (key == null) return "";
            String v = params.get(key);
            return v == null ? "" : v;
        }
    }

    public static final class Env {
        private final Request request;
        private final String template;

        public Env(Request request, String template) {
            this.request = request == null ? new Request(Collections.emptyMap()) : request;
            this.template = template;
        }

        public Request getRequest() {
            return request;
        }

        public String getTemplate() {
            return template;
        }
    }

    public static final class Response {
        private final int status;
        private final String body;

        public Response(int status, String body) {
            this.status = status;
            this.body = body == null ? "" : body;
        }

        public int getStatus() {
            return status;
        }

        public String getBody() {
            return body;
        }

        @Override
        public String toString() {
            return "Status=" + status + " Body=" + body;
        }
    }

    private static String sanitizeName(String raw) {
        if (raw == null) return "Guest";
        String s = raw.trim();
        if (s.isEmpty()) return "Guest";
        StringBuilder sb = new StringBuilder();
        int count = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z')
                    || (c >= 'A' && c <= 'Z')
                    || (c >= '0' && c <= '9')
                    || c == ' ' || c == '-' || c == '_';
            if (ok) {
                sb.append(c);
                count++;
                if (count >= 100) break;
            }
        }
        String out = sb.toString().trim();
        if (out.isEmpty()) return "Guest";
        return out;
    }

    private static String htmlEscape(String s) {
        if (s == null || s.isEmpty()) return "";
        StringBuilder out = new StringBuilder(Math.min(4096, s.length() * 2));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#x27;"); break;
                case '/': out.append("&#x2F;"); break;
                default: out.append(c);
            }
        }
        return out.toString();
    }

    private static String renderTemplate(String template, String nameVal) {
        String safeTemplate = (template == null || template.length() > 8000) ? "Hello, {{name}}!" : template;
        String token = "{{name}}";
        StringBuilder sb = new StringBuilder(Math.min(8192, safeTemplate.length() + Math.max(0, nameVal.length() - token.length()) * 2));
        int i = 0;
        while (i < safeTemplate.length()) {
            if (i + token.length() <= safeTemplate.length() && safeTemplate.regionMatches(i, token, 0, token.length())) {
                sb.append(nameVal);
                i += token.length();
            } else {
                sb.append(safeTemplate.charAt(i));
                i++;
            }
            if (sb.length() > 8192) {
                return "Error: content too large";
            }
        }
        return sb.toString();
    }

    public static Response render_response_from_env(Env env) {
        if (env == null) {
            return new Response(400, "Bad Request");
        }
        String rawName = env.getRequest() != null ? env.getRequest().getParam("name") : "";
        String sanitized = sanitizeName(rawName);
        String escaped = htmlEscape(sanitized);
        String body = renderTemplate(env.getTemplate(), escaped);
        return new Response(200, body);
    }

    public static Response home(Env env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        // Test 1: normal name
        Map<String, String> p1 = new HashMap<>();
        p1.put("name", "Alice");
        Env e1 = new Env(new Request(p1), "Hello, {{name}}! Welcome.");
        System.out.println(home(e1).getBody());

        // Test 2: name with HTML/script - should sanitize
        Map<String, String> p2 = new HashMap<>();
        p2.put("name", "Bob <script>alert(1)</script>");
        Env e2 = new Env(new Request(p2), "Hi {{name}}, your input is safe.");
        System.out.println(home(e2).getBody());

        // Test 3: empty name -> Guest
        Map<String, String> p3 = new HashMap<>();
        p3.put("name", "   ");
        Env e3 = new Env(new Request(p3), "Hello, {{name}}!");
        System.out.println(home(e3).getBody());

        // Test 4: very long name -> truncated/sanitized or Guest if invalid
        StringBuilder longName = new StringBuilder();
        for (int i = 0; i < 150; i++) longName.append('X');
        Map<String, String> p4 = new HashMap<>();
        p4.put("name", longName.toString());
        Env e4 = new Env(new Request(p4), "Long name test: {{name}}.");
        System.out.println(home(e4).getBody());

        // Test 5: multiple placeholders and null template -> default
        Map<String, String> p5 = new HashMap<>();
        p5.put("name", "Eve-User_42");
        Env e5 = new Env(new Request(p5), null);
        System.out.println(home(e5).getBody());
    }
}