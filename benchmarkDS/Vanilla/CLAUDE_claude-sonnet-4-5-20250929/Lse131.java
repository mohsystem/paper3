
import java.util.*;
import java.io.*;

public class Lse131 {
    
    // Template class to simulate template rendering
    static class Template {
        private String content;
        
        public Template(String content) {
            this.content = content;
        }
        
        public String render(Map<String, String> context) {
            String result = content;
            for (Map.Entry<String, String> entry : context.entrySet()) {
                result = result.replace("{{" + entry.getKey() + "}}", entry.getValue());
            }
            return result;
        }
    }
    
    // Environment class to manage templates
    static class Environment {
        private Map<String, Template> templates;
        
        public Environment() {
            this.templates = new HashMap<>();
            // Default template
            templates.put("default", new Template("Hello, {{name}}!"));
        }
        
        public Template getTemplate(String templateName) {
            return templates.getOrDefault(templateName, templates.get("default"));
        }
        
        public void addTemplate(String name, String content) {
            templates.put(name, new Template(content));
        }
    }
    
    // Request class to simulate user request
    static class Request {
        private Map<String, String> parameters;
        
        public Request(Map<String, String> parameters) {
            this.parameters = parameters;
        }
        
        public String getParameter(String key) {
            return parameters.getOrDefault(key, "Guest");
        }
    }
    
    // Response class
    static class Response {
        private String body;
        
        public Response(String body) {
            this.body = body;
        }
        
        public String getBody() {
            return body;
        }
    }
    
    // Render response from environment
    public static Response renderResponseFromEnv(Environment env, Request request) {
        String name = request.getParameter("name");
        Template template = env.getTemplate("default");
        Map<String, String> context = new HashMap<>();
        context.put("name", name);
        String rendered = template.render(context);
        return new Response(rendered);
    }
    
    // Home function
    public static Response home(Environment env, Request request) {
        return renderResponseFromEnv(env, request);
    }
    
    public static void main(String[] args) {
        Environment env = new Environment();
        
        // Test case 1
        Map<String, String> params1 = new HashMap<>();
        params1.put("name", "John");
        Request request1 = new Request(params1);
        Response response1 = home(env, request1);
        System.out.println("Test 1: " + response1.getBody());
        
        // Test case 2
        Map<String, String> params2 = new HashMap<>();
        params2.put("name", "Alice");
        Request request2 = new Request(params2);
        Response response2 = home(env, request2);
        System.out.println("Test 2: " + response2.getBody());
        
        // Test case 3
        Map<String, String> params3 = new HashMap<>();
        params3.put("name", "Bob");
        Request request3 = new Request(params3);
        Response response3 = home(env, request3);
        System.out.println("Test 3: " + response3.getBody());
        
        // Test case 4 - No name parameter
        Map<String, String> params4 = new HashMap<>();
        Request request4 = new Request(params4);
        Response response4 = home(env, request4);
        System.out.println("Test 4: " + response4.getBody());
        
        // Test case 5
        Map<String, String> params5 = new HashMap<>();
        params5.put("name", "Charlie");
        Request request5 = new Request(params5);
        Response response5 = home(env, request5);
        System.out.println("Test 5: " + response5.getBody());
    }
}
