#include <iostream>
#include <string>
#include <map>
#include <utility>

// Simulates an environment that holds templates
struct Environment {
    std::string template_str;

    explicit Environment(std::string tpl) : template_str(std::move(tpl)) {}

    std::string getTemplate() const {
        return template_str;
    }
};

// Simulates a user request containing parameters
class UserRequest {
private:
    std::map<std::string, std::string> parameters;

public:
    void setParameter(const std::string& key, const std::string& value) {
        parameters[key] = value;
    }

    std::string getParameter(const std::string& key) const {
        auto it = parameters.find(key);
        if (it != parameters.end()) {
            return it->second;
        }
        return "Guest"; // Default value
    }
};

// Simulates a response to be sent back
struct Response {
    std::string body;
    explicit Response(std::string b) : body(std::move(b)) {}
};

// A helper function to replace all occurrences of a substring.
// This is a secure way to do literal replacement.
std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
    }
    return str;
}


/**
 * @brief Renders a response from a template.
 * @param env The environment containing the template.
 * @param request The user request containing parameters.
 * @return A Response object with the rendered template.
 */
Response render_response_from_env(const Environment& env, const UserRequest& request) {
    // 1. Get the name parameter from the user request.
    // In a real web app, input should be sanitized to prevent XSS.
    std::string name = request.getParameter("name");

    // 2. Get the template from the environment.
    std::string tpl = env.getTemplate();

    // 3. Render the template with the name parameter.
    std::string renderedTemplate = replace_all(tpl, "{{name}}", name);

    // 4. Return a response with the rendered template.
    return Response(renderedTemplate);
}

/**
 * @brief Simulates a home route handler.
 * @param env The environment object.
 * @param request The request object.
 * @return The response from the rendering function.
 */
Response home(const Environment& env, const UserRequest& request) {
    return render_response_from_env(env, request);
}

int main() {
    // Setup a shared environment for all requests
    Environment env("Hello, {{name}}! Welcome to our site.");

    // --- Test Cases ---

    // Test Case 1: Standard name
    UserRequest request1;
    request1.setParameter("name", "Alice");
    Response response1 = home(env, request1);
    std::cout << "Test Case 1: " << response1.body << std::endl;

    // Test Case 2: Another name
    UserRequest request2;
    request2.setParameter("name", "Bob");
    Response response2 = home(env, request2);
    std::cout << "Test Case 2: " << response2.body << std::endl;
    
    // Test Case 3: Name with numbers
    UserRequest request3;
    request3.setParameter("name", "Charlie7");
    Response response3 = home(env, request3);
    std::cout << "Test Case 3: " << response3.body << std::endl;
    
    // Test Case 4: No "name" parameter provided (defaults to "Guest")
    UserRequest request4;
    Response response4 = home(env, request4);
    std::cout << "Test Case 4: " << response4.body << std::endl;

    // Test Case 5: Name that could be an XSS attempt in a real web app
    UserRequest request5;
    request5.setParameter("name", "<script>alert('xss')</script>");
    Response response5 = home(env, request5);
    std::cout << "Test Case 5: " << response5.body << std::endl;

    return 0;
}