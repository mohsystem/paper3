#include <iostream>
#include <string>
#include <map>
#include <sstream>

// Simulating the environment and request structures
struct Request {
    std::map<std::string, std::string> params;
};

struct Environment {
    Request request;
    std::string template_str;
};

/**
 * Escapes characters in a string to be safe for use in HTML, preventing XSS.
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
            case '\'': escaped << "&#39;";       break;
            default:   escaped << c;
        }
    }
    return escaped.str();
}

/**
 * Replaces all occurrences of a substring in a string.
 * @param str The original string.
 * @param from The substring to replace.
 * @param to The string to replace with.
 * @return The modified string.
 */
std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}


/**
 * Renders a response from an environment object. It gets a name from the request,
 * securely renders it into a template, and returns an HTTP-like response.
 * @param env An Environment object.
 * @return A string representing the rendered response.
 */
std::string render_response_from_env(const Environment& env) {
    std::string name = "Guest"; // Default value
    
    // Get the name parameter from the request
    auto it = env.request.params.find("name");
    if (it != env.request.params.end()) {
        name = it->second;
    }

    // Get the template from the environment
    std::string template_str = env.template_str;
    
    // Securely escape the user-provided name to prevent XSS
    std::string safe_name = htmlEscape(name);

    // Render the template with the sanitized name
    std::string rendered_template = replaceAll(template_str, "{name}", safe_name);

    // Return a "response" with the rendered template
    return "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + rendered_template;
}

/**
 * Home function that calls render_response_from_env.
 * @param env An Environment object.
 * @return The response from render_response_from_env.
 */
std::string home(const Environment& env) {
    return render_response_from_env(env);
}

int main() {
    std::string template_str = "<h1>Hello, {name}!</h1>";

    // Test Case 1: Simple name
    Environment env1;
    env1.request.params["name"] = "Alice";
    env1.template_str = template_str;
    std::cout << "--- Test Case 1 ---" << std::endl;
    std::cout << home(env1) << std::endl << std::endl;

    // Test Case 2: Name with a space
    Environment env2;
    env2.request.params["name"] = "Bob Smith";
    env2.template_str = template_str;
    std::cout << "--- Test Case 2 ---" << std::endl;
    std::cout << home(env2) << std::endl << std::endl;

    // Test Case 3: Name with characters that need escaping
    Environment env3;
    env3.request.params["name"] = "O'Malley & Sons";
    env3.template_str = template_str;
    std::cout << "--- Test Case 3 ---" << std::endl;
    std::cout << home(env3) << std::endl << std::endl;

    // Test Case 4: XSS attack attempt
    Environment env4;
    env4.request.params["name"] = "<script>alert('pwned')</script>";
    env4.template_str = template_str;
    std::cout << "--- Test Case 4 (XSS Prevention) ---" << std::endl;
    std::cout << home(env4) << std::endl << std::endl;

    // Test Case 5: Missing name parameter
    Environment env5;
    // No "name" parameter here
    env5.template_str = template_str;
    std::cout << "--- Test Case 5 (Default Name) ---" << std::endl;
    std::cout << home(env5) << std::endl << std::endl;

    return 0;
}