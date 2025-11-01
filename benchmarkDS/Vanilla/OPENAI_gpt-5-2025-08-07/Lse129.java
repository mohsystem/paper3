import java.util.HashMap;
import java.util.Map;

public class Lse129 {

    static class Request {
        private final Map<String, String> params;

        Request(Map<String, String> params) {
            this.params = params;
        }

        String getParam(String key) {
            return params.get(key);
        }
    }

    static class Env {
        final Request request;
        final String template;

        Env(Request request, String template) {
            this.request = request;
            this.template = template;
        }

        String getTemplate() {
            return template;
        }
    }

    static class Response {
        final int status;
        final String body;

        Response(int status, String body) {
            this.status = status;
            this.body = body;
        }
    }

    public static Response render_response_from_env(Env env) {
        String name = env.request.getParam("name");
        if (name == null || name.isEmpty()) {
            name = "World";
        }
        String tpl = env.getTemplate();
        if (tpl == null) {
            tpl = "";
        }
        String rendered = tpl.replace("{{name}}", name);
        return new Response(200, rendered);
    }

    public static Response home(Env env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        // Test case 1
        Map<String, String> p1 = new HashMap<>();
        p1.put("name", "Alice");
        Env e1 = new Env(new Request(p1), "Hello, {{name}}!");
        Response r1 = home(e1);
        System.out.println(r1.body);

        // Test case 2
        Map<String, String> p2 = new HashMap<>();
        p2.put("name", "Bob");
        Env e2 = new Env(new Request(p2), "Welcome, {{name}}.");
        Response r2 = home(e2);
        System.out.println(r2.body);

        // Test case 3 (empty name -> default)
        Map<String, String> p3 = new HashMap<>();
        p3.put("name", "");
        Env e3 = new Env(new Request(p3), "Hi, {{name}}");
        Response r3 = home(e3);
        System.out.println(r3.body);

        // Test case 4 (multiple occurrences)
        Map<String, String> p4 = new HashMap<>();
        p4.put("name", "Eve");
        Env e4 = new Env(new Request(p4), "User: {{name}} | Echo: {{name}}");
        Response r4 = home(e4);
        System.out.println(r4.body);

        // Test case 5 (no name provided -> default)
        Map<String, String> p5 = new HashMap<>();
        Env e5 = new Env(new Request(p5), "Greetings, {{name}}!");
        Response r5 = home(e5);
        System.out.println(r5.body);
    }
}