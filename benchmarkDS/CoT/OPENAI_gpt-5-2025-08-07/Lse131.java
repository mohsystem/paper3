import java.util.*;

public class Lse131 {
    static class Request {
        String name;
        Request(String name) { this.name = name; }
    }

    static class Env {
        String template;
        Request request;
        Env(String template, Request request) {
            this.template = template;
            this.request = request;
        }
    }

    public static String render_response_from_env(Env env) {
        String template = (env != null && env.template != null) ? env.template : "";
        String name = (env != null && env.request != null && env.request.name != null) ? env.request.name : "";
        return template.replace("{name}", name);
    }

    public static String home(Env env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        Env e1 = new Env("Hello, {name}!", new Request("Alice"));
        Env e2 = new Env("Welcome {name} to the system.", new Request("Bob"));
        Env e3 = new Env("Goodbye, {name}.", new Request("Charlie"));
        Env e4 = new Env("{name}, your order is ready.", new Request(""));
        Env e5 = new Env("Hi {name}, hi again {name}!", new Request("Dana"));

        System.out.println(home(e1));
        System.out.println(home(e2));
        System.out.println(home(e3));
        System.out.println(home(e4));
        System.out.println(home(e5));
    }
}