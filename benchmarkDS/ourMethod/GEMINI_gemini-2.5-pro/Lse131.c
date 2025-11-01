#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structs to model the web environment
typedef struct {
    char* key;
    char* value;
} KeyValuePair;

typedef struct {
    KeyValuePair* params;
    size_t param_count;
} UserRequest;

typedef struct {
    char* body;
} Response;

typedef struct {
    UserRequest* request;
    const char* template_str;
} Environment;

// Forward declarations
Response render_response_from_env(const Environment* env);
Response home(const Environment* env);
char* html_escape(const char* input);
char* string_replace(const char* orig, const char* rep, const char* with);
const char* get_request_parameter(const UserRequest* req, const char* key);
void free_response(Response* res);


/**
 * Sanitizes a string for HTML output to prevent XSS.
 * The caller is responsible for freeing the returned string.
 */
char* html_escape(const char* input) {
    if (!input) return NULL;
    
    size_t new_len = 0;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&': new_len += 5; break; // &amp;
            case '<': new_len += 4; break; // &lt;
            case '>': new_len += 4; break; // &gt;
            case '"': new_len += 6; break; // &quot;
            case '\'': new_len += 6; break; // &#x27;
            default: new_len++; break;
        }
    }

    char* escaped = (char*)malloc(new_len + 1);
    if (!escaped) {
        perror("malloc failed");
        return NULL;
    }

    char* p = escaped;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&': memcpy(p, "&amp;", 5); p += 5; break;
            case '<': memcpy(p, "&lt;", 4); p += 4; break;
            case '>': memcpy(p, "&gt;", 4); p += 4; break;
            case '"': memcpy(p, "&quot;", 6); p += 6; break;
            case '\'': memcpy(p, "&#x27;", 6); p += 6; break;
            default: *p++ = input[i]; break;
        }
    }
    *p = '\0';
    return escaped;
}


/**
 * Replaces all occurrences of a substring with another string.
 * The caller is responsible for freeing the returned string.
 */
char* string_replace(const char* orig, const char* rep, const char* with) {
    if (!orig || !rep || !with) return NULL;

    size_t len_rep = strlen(rep);
    if (len_rep == 0) return NULL;

    const char* p = orig;
    int count = 0;
    while ((p = strstr(p, rep))) {
        count++;
        p += len_rep;
    }

    size_t len_with = strlen(with);
    size_t len_orig = strlen(orig);
    size_t new_len = len_orig + count * (len_with - len_rep);

    char* result = (char*)malloc(new_len + 1);
    if (!result) {
        perror("malloc failed");
        return NULL;
    }

    char* q = result;
    p = orig;
    while (count > 0) {
        const char* next_rep = strstr(p, rep);
        size_t len_front = next_rep - p;
        memcpy(q, p, len_front);
        q += len_front;
        memcpy(q, with, len_with);
        q += len_with;
        p = next_rep + len_rep;
        count--;
    }
    strcpy(q, p);
    return result;
}

/**
 * Gets a parameter from a request.
 */
const char* get_request_parameter(const UserRequest* req, const char* key) {
    if (!req || !key) return NULL;
    for (size_t i = 0; i < req->param_count; ++i) {
        if (strcmp(req->params[i].key, key) == 0) {
            return req->params[i].value;
        }
    }
    return NULL;
}


/**
 * Renders a response from a template and request found in the environment.
 */
Response render_response_from_env(const Environment* env) {
    Response res = { NULL };
    if (!env) return res;

    const char* name = get_request_parameter(env->request, "name");
    const char* name_to_render = (name && name[0] != '\0') ? name : "World";
    
    char* sanitized_name = html_escape(name_to_render);
    if (!sanitized_name) {
        return res;
    }

    res.body = string_replace(env->template_str, "{{name}}", sanitized_name);
    
    free(sanitized_name);
    return res;
}

/**
 * Main handler that simulates a web request.
 */
Response home(const Environment* env) {
    return render_response_from_env(env);
}

void free_response(Response* res) {
    if (res && res->body) {
        free(res->body);
        res->body = NULL;
    }
}

void run_test_case(const char* test_name, UserRequest* request, const char* template_str) {
    Environment env = { request, template_str };
    Response response = home(&env);
    printf("%s: %s\n", test_name, response.body ? response.body : "[NULL]");
    free_response(&response);
}

int main() {
    const char* template_str = "<h1>Hello, {{name}}!</h1>";

    // Test Case 1: Normal name
    KeyValuePair params1[] = {{"name", "Alice"}};
    UserRequest request1 = {params1, 1};
    run_test_case("Test Case 1 (Normal)", &request1, template_str);

    // Test Case 2: XSS attack attempt
    KeyValuePair params2[] = {{"name", "<script>alert('xss')</script>"}};
    UserRequest request2 = {params2, 1};
    run_test_case("Test Case 2 (XSS)", &request2, template_str);

    // Test Case 3: Name with special HTML characters
    KeyValuePair params3[] = {{"name", "Bob & \"Charlie\""}};
    UserRequest request3 = {params3, 1};
    run_test_case("Test Case 3 (Special Chars)", &request3, template_str);

    // Test Case 4: Empty name parameter
    KeyValuePair params4[] = {{"name", ""}};
    UserRequest request4 = {params4, 1};
    run_test_case("Test Case 4 (Empty Name)", &request4, template_str);

    // Test Case 5: Missing name parameter
    UserRequest request5 = {NULL, 0};
    run_test_case("Test Case 5 (Missing Name)", &request5, template_str);

    return 0;
}