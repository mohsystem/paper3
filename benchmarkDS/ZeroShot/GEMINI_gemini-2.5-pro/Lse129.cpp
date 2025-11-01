#include <iostream>
#include <string>
#include <map>
#include <vector>

/**
 * @brief Escapes special HTML characters in a string to prevent Cross-Site Scripting (XSS).
 * 
 * @param input The string to escape.
 * @return The escaped string.
 */
std::string html_escape(const std::string& input) {
    std::string escaped_string;
    escaped_string.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '&':  escaped_string.append("&amp;");       break;
            case '<':  escaped_string.append("&lt;");        break;
            case '>':  escaped_string.append("&gt;");        break;
            case '"':  escaped_string.append("&quot;");      break;
            case '\'': escaped_string.append("&#39;");       break;
            default:   escaped_string.push_back(c);
        }
    }
    return escaped_string;
}

/**
 * @brief Renders a template from an environment with a given name parameter from a request.
 * 
 * @param env A map representing the environment, containing the template.
 * @param request_params A map representing request parameters, containing the name.
 * @return A string representing the rendered response.
 */
std::string render_response_from_env(
    const std::map<std::string, std::string>& env,
    const std::map<std::string, std::string>& request_params
) {
    std::string name = "";
    auto it_name = request_params.find("name");
    if (it_name != request_params.end()) {
        name = it_name->second;
    }

    std::string template_str = "";
    auto it_template = env.find("template");
    if (it_template != env.end()) {
        template_str = it_template->second;
    }

    // Secure: Escape the user-provided name to prevent XSS attacks.
    std::string escaped_name = html_escape(name);

    // Render the template by replacing the placeholder with the escaped name.
    std::string placeholder = "{{ name }}";
    size_t pos = template_str.find(placeholder);
    if (pos != std::string::npos) {
        return template_str.replace(pos, placeholder.length(), escaped_name);
    }
    
    return template_str;
}

/**
 * @brief Simulates a home route handler that renders a response.
 * 
 * @param env The environment map.
 * @param request_params The request parameters map.
 * @return The response from the renderer.
 */
std::string home(
    const std::map<std::string, std::string>& env,
    const std::map<std::string, std::string>& request_params
) {
    return render_response_from_env(env, request_params);
}

int main() {
    // 1. Setup the environment with a template.
    std::map<std::string, std::string> env = {
        {"template", "<h1>Hello, {{ name }}!</h1>"}
    };

    // 2. Define test cases.
    std::vector<std::map<std::string, std::string>> test_cases = {
        {{"name", "World"}},
        {{"name", "Jane Doe"}},
        {{"name", ""}},
        {{"name", "<script>alert('XSS')</script>"}},
        {{"name", "<b>Bold</b>"}}
    };
    
    // 3. Run test cases.
    int i = 1;
    for (const auto& request : test_cases) {
        std::string response = home(env, request);
        std::cout << "Test Case " << i++ << " (name=" << request.at("name") << "):" << std::endl;
        std::cout << "Output: " << response << std::endl << std::endl;
    }

    return 0;
}