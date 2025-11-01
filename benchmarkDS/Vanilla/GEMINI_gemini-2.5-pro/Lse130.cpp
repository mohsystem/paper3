#include <iostream>
#include <string>
#include <map>
#include <stdexcept>

// A simple representation of a Request
struct Request {
    std::map<std::string, std::string> params;
};

// A simple representation of a Response
struct Response {
    std::string body;
};

// A simple representation of an Environment
struct Environment {
    Request request;
    std::string template_str;
};

// Helper function to replace all occurrences of a substring
std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles cases where 'to' is a substring of 'from'
    }
    return str;
}


/**
 * The render_response_from_env function takes an environment as an argument
 * and gets the name parameter from the request. The function also gets the
 * template from the environment. The function renders the template with the
 * name parameter. The function returns a response with the rendered template.
 * @param env A constant reference to the Environment struct.
 * @return A Response struct with the rendered content.
 */
Response render_response_from_env(const Environment& env) {
    std::string name;
    try {
        name = env.request.params.at("name");
    } catch (const std::out_of_range& oor) {
        name = ""; // Default to empty string if 'name' is not found
    }
    
    std::string template_str = env.template_str;
    
    std::string rendered_template = replace_all(template_str, "{name}", name);
    
    Response res;
    res.body = rendered_template;
    return res;
}

/**
 * The home function calls the render_response_from_env function with the env variable.
 * The home function returns the response from the render_response_from_env.
 * @param env A constant reference to the Environment struct.
 * @return A Response struct.
 */
Response home(const Environment& env) {
    return render_response_from_env(env);
}

// The Lse130 class is not idiomatic in C++, so we use the main function directly.
int main() {
    // Test Case 1
    Environment env1;
    env1.request.params["name"] = "Alice";
    env1.template_str = "Hello, {name}!";
    Response res1 = home(env1);
    std::cout << "Test Case 1: Response{body='" << res1.body << "'}" << std::endl;

    // Test Case 2
    Environment env2;
    env2.request.params["name"] = "Bob";
    env2.template_str = "Welcome, {name}. How are you?";
    Response res2 = home(env2);
    std::cout << "Test Case 2: Response{body='" << res2.body << "'}" << std::endl;

    // Test Case 3
    Environment env3;
    env3.request.params["name"] = "Charlie";
    env3.template_str = "Goodbye, {name}.";
    Response res3 = home(env3);
    std::cout << "Test Case 3: Response{body='" << res3.body << "'}" << std::endl;

    // Test Case 4: No name parameter
    Environment env4;
    // env4.request.params does not contain "name"
    env4.template_str = "Hello, {name}!";
    Response res4 = home(env4);
    std::cout << "Test Case 4: Response{body='" << res4.body << "'}" << std::endl;

    // Test Case 5: Multiple placeholders
    Environment env5;
    env5.request.params["name"] = "David";
    env5.template_str = "{name}, we are happy to see you, {name}.";
    Response res5 = home(env5);
    std::cout << "Test Case 5: Response{body='" << res5.body << "'}" << std::endl;

    return 0;
}