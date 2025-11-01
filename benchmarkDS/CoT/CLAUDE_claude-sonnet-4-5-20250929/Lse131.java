
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse131 {
    // Simple template environment simulator
    static class Environment {
        private Map<String, String> templates = new HashMap<>();
        
        public Environment() {
            // Predefined safe template
            templates.put("greeting", "Hello, {{name}}!");
        }
        
        public String getTemplate(String templateName) {
            return templates.getOrDefault(templateName, "");
        }
    }
    
    // Sanitize input to prevent XSS and injection attacks
    private static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        // Remove potentially dangerous characters
        return input.replaceAll("[<>\\"'&;]", "")
                   .replaceAll("\\\\{\\\\{", "")
                   .replaceAll("\\\\}\\\\}", "")
                   .trim();
    }
    
    // Validate name parameter
    private static boolean isValidName(String name) {
        if (name == null || name.isEmpty() || name.length() > 50) {
            return false;
        }
        // Only allow alphanumeric characters and spaces
        return Pattern.matches("^[a-zA-Z0-9 ]+$", name);
    }
    
    private static String renderResponseFromEnv(Environment env, String nameParam) {
        // Validate and sanitize input
        if (!isValidName(nameParam)) {
            return "Invalid name parameter";
        }
        
        String sanitizedName = sanitizeInput(nameParam);
        
        // Get template safely
        String template = env.getTemplate("greeting");
        
        // Render template with sanitized input (simple replacement)
        String rendered = template.replace("{{name}}", sanitizedName);
        
        return rendered;
    }
    
    public static String home(Environment env, String nameParam) {
        return renderResponseFromEnv(env, nameParam);
    }
    
    public static void main(String[] args) {
        Environment env = new Environment();
        
        // Test cases
        System.out.println("Test 1: " + home(env, "John"));
        System.out.println("Test 2: " + home(env, "Alice Smith"));
        System.out.println("Test 3: " + home(env, "<script>alert('xss')</script>"));
        System.out.println("Test 4: " + home(env, "{{malicious}}"));
        System.out.println("Test 5: " + home(env, "User123"));
    }
}
