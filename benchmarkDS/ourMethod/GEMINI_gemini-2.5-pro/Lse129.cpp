#include <iostream>
#include <string>
#include <vector>
#include <map>

/**
 * @brief Escapes HTML special characters in a string.
 * This is a security measure to prevent Cross-Site Scripting (XSS).
 * 
 * @param data The input string to escape.
 * @return std::string The HTML-escaped string.
 */
std::string htmlEscape(const std::string& data) {
    std::string escaped;
    escaped.reserve(data.length());
    for (char c : data) {
        switch (c) {
            case '&':  escaped.append("&amp;");       break;
            case '\"': escaped.append("&quot;");      break;
            case '\'': escaped.append("&#x27;");      break;
            case '<':  escaped.append("&lt;");        break;
            case '>':  escaped.append("&gt;");        break;
            default:   escaped.push_back(c);        break;
        }
    }
    return escaped;
}

/**
 * @brief Renders a response from a template and request data.
 * 
 * @param env A map representing the environment, containing templates.
 * @param request A map representing the incoming request parameters.
 * @return std::string A string representing the rendered response.
 */
std::string renderResponseFromEnv(
    const std::map<std::string, std::string>& env,
    const std::map<std::string, std::string>& request) {
    
    // Get the template from the environment
    auto template_it = env.find("template");
    if (template_it == env.end()) {
        return "Error: Template not found.";
    }
    std::string tpl = template_it->second;

    // Get the name parameter from the request, with a default value
    std::string name = "Guest";
    auto name_it = request.find("name");
    if (name_it != request.end()) {
        name = name_it->second;
    }

    // Sanitize input to prevent XSS
    std::string sanitizedName = htmlEscape(name);

    // Render the template with the sanitized parameter
    std::string placeholder = "{{name}}";
    size_t pos = tpl.find(placeholder);
    if (pos != std::string::npos) {
        tpl.replace(pos, placeholder.length(), sanitizedName);
    }
    
    return tpl;
}

/**
 * @brief A home function that simulates a controller action.
 * 
 * @param env A map representing the environment.
 * @param request A map representing the request.
 * @return std::string The response from the renderer.
 */
std::string home(
    const std::map<std::string, std::string>& env,
    const std::map<std::string, std::string>& request) {
    return renderResponseFromEnv(env, request);
}

int main() {
    // Setup a mock environment with a template
    std::map<std::string, std::string> env;
    env["template"] = "<h1>Hello, {{name}}!</h1>";

    std::cout << "--- 5 Test Cases ---" << std::endl;

    // Test Case 1: Normal name
    std::map<std::string, std::string> request1 = {{"name", "Alice"}};
    std::cout << "Test 1 (Normal): " << home(env, request1) << std::endl;

    // Test Case 2: Name with HTML characters (potential XSS)
    std::map<std::string, std::string> request2 = {{"name", "<script>alert('xss')</script>"}};
    std::cout << "Test 2 (XSS): " << home(env, request2) << std::endl;

    // Test Case 3: Empty name parameter
    std::map<std::string, std::string> request3 = {{"name", ""}};
    std::cout << "Test 3 (Empty): " << home(env, request3) << std::endl;

    // Test Case 4: Missing name parameter
    std::map<std::string, std::string> request4 = {};
    std::cout << "Test 4 (Missing): " << home(env, request4) << std::endl;

    // Test Case 5: Name with other special characters
    std::map<std::string, std::string> request5 = {{"name", "Bob & Charlie's \"Shop\""}};
    std::cout << "Test 5 (Special Chars): " << home(env, request5) << std::endl;

    return 0;
}