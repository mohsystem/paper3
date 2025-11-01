import java.util.HashMap;
import java.util.Map;

public class Lse129 {
    static class Request {
        private final Map<String, String> params;
        Request(Map<String, String> params) {
            this.params = params;
        }
        String getParam(String key) {
            return params.getOrDefault(key, "");
        }
    }

    static class Environment {
        private final String template;
        private final Request request;
        Environment(String template, Request request) {
            this.template = template;
            this.request = request;
        }
        String getTemplate() {
            return template;
        }
        Request getRequest() {
            return request;
        }
    }

    static class Response {
        final String content;
        Response(String content) {
            this.content = content;
        }
    }

    static String renderTemplate(String template, Map<String, String> context) {
        String name = context.getOrDefault("name", "");
        String rendered = template.replace("{{ name }}", name);
        rendered = rendered.replace("{{name}}", name);
        return rendered;
    }

    public static Response render_response_from_env(Environment env) {
        String name = env.getRequest().getParam("name");
        String template = env.getTemplate();
        Map<String, String> ctx = new HashMap<>();
        ctx.put("name", name);
        String rendered = renderTemplate(template, ctx);
        return new Response(rendered);
    }

    public static Response home(Environment env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        // Test case 1
        {
            Map<String, String> p = new HashMap<>();
            p.put("name", "Alice");
            Environment env = new Environment("Hello, {{ name }}!", new Request(p));
            Response res = home(env);
            System.out.println(res.content);
        }
        // Test case 2
        {
            Map<String, String> p = new HashMap<>();
            p.put("name", "Bob");
            Environment env = new Environment("Welcome, {{name}}.", new Request(p));
            Response res = home(env);
            System.out.println(res.content);
        }
        // Test case 3
        {
            Map<String, String> p = new HashMap<>();
            p.put("name", "Eve");
            Environment env = new Environment("Hi {{ name }}, you're number {{ name }}", new Request(p));
            Response res = home(env);
            System.out.println(res.content);
        }
        // Test case 4
        {
            Map<String, String> p = new HashMap<>();
            p.put("name", "Carol");
            Environment env = new Environment("No placeholder here.", new Request(p));
            Response res = home(env);
            System.out.println(res.content);
        }
        // Test case 5
        {
            Map<String, String> p = new HashMap<>();
            // name missing -> treated as empty string
            Environment env = new Environment("{{ name }} only", new Request(p));
            Response res = home(env);
            System.out.println(res.content);
        }
    }
}