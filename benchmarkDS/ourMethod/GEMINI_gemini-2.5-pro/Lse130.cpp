#include <iostream>
#include <string>
#include <map>
#include <vector>

// Mock structures to represent web components
struct Request {
    std::map<std::string, std::string> params;

    std::string getParameter(const std::string& key) const {
        auto it = params.find(key);
        if (it != params.end()) {
            return it->second;
        }
        return ""; // Return empty string if parameter not found
    }
};

struct Template {
    std::string content;
};

struct Response {
    std::string body;
};

struct Environment {
    const Request* request;
    const Template* tmpl;
};

/**
 * @brief Escapes characters in a string that are sensitive in HTML.
 * This is a crucial security measure to prevent Cross-Site Scripting (XSS).
 * @param data The string to escape.
 * @return The escaped string.
 */
std::string escape_html(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(const char c : data) {
        switch(c) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&#39;");       break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.push_back(c);          break;
        }
    }
    return buffer;
}


/**
 * @brief Gets name from request, gets template from env, renders, and returns response.
 * @param env The environment containing the request and template.
 * @return A Response object with the rendered template.
 */
Response render_response_from_env(const Environment& env) {
    if (env.request == nullptr || env.tmpl == nullptr) {
        return {"Error: Invalid environment."};
    }

    std::string name = env.request->getParameter("name");
    
    // Rules#1: Ensure all input is validated.
    if (name.empty()) {
        name = "Guest";
    }

    // Rules#2: Sanitize by escaping before embedding in the template to prevent XSS.
    std::string escaped_name = escape_html(name);
    
    std::string rendered_template = env.tmpl->content;
    
    std::string placeholder = "{{name}}";
    size_t pos = rendered_template.find(placeholder);
    if (pos != std::string::npos) {
        rendered_template.replace(pos, placeholder.length(), escaped_name);
    }
    
    return {rendered_template};
}

/**
 * @brief Home function that calls the render function.
 * @param env The environment variable.
 * @return The response from the render function.
 */
Response home(const Environment& env) {
    return render_response_from_env(env);
}

void run_test(const std::string& test_name, const Request& request, const Template& tmpl) {
    std::cout << test_name << std::endl;
    Environment env = {&request, &tmpl};
    Response res = home(env);
    std::cout << "  Output: " << res.body << std::endl << std::endl;
}

int main() {
    Template tmpl = {"<h1>Hello, {{name}}!</h1>"};

    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test 1: Normal name
    run_test("Test 1: Normal name", { {{"name", "Alice"}} }, tmpl);

    // Test 2: Name with special characters that need escaping
    run_test("Test 2: Name with special characters", { {{"name", "Bob & Charlie"}} }, tmpl);
    
    // Test 3: A potential XSS payload
    run_test("Test 3: XSS payload", { {{"name", "<script>alert('xss')</script>"}} }, tmpl);

    // Test 4: Empty name
    run_test("Test 4: Empty name", { {{"name", ""}} }, tmpl);

    // Test 5: No name parameter
    run_test("Test 5: No name parameter", { {} }, tmpl);

    return 0;
}