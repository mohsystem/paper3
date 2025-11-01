import java.util.*;

public class Lse130 {

    public static class Env {
        private final Map<String, String> request;
        private final String template;

        public Env(Map<String, String> request, String template) {
            this.request = request != null ? new HashMap<>(request) : new HashMap<>();
            this.template = template;
        }

        public Map<String, String> getRequest() {
            return request;
        }

        public String getTemplate() {
            return template;
        }
    }

    private static String htmlEscape(String input) {
        if (input == null) return "";
        StringBuilder sb = new StringBuilder(input.length());
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
        }
        return sb.toString();
    }

    public static String render_response_from_env(Env env) {
        if (env == null) {
            return "Invalid environment";
        }
        String name = env.getRequest().getOrDefault("name", "World");
        String template = env.getTemplate() != null ? env.getTemplate() : "Hello, {{name}}!";
        final int MAX_TEMPLATE_LEN = 1_000_000;
        if (template.length() > MAX_TEMPLATE_LEN) {
            return "Template too large";
        }
        String safeName = htmlEscape(name);
        String rendered = template.replace("{{name}}", safeName);
        return rendered;
    }

    public static String home(Env env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        // Test case 1: Normal name with default template
        Map<String, String> req1 = new HashMap<>();
        req1.put("name", "Alice");
        Env env1 = new Env(req1, "Hello, {{name}}!");
        System.out.println(home(env1));

        // Test case 2: Missing name parameter -> default "World"
        Map<String, String> req2 = new HashMap<>();
        Env env2 = new Env(req2, "Welcome, {{name}}.");
        System.out.println(home(env2));

        // Test case 3: Name with HTML that should be escaped
        Map<String, String> req3 = new HashMap<>();
        req3.put("name", "<script>alert(1)</script>");
        Env env3 = new Env(req3, "<div>User: {{name}}</div>");
        System.out.println(home(env3));

        // Test case 4: Missing template -> default template used
        Map<String, String> req4 = new HashMap<>();
        req4.put("name", "Bob");
        Env env4 = new Env(req4, null);
        System.out.println(home(env4));

        // Test case 5: Multiple placeholders
        Map<String, String> req5 = new HashMap<>();
        req5.put("name", "Charlie");
        Env env5 = new Env(req5, "Hi {{name}}, bye {{name}}!");
        System.out.println(home(env5));
    }
}