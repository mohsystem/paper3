#include <iostream>
#include <string>
#include <vector>

// A simple representation of an environment that holds a template
struct Environment {
    std::string template_str;
};

/**
 * Securely escapes HTML special characters to prevent XSS.
 * @param input The raw string from user input.
 * @return The escaped string, safe for HTML rendering.
 */
std::string escape_html(const std::string& input) {
    std::string escaped;
    escaped.reserve(input.size()); // Reserve space to avoid multiple reallocations
    for (char c : input) {
        switch (c) {
            case '&':  escaped += "&amp;";       break;
            case '<':  escaped += "&lt;";        break;
            case '>':  escaped += "&gt;";        break;
            case '"':  escaped += "&quot;";      break;
            case '\'': escaped += "&#39;";       break;
            default:   escaped += c;             break;
        }
    }
    return escaped;
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
std::string render_response_from_env(const Environment& env, const std::string& name) {
    // Get the template from the environment
    std::string tpl = env.template_str;
    
    // Securely escape the user-provided name parameter to prevent XSS
    std::string safe_name = escape_html(name);
    
    // Find the placeholder and replace it
    std::string placeholder = "{{name}}";
    size_t pos = tpl.find(placeholder);
    if (pos != std::string::npos) {
        return tpl.replace(pos, placeholder.length(), safe_name);
    }
    
    return tpl; // Return original if placeholder not found
}

/**
 * Simulates a "home" endpoint/controller function.
 * It calls the render_response_from_env function with the env variable.
 *
 * @param env The template environment.
 * @param name The name parameter from the user request.
 * @return The response from the render_response_from_env function.
 */
std::string home(const Environment& env, const std::string& name) {
    return render_response_from_env(env, name);
}

int main() {
    // The environment is created once, holding our template.
    Environment env = {"<h1>Hello, {{name}}!</h1>"};

    // --- Test Cases ---
    std::vector<std::string> test_names = {
        "World",                               // Test Case 1: Normal input
        "<script>alert('XSS');</script>",      // Test Case 2: Malicious script input
        "",                                    // Test Case 3: Empty input
        "John \"Johnny\" Doe & Sons' Store",   // Test Case 4: Input with special characters
        "A very long name with multiple spaces and characters to test rendering" // Test Case 5: Long input
    };

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (size_t i = 0; i < test_names.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << " (Input: \"" << test_names[i] << "\"):" << std::endl;
        std::string response = home(env, test_names[i]);
        std::cout << "Output: " << response << std::endl << std::endl;
    }

    return 0;
}