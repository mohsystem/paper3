#include <iostream>
#include <string>
#include <vector>

class Lse131 {
public:
    class Environment {
    private:
        std::string template_str;

    public:
        Environment(const std::string& tpl) : template_str(tpl) {}

        std::string getTemplate() const {
            return template_str;
        }
    };

    /**
     * Renders a template from the environment with a given name.
     * @param env The environment containing the template.
     * @param name The name to insert into the template.
     * @return A response string with the rendered template.
     */
    std::string render_response_from_env(const Environment& env, const std::string& name) {
        // The function gets the template from the environment.
        std::string tpl = env.getTemplate();
        std::string placeholder = "{name}";

        // The function renders the template with the name parameter from the request.
        size_t pos = tpl.find(placeholder);
        if (pos != std::string::npos) {
            tpl.replace(pos, placeholder.length(), name);
        }
        
        // The function returns a response with the rendered template.
        return tpl;
    }

    /**
     * Main entry point for the "home" route.
     * @param env The environment.
     * @param name The name from the request.
     * @return The response from the rendering function.
     */
    std::string home(const Environment& env, const std::string& name) {
        // The home function calls the render_response_from_env function with the env variable.
        // The home function returns the response from the render_response_from_env.
        return render_response_from_env(env, name);
    }
};

int main() {
    Lse131 app;
    // The render_response_from_env function takes an environment as an argument.
    Lse131::Environment env("Hello, {name}!");

    std::vector<std::string> test_cases = {"Alice", "Bob", "Charlie", "World", ""};
    
    std::cout << "CPP Test Cases:" << std::endl;
    for (int i = 0; i < test_cases.size(); ++i) {
        // The function gets the name parameter from the user request. (Simulated)
        const std::string& name = test_cases[i];
        std::string response = app.home(env, name);
        std::cout << "Test Case " << i + 1 << " (Name: '" << name << "'): " << response << std::endl;
    }

    return 0;
}