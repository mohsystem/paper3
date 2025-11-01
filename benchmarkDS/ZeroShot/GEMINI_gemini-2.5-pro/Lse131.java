import java.util.Map;
import java.util.HashMap;

// All code needs to be in one file.
// Main class name should be Lse131
public class Lse131 {

    // A simple representation of an environment that holds a template
    static class TemplateEnvironment {
        private final String template;

        public TemplateEnvironment(String template) {
            this.template = template;
        }

        public String getTemplate() {
            return this.template;
        }
    }

    /**
     * Securely escapes HTML special characters to prevent XSS.
     * @param input The raw string from user input.
     * @return The escaped string, safe for HTML rendering.
     */
    private static String escapeHtml(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder escaped = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '&':
                    escaped.append("&amp;");
                    break;
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\'':
                    escaped.append("&#39;");
                    break;
                default:
                    escaped.append(c);
            }
        }
        return escaped.toString();
    }

    /**
     * Renders a response from a template environment.
     * This function simulates getting a 'name' parameter from a user request.
     * It securely renders the template by escaping the user-provided name.
     *
     * @param env The template environment.
     * @param name The name parameter from the user request.
     * @return A response string with the rendered template.
     */
    public static String render_response_from_env(TemplateEnvironment env, String name) {
        // Get the template from the environment
        String template = env.getTemplate();
        
        // Securely escape the user-provided name parameter to prevent XSS
        String safeName = escapeHtml(name);
        
        // Render the template with the sanitized parameter
        // This is a simple substitution; real engines are more complex but should always escape
        return template.replace("{{name}}", safeName);
    }

    /**
     * Simulates a "home" endpoint/controller function.
     * It calls the render_response_from_env function with the env variable.
     *
     * @param env The template environment.
     * @param name The name parameter from the user request.
     * @return The response from the render_response_from_env function.
     */
    public static String home(TemplateEnvironment env, String name) {
        return render_response_from_env(env, name);
    }

    public static void main(String[] args) {
        // The environment is created once, holding our template.
        TemplateEnvironment env = new TemplateEnvironment("<h1>Hello, {{name}}!</h1>");

        // --- Test Cases ---
        String[] testNames = {
            "World", // Test Case 1: Normal input
            "<script>alert('XSS');</script>", // Test Case 2: Malicious script input
            "", // Test Case 3: Empty input
            "John \"Johnny\" Doe & Sons' Store", // Test Case 4: Input with special characters
            "A very long name with multiple spaces and characters to test rendering" // Test Case 5: Long input
        };

        System.out.println("--- Java Test Cases ---");
        for (int i = 0; i < testNames.length; i++) {
            System.out.println("Test Case " + (i + 1) + " (Input: \"" + testNames[i] + "\"):");
            String response = home(env, testNames[i]);
            System.out.println("Output: " + response);
            System.out.println();
        }
    }
}