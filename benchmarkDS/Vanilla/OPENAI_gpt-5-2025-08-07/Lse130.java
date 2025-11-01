import java.util.*;

public class Lse130 {
    static class Request {
        String name;
        Request(String name) { this.name = name; }
    }

    static class Environment {
        Request request;
        String template;
        Environment(Request request, String template) {
            this.request = request;
            this.template = template;
        }
    }

    static class Response {
        String body;
        Response(String body) { this.body = body; }
    }

    public static Response render_response_from_env(Environment env) {
        String t = env != null && env.template != null ? env.template : "";
        String n = (env != null && env.request != null) ? env.request.name : null;
        if (n == null || n.isEmpty()) n = "World";
        String rendered = t.replace("{name}", n);
        return new Response(rendered);
    }

    public static Response home(Environment env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        Environment[] tests = new Environment[] {
            new Environment(new Request("Alice"), "Hello, {name}!"),
            new Environment(new Request("Bob"), "Welcome, {name}."),
            new Environment(new Request(""), "Hi {name}"),
            new Environment(new Request("Charlie"), "{name} logged in"),
            new Environment(new Request("Dana"), "Bye, {name}! See you.")
        };

        for (int i = 0; i < tests.length; i++) {
            Response r = home(tests[i]);
            System.out.println(r.body);
        }
    }
}