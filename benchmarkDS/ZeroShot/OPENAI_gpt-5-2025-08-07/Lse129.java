import java.util.*;

public class Lse129 {

    // Request holds query parameters
    public static class Request {
        private final Map<String, String> params;

        public Request(Map<String, String> params) {
            this.params = params == null ? new HashMap<>() : new HashMap<>(params);
        }

        public String getParam(String key) {
            if (key == null) return null;
            String val = params.get(key);
            return val;
        }
    }

    // Template for simple {{name}} rendering with HTML escaping
    public static class Template {
        private final String templateText;

        public Template(String templateText) {
            this.templateText = templateText == null ? "" : templateText;
        }

        public String render(Map<String, String> context) {
            String name = context != null ? context.getOrDefault("name", "") : "";
            String safe = htmlEscape(name);
            return templateText.replace("{{name}}", safe);
        }

        private static String htmlEscape(String s) {
            if (s == null) return "";
            StringBuilder out = new StringBuilder();
            for (int i = 0; i < s.length(); i++) {
                char c = s.charAt(i);
                switch (c) {
                    case '&': out.append("&amp;"); break;
                    case '<': out.append("&lt;"); break;
                    case '>': out.append("&gt;"); break;
                    case '"': out.append("&quot;"); break;
                    case '\'': out.append("&#x27;"); break;
                    case '/': out.append("&#x2F;"); break;
                    default: out.append(c); break;
                }
            }
            return out.toString();
        }
    }

    // Environment holds request and template
    public static class Environment {
        private final Request request;
        private final Template template;

        public Environment(Request request, Template template) {
            this.request = request;
            this.template = template;
        }

        public Request getRequest() {
            return request;
        }

        public Template getTemplate() {
            return template;
        }
    }

    // Response object
    public static class Response {
        public final int status;
        public final String body;

        public Response(int status, String body) {
            this.status = status;
            this.body = body == null ? "" : body;
        }

        @Override
        public String toString() {
            return "Status: " + status + ", Body: " + body;
        }
    }

    private static String sanitizeName(String name) {
        String n = name == null ? "" : name;
        n = n.trim();
        // Remove control characters
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < n.length(); i++) {
            char c = n.charAt(i);
            if (c >= 32 && c != 127) {
                sb.append(c);
            }
        }
        n = sb.toString();
        // Limit length
        if (n.length() > 100) {
            n = n.substring(0, 100);
        }
        if (n.isEmpty()) {
            n = "Guest";
        }
        return n;
    }

    // Renders response from environment
    public static Response render_response_from_env(Environment env) {
        if (env == null) {
            return new Response(500, "Internal Server Error");
        }
        Request req = env.getRequest();
        String rawName = req != null ? req.getParam("name") : null;
        String safeName = sanitizeName(rawName);
        Template tpl = env.getTemplate();
        if (tpl == null) {
            return new Response(500, "Template not found");
        }
        Map<String, String> context = new HashMap<>();
        context.put("name", safeName);
        String rendered = tpl.render(context);
        return new Response(200, rendered);
    }

    // Home function calls render_response_from_env with env
    public static Response home(Environment env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        Template tpl = new Template("Hello, {{name}}!");
        // Test case 1: Normal name
        Map<String, String> p1 = new HashMap<>();
        p1.put("name", "Alice");
        Response r1 = home(new Environment(new Request(p1), tpl));
        System.out.println(r1);

        // Test case 2: Missing name
        Map<String, String> p2 = new HashMap<>();
        Response r2 = home(new Environment(new Request(p2), tpl));
        System.out.println(r2);

        // Test case 3: XSS attempt
        Map<String, String> p3 = new HashMap<>();
        p3.put("name", "<script>alert(1)</script>");
        Response r3 = home(new Environment(new Request(p3), tpl));
        System.out.println(r3);

        // Test case 4: Whitespace and control chars
        Map<String, String> p4 = new HashMap<>();
        p4.put("name", "  Bob \n\t ");
        Response r4 = home(new Environment(new Request(p4), tpl));
        System.out.println(r4);

        // Test case 5: Very long string
        StringBuilder longName = new StringBuilder();
        for (int i = 0; i < 200; i++) longName.append('A');
        Map<String, String> p5 = new HashMap<>();
        p5.put("name", longName.toString());
        Response r5 = home(new Environment(new Request(p5), tpl));
        System.out.println(r5);
    }
}