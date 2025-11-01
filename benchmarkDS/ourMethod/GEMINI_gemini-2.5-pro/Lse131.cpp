#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

// Represents a user request
class UserRequest {
public:
    std::string getParameter(const std::string& key) const {
        auto it = parameters.find(key);
        if (it != parameters.end()) {
            return it->second;
        }
        return "";
    }

    void setParameter(const std::string& key, const std::string& value) {
        parameters[key] = value;
    }

private:
    std::map<std::string, std::string> parameters;
};

// Represents a response to be sent
class Response {
public:
    std::string getBody() const {
        return body;
    }

    void setBody(const std::string& b) {
        body = b;
    }

private:
    std::string body;
};

// Represents the environment containing request and template
class Environment {
public:
    Environment(const UserRequest& req, const std::string& tmpl)
        : request(req), template_str(tmpl) {}

    const UserRequest& getRequest() const {
        return request;
    }

    const std::string& getTemplate() const {
        return template_str;
    }

private:
    const UserRequest& request;
    const std::string& template_str;
};

/**
 * Sanitizes a string for HTML output to prevent XSS.
 *
 * @param input The string to escape.
 * @return The escaped string.
 */
std::string htmlEscape(const std::string& input) {
    std::stringstream escaped;
    for (char c : input) {
        switch (c) {
            case '&':  escaped << "&amp;";       break;
            case '<':  escaped << "&lt;";        break;
            case '>':  escaped << "&gt;";        break;
            case '"':  escaped << "&quot;";      break;
            case '\'': escaped << "&#x27;";      break;
            default:   escaped << c;             break;
        }
    }
    return escaped.str();
}


/**
 * Replaces the first occurrence of a substring.
 *
 * @param str The original string.
 * @param from The substring to be replaced.
 * @param to The string to replace with.
 * @return A new string with the replacement.
 */
std::string replace_first(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return str;
    str.replace(start_pos, from.length(), to);
    return str;
}


/**
 * Renders a response from a template and request found in the environment.
 *
 * @param env The environment containing the request and template.
 * @return A Response object with the rendered content.
 */
Response render_response_from_env(const Environment& env) {
    // Get the name parameter from the user request.
    std::string name = env.getRequest().getParameter("name");
    if (name.empty()) {
        name = "World";
    }

    // Sanitize the input to prevent XSS attacks.
    std::string sanitizedName = htmlEscape(name);

    // Get the template from the environment.
    std::string template_str = env.getTemplate();

    // Render the template with the sanitized name parameter.
    std::string renderedTemplate = replace_first(template_str, "{{name}}", sanitizedName);
    
    // Return a response with the rendered template.
    Response response;
    response.setBody(renderedTemplate);
    return response;
}

/**
 * Main handler that simulates a web request.
 *
 * @param env The environment for the request.
 * @return The generated response.
 */
Response home(const Environment& env) {
    return render_response_from_env(env);
}


int main() {
    std::string template_str = "<h1>Hello, {{name}}!</h1>";

    // Test Case 1: Normal name
    UserRequest request1;
    request1.setParameter("name", "Alice");
    Environment env1(request1, template_str);
    std::cout << "Test Case 1 (Normal): " << home(env1).getBody() << std::endl;

    // Test Case 2: XSS attack attempt
    UserRequest request2;
    request2.setParameter("name", "<script>alert('xss')</script>");
    Environment env2(request2, template_str);
    std::cout << "Test Case 2 (XSS): " << home(env2).getBody() << std::endl;

    // Test Case 3: Name with special HTML characters
    UserRequest request3;
    request3.setParameter("name", "Bob & \"Charlie\"");
    Environment env3(request3, template_str);
    std::cout << "Test Case 3 (Special Chars): " << home(env3).getBody() << std::endl;

    // Test Case 4: Empty name parameter
    UserRequest request4;
    request4.setParameter("name", "");
    Environment env4(request4, template_str);
    std::cout << "Test Case 4 (Empty Name): " << home(env4).getBody() << std::endl;

    // Test Case 5: Missing name parameter
    UserRequest request5;
    Environment env5(request5, template_str);
    std::cout << "Test Case 5 (Missing Name): " << home(env5).getBody() << std::endl;

    return 0;
}