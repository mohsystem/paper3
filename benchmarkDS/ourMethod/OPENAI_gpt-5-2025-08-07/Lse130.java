import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class Lse130 {

    private static final int MAX_NAME_LEN = 100;
    private static final int MAX_TEMPLATE_LEN = 4096;

    public static final class Env {
        private final String template;
        private final String requestName;

        public Env(String template, String requestName) {
            this.template = template == null ? "" : template;
            this.requestName = requestName;
        }

        public String getTemplate() {
            return template;
        }

        public String getRequestName() {
            return requestName;
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
            return "Status: " + status + ", Body: " + body;
        }
    }

    private static String sanitizeName(String name) {
        String n = name == null ? "" : name;
        StringBuilder sb = new StringBuilder();
        int limit = Math.min(n.length(), MAX_NAME_LEN);
        for (int i = 0; i < limit; i++) {
            char c = n.charAt(i);
            if (Character.isLetterOrDigit(c) || c == ' ' || c == '-' || c == '_' || c == '.' || c == ',') {
                sb.append(c);
            } // else drop disallowed characters
        }
        String out = sb.toString().trim();
        if (out.isEmpty()) {
            out = "Guest";
        }
        return out;
    }

    private static String escapeHtml(String s) {
        String str = s == null ? "" : s;
        StringBuilder sb = new StringBuilder(Math.min(str.length() * 6, 1024)); // rough cap
        for (int i = 0; i < str.length(); i++) {
            char c = str.charAt(i);
            switch (c) {
                case '&': sb.append("&amp;"); break;
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#39;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    private static String renderTemplate(String template, String escapedName) {
        String tpl = template == null ? "" : template;
        if (tpl.length() > MAX_TEMPLATE_LEN) {
            return "Invalid template";
        }
        return tpl.replace("{name}", escapedName);
    }

    // The function takes an environment, extracts name and template, renders safely, and returns a response
    public static Response render_response_from_env(Env env) {
        if (env == null) {
            return new Response(400, "Invalid environment");
        }
        String sanitized = sanitizeName(env.getRequestName());
        String escaped = escapeHtml(sanitized);
        String rendered = renderTemplate(env.getTemplate(), escaped);
        return new Response(200, rendered);
    }

    // The home function calls render_response_from_env with the env and returns its response
    public static Response home(Env env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        List<Env> tests = new ArrayList<>();
        tests.add(new Env("Hello, {name}!", "Alice"));
        tests.add(new Env("<div>User: {name}</div>", "Bob <script>alert(1)</script>"));
        tests.add(new Env("Len test: {name}", "a".repeat(500)));
        tests.add(new Env("Safe SQL-ish: {name}", "Eve; DROP TABLE users;--"));
        tests.add(new Env("Welcome!", null));

        int i = 1;
        for (Env e : tests) {
            Response r = home(e);
            System.out.println("Test " + (i++) + " -> " + r.toString());
        }
    }
}