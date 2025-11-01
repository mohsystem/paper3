#include <iostream>
#include <string>
#include <map>

// Structs to represent the environment and request
struct Request {
    std::map<std::string, std::string> params;
};

struct Environment {
    Request request;
    std::string template_str;
};

/**
 * Renders a template from an Environment object.
 * @param env The Environment object containing the request and template.
 * @return The rendered string as a response.
 */
std::string render_response_from_env(const Environment& env) {
    // The function gets the name parameter from the request.
    std::string name = env.request.params.at("name");

    // The function gets the template from the environment.
    std::string tpl = env.template_str;

    // The function renders the template with the name parameter.
    std::string placeholder = "{{name}}";
    size_t pos = tpl.find(placeholder);
    if (pos != std::string::npos) {
        tpl.replace(pos, placeholder.length(), name);
    }

    // The function returns a response with the rendered template.
    return tpl;
}

/**
 * Calls the render_response_from_env function.
 * @param env The Environment object.
 * @return The response from the render function.
 */
std::string home(const Environment& env) {
    return render_response_from_env(env);
}

int main() {
    // 5 test cases
    Environment testCases[5];

    // Test Case 1
    testCases[0].request.params["name"] = "Alice";
    testCases[0].template_str = "Hello, {{name}}! Welcome to our site.";

    // Test Case 2
    testCases[1].request.params["name"] = "Bob";
    testCases[1].template_str = "Greetings, {{name}}!";

    // Test Case 3
    testCases[2].request.params["name"] = "Charlie";
    testCases[2].template_str = "Dear {{name}}, your profile is updated.";

    // Test Case 4: Name with spaces
    testCases[3].request.params["name"] = "David Smith";
    testCases[3].template_str = "User: {{name}}";

    // Test Case 5: Empty name
    testCases[4].request.params["name"] = "";
    testCases[4].template_str = "Hello, {{name}}! You are a guest.";

    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::string response = home(testCases[i]);
        std::cout << "  Input Env: { request: { name: \"" << testCases[i].request.params["name"] 
                  << "\" }, template: \"" << testCases[i].template_str << "\" }" << std::endl;
        std::cout << "  Response: " << response << std::endl << std::endl;
    }

    return 0;
}