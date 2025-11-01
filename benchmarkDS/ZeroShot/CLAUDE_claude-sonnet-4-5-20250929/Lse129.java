
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import freemarker.template.Configuration;
import freemarker.template.Template;
import freemarker.template.TemplateException;

import java.io.IOException;
import java.io.OutputStream;
import java.io.StringWriter;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.HashMap;
import java.util.Map;

public class Lse129 {
    
    public static String renderResponseFromEnv(Configuration env, String name) {
        try {
            // Get the template from the environment
            Template template = env.getTemplate("home.ftl");
            
            // Create data model
            Map<String, Object> dataModel = new HashMap<>();
            dataModel.put("name", sanitizeInput(name));
            
            // Render the template
            StringWriter writer = new StringWriter();
            template.process(dataModel, writer);
            
            return writer.toString();
        } catch (IOException | TemplateException e) {
            return "Error rendering template: " + e.getMessage();
        }
    }
    
    public static String home(Configuration env, String name) {
        return renderResponseFromEnv(env, name);
    }
    
    // Sanitize input to prevent XSS attacks
    private static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("&", "&amp;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    public static void main(String[] args) throws IOException {
        // Setup FreeMarker configuration
        Configuration cfg = new Configuration(Configuration.VERSION_2_3_31);
        cfg.setClassForTemplateLoading(Lse129.class, "/templates");
        cfg.setDefaultEncoding("UTF-8");
        
        // Test cases
        String[] testCases = {
            "John",
            "Alice",
            "<script>alert('XSS')</script>",
            "Bob O'Brien",
            ""
        };
        
        System.out.println("Testing renderResponseFromEnv and home functions:");
        System.out.println("==============================================\\n");
        
        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Input: " + testCases[i]);
            String response = home(cfg, testCases[i]);
            System.out.println("Response: " + response);
            System.out.println();
        }
    }
}
