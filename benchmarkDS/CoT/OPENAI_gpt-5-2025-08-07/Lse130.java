import java.util.*;

class Lse130 {
    // Data models
    static class Request {
        private final Map<String, String> params;
        Request(Map<String, String> params) { this.params = params; }
        String getParam(String key) { return params.get(key); }
    }

    static class Env {
        final Request request;
        final String template;
        Env(Request request, String template) {
            this.request = request;
            this.template = template;
        }
    }

    static class Response {
        private final String body;
        Response(String body) { this.body = body; }
        String getBody() { return body; }
    }

    // Helper to render the template
    private static String renderTemplate(String template, String name) {
        if (template == null) return "";
        String safeName = name == null ? "" : name;
        return template.replace("{{name}}", safeName).replace("{name}", safeName);
    }

    // Function: render_response_from_env
    public static Response render_response_from_env(Env env) {
        String name = env != null && env.request != null ? env.request.getParam("name") : "";
        String template = env != null ? env.template : "";
        String rendered = renderTemplate(template, name);
        return new Response(rendered);
    }

    // Function: home
    public static Response home(Env env) {
        return render_response_from_env(env);
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        List<Env> tests = new ArrayList<>();
        tests.add(new Env(new Request(Map.of("name", "Alice")), "Hello, {{name}}!"));
        tests.add(new Env(new Request(Map.of("name", "Bob")), "Welcome, {name}."));
        tests.add(new Env(new Request(Map.of("name", "Charlie")), "Hi {name}, this is a test."));
        tests.add(new Env(new Request(Map.of("name", "Dana")), "Dear {name}, your order is ready."));
        tests.add(new Env(new Request(Map.of("name", "Eve")), "Goodbye, {{name}}."));

        int i = 1;
        for (Env env : tests) {
            Response r = home(env);
            System.out.println("Test " + i + ": " + r.getBody());
            i++;
        }
    }
}