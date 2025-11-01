#include <iostream>
#include <string>
#include <vector>
#include <map>

class Lse129 {
public:
    // A simple structure to simulate a request's parameters
    struct Request {
        std::map<std::string, std::string> params;
    };

    // A simple structure to simulate the environment
    struct Environment {
        Request request;
        std::string template_str;
    };

    /**
     * @brief Escapes HTML special characters in a string to prevent XSS.
     * @param str The string to escape.
     * @return The escaped string.
     */
    static std::string escapeHtml(const std::string& str) {
        std::string escaped_str;
        escaped_str.reserve(str.length());
        for (char c : str) {
            switch (c) {
                case '&':  escaped_str += "&amp;";       break;
                case '<':  escaped_str += "&lt;";        break;
                case '>':  escaped_str += "&gt;";        break;
                case '"':  escaped_str += "&quot;";      break;
                case '\'': escaped_str += "&#39;";      break;
                default:   escaped_str += c;             break;
            }
        }
        return escaped_str;
    }

    /**
     * @brief Renders a template from an environment with a name parameter.
     * @param env The environment containing the request and template.
     * @return A response string with the rendered template.
     */
    static std::string render_response_from_env(const Environment& env) {
        std::string name = "Guest";
        
        // Get the name parameter from the request
        auto it = env.request.params.find("name");
        if (it != env.request.params.end()) {
            // Check if the provided name is not empty
            if (!it->second.empty()) {
               name = it->second;
            }
        }

        // Get the template from the environment
        std::string template_str = env.template_str;
        
        // SECURITY: Escape the user-provided name to prevent XSS attacks
        std::string safe_name = escapeHtml(name);

        // Render the template with the name parameter
        std::string placeholder = "{{ name }}";
        size_t pos = template_str.find(placeholder);
        if (pos != std::string::npos) {
            template_str.replace(pos, placeholder.length(), safe_name);
        }

        // Return a response with the rendered template
        return template_str;
    }

    /**
     * @brief Simulates a "home" endpoint controller.
     * @param env The environment for the request.
     * @return The response generated from the environment.
     */
    static std::string home(const Environment& env) {
        return render_response_from_env(env);
    }
    
    /**
     * @brief Main function to run test cases.
     */
    static void main() {
        // --- Test Cases ---
        std::string template_str = "<h1>Hello, {{ name }}!</h1>";

        // Test Case 1: Normal name
        Environment env1;
        env1.request.params["name"] = "Alice";
        env1.template_str = template_str;
        std::cout << "Test Case 1 (Normal): " << home(env1) << std::endl;

        // Test Case 2: XSS attack attempt
        Environment env2;
        env2.request.params["name"] = "<script>alert('XSS')</script>";
        env2.template_str = template_str;
        std::cout << "Test Case 2 (XSS): " << home(env2) << std::endl;

        // Test Case 3: Empty name parameter
        Environment env3;
        env3.request.params["name"] = "";
        env3.template_str = template_str;
        std::cout << "Test Case 3 (Empty): " << home(env3) << std::endl;

        // Test Case 4: Name with special characters
        Environment env4;
        env4.request.params["name"] = "Bob & Charlie";
        env4.template_str = template_str;
        std::cout << "Test Case 4 (Special Chars): " << home(env4) << std::endl;

        // Test Case 5: Missing 'name' key in params
        Environment env5;
        // 'name' key is missing
        env5.template_str = template_str;
        std::cout << "Test Case 5 (Missing): " << home(env5) << std::endl;
    }
};

int main() {
    Lse129::main();
    return 0;
}