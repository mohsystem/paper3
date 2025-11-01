
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.HashMap;
import java.util.Map;

public class Lse129 {
    static class Environment {
        private Map<String, String> templates;
        
        public Environment() {
            this.templates = new HashMap<>();
            this.templates.put("main", "Hello, {{name}}! Welcome to our service.");
        }
        
        public String getTemplate(String templateName) {
            return this.templates.getOrDefault(templateName, "Template not found");
        }
        
        public String render(String template, Map<String, String> params) {
            String result = template;
            for (Map.Entry<String, String> entry : params.entrySet()) {
                result = result.replace("{{" + entry.getKey() + "}}", entry.getValue());
            }
            return result;
        }
    }
    
    static class Request {
        private Map<String, String> params;
        
        public Request(String query) {
            this.params = new HashMap<>();
            if (query != null && !query.isEmpty()) {
                String[] pairs = query.split("&");
                for (String pair : pairs) {
                    String[] keyValue = pair.split("=");
                    if (keyValue.length == 2) {
                        this.params.put(keyValue[0], keyValue[1]);
                    }
                }
            }
        }
        
        public String getParameter(String key) {
            return this.params.getOrDefault(key, "Guest");
        }
    }
    
    static class Response {
        private String content;
        
        public Response(String content) {
            this.content = content;
        }
        
        public String getContent() {
            return this.content;
        }
    }
    
    public static Response renderResponseFromEnv(Environment env, Request request) {
        String name = request.getParameter("name");
        String template = env.getTemplate("main");
        Map<String, String> params = new HashMap<>();
        params.put("name", name);
        String renderedTemplate = env.render(template, params);
        return new Response(renderedTemplate);
    }
    
    public static Response home(Environment env, Request request) {
        return renderResponseFromEnv(env, request);
    }
    
    public static void main(String[] args) {
        Environment env = new Environment();
        
        // Test case 1: Name parameter provided
        Request req1 = new Request("name=John");
        Response resp1 = home(env, req1);
        System.out.println("Test 1: " + resp1.getContent());
        
        // Test case 2: Name parameter with different value
        Request req2 = new Request("name=Alice");
        Response resp2 = home(env, req2);
        System.out.println("Test 2: " + resp2.getContent());
        
        // Test case 3: No name parameter (should default to Guest)
        Request req3 = new Request("");
        Response resp3 = home(env, req3);
        System.out.println("Test 3: " + resp3.getContent());
        
        // Test case 4: Name parameter with another value
        Request req4 = new Request("name=Bob");
        Response resp4 = home(env, req4);
        System.out.println("Test 4: " + resp4.getContent());
        
        // Test case 5: Name parameter with special name
        Request req5 = new Request("name=Charlie");
        Response resp5 = home(env, req5);
        System.out.println("Test 5: " + resp5.getContent());
    }
}
