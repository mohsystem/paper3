
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse129 {
    // Simple template environment that stores templates
    static class Environment {
        private final Map<String, String> templates = new HashMap<>();
        
        public void addTemplate(String name, String content) {
            if (name == null || content == null) {
                throw new IllegalArgumentException("Template name and content cannot be null");
            }
            templates.put(name, content);
        }
        
        public String getTemplate(String name) {
            return templates.get(name);
        }
    }
    
    // Simple template class with safe rendering
    static class Template {
        private final String content;
        private static final Pattern SAFE_NAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\\\-\\\\s]{1,100}$");
        
        public Template(String content) {
            this.content = content;
        }
        
        // Render template with name parameter, escaping HTML to prevent XSS
        public String render(String name) {
            // Validate name parameter - reject if null or doesn't match safe pattern\n            if (name == null || !SAFE_NAME_PATTERN.matcher(name).matches()) {\n                // Fail closed: use a safe default instead of user input\n                name = "Guest";\n            }\n            \n            // Escape HTML special characters to prevent XSS\n            String safeName = escapeHtml(name);\n            \n            // Simple template variable replacement\n            return content.replace("{{name}}", safeName);\n        }\n        \n        // Escape HTML special characters to prevent XSS attacks\n        private String escapeHtml(String input) {\n            if (input == null) {\n                return "";\n            }\n            return input.replace("&", "&amp;")\n                       .replace("<", "&lt;")\n                       .replace(">", "&gt;")\n                       .replace("\\"", "&quot;")\n                       .replace("'", "&#x27;")\n                       .replace("/", "&#x2F;");\n        }\n    }\n    \n    // Response class to encapsulate HTTP response\n    static class Response {\n        private final String body;\n        private final int statusCode;\n        \n        public Response(String body, int statusCode) {\n            this.body = body;\n            this.statusCode = statusCode;\n        }\n        \n        public String getBody() {\n            return body;\n        }\n        \n        public int getStatusCode() {\n            return statusCode;\n        }\n    }\n    \n    // Extract query parameter safely from URI\n    private static String getQueryParameter(URI uri, String paramName) {\n        if (uri == null || paramName == null) {\n            return null;\n        }\n        \n        String query = uri.getQuery();\n        if (query == null || query.isEmpty()) {\n            return null;\n        }\n        \n        // Parse query string safely\n        String[] pairs = query.split("&");\n        for (String pair : pairs) {\n            int idx = pair.indexOf("=");\n            if (idx > 0) {\n                try {\n                    String key = java.net.URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8);\n                    if (key.equals(paramName)) {\n                        String value = java.net.URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8);\n                        // Limit parameter length to prevent DoS\n                        if (value.length() > 1000) {\n                            return null;\n                        }\n                        return value;\n                    }\n                } catch (Exception e) {\n                    // Invalid encoding - return null\n                    return null;\n                }\n            }\n        }\n        return null;\n    }\n    \n    // Render response from environment with security controls\n    public static Response renderResponseFromEnv(Environment env, URI requestUri) {\n        if (env == null || requestUri == null) {\n            return new Response("Bad Request", 400);\n        }\n        \n        // Get name parameter from request with input validation\n        String name = getQueryParameter(requestUri, "name");\n        \n        // Get template from environment - use a fixed template name to prevent template injection\n        String templateContent = env.getTemplate("greeting");\n        if (templateContent == null) {\n            return new Response("Template not found", 500);\n        }\n        \n        // Create template and render with validated name\n        Template template = new Template(templateContent);\n        String rendered = template.render(name);\n        \n        // Return response with rendered content\n        return new Response(rendered, 200);\n    }\n    \n    // Home function that calls renderResponseFromEnv\n    public static Response home(Environment env, URI requestUri) {\n        // Delegate to renderResponseFromEnv\n        return renderResponseFromEnv(env, requestUri);\n    }\n    \n    // Test cases\n    public static void main(String[] args) throws IOException {\n        Environment env = new Environment();\n        env.addTemplate("greeting", "<html><body>Hello, {{name}}!</body></html>");\n        \n        System.out.println("Test Case 1: Valid name parameter");\n        Response resp1 = home(env, URI.create("http://localhost/?name=Alice"));\n        System.out.println("Status: " + resp1.getStatusCode());\n        System.out.println("Body: " + resp1.getBody());\n        System.out.println();\n        \n        System.out.println("Test Case 2: Name with spaces");\n        Response resp2 = home(env, URI.create("http://localhost/?name=John%20Doe"));\n        System.out.println("Status: " + resp2.getStatusCode());\n        System.out.println("Body: " + resp2.getBody());\n        System.out.println();\n        \n        System.out.println("Test Case 3: XSS attempt (should be escaped)");\n        Response resp3 = home(env, URI.create("http://localhost/?name=%3Cscript%3Ealert(1)%3C/script%3E"));\n        System.out.println("Status: " + resp3.getStatusCode());\n        System.out.println("Body: " + resp3.getBody());\n        System.out.println();\n        \n        System.out.println("Test Case 4: No name parameter (should use default)");\n        Response resp4 = home(env, URI.create("http://localhost/"));\n        System.out.println("Status: " + resp4.getStatusCode());\n        System.out.println("Body: " + resp4.getBody());\n        System.out.println();\n        \n        System.out.println("Test Case 5: Invalid characters (should use default)");\n        Response resp5 = home(env, URI.create("http://localhost/?name=%00%01%02"));\n        System.out.println("Status: " + resp5.getStatusCode());\n        System.out.println("Body: " + resp5.getBody());
    }
}
