class Environment {
    private final String template;

    public Environment(String template) {
        this.template = template;
    }

    public String getTemplate() {
        return this.template;
    }
}

public class Lse131 {

    /**
     * Renders a template from the environment with a given name.
     * @param env The environment containing the template.
     * @param name The name to insert into the template.
     * @return A response string with the rendered template.
     */
    public String render_response_from_env(Environment env, String name) {
        // The function gets the template from the environment.
        String template = env.getTemplate();
        // The function renders the template with the name parameter from the request.
        String renderedTemplate = template.replace("{name}", name);
        // The function returns a response with the rendered template.
        return renderedTemplate;
    }

    /**
     * Main entry point for the "home" route.
     * @param env The environment.
     * @param name The name from the request.
     * @return The response from the rendering function.
     */
    public String home(Environment env, String name) {
        // The home function calls the render_response_from_env function with the env variable.
        // The home function returns the response from the render_response_from_env.
        return render_response_from_env(env, name);
    }

    public static void main(String[] args) {
        // The render_response_from_env function takes an environment as an argument.
        Environment env = new Environment("Hello, {name}!");
        Lse131 app = new Lse131();

        String[] testCases = {"Alice", "Bob", "Charlie", "World", ""};
        
        System.out.println("Java Test Cases:");
        for (int i = 0; i < testCases.length; i++) {
            // The function gets the name parameter from the user request. (Simulated)
            String name = testCases[i];
            String response = app.home(env, name);
            System.out.printf("Test Case %d (Name: '%s'): %s\n", i + 1, name, response);
        }
    }
}