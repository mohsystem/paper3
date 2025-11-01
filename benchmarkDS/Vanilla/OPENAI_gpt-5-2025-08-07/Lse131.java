import java.util.*;

public class Lse131 {
    static class Env {
        private final String template;
        Env(String template) { this.template = template; }
        String getTemplate() { return template; }
    }

    static class Request {
        private final String name;
        Request(String name) { this.name = name; }
        String getParameter(String key) {
            if ("name".equals(key)) return name;
            return null;
        }
    }

    static class Response {
        private final String body;
        Response(String body) { this.body = body; }
        String getBody() { return body; }
    }

    // Global environment and current request
    static Env env;
    static Request currentRequest;

    static Response render_response_from_env(Env environment) {
        String name = (currentRequest != null) ? currentRequest.getParameter("name") : "";
        if (name == null) name = "";
        String tpl = environment != null ? environment.getTemplate() : "";
        if (tpl == null) tpl = "";
        String rendered = tpl.replace("{{name}}", name);
        return new Response(rendered);
    }

    static Response home() {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        env = new Env("Hello, {{name}}!");
        String[] testNames = new String[] { "Alice", "Bob", "Charlie", "Dana", "Eve" };
        for (String n : testNames) {
            currentRequest = new Request(n);
            Response r = home();
            System.out.println(r.getBody());
        }
    }
}