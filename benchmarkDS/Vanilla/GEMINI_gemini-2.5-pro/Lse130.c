#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A simple representation of a Request
typedef struct {
    char* name;
} Request;

// A simple representation of a Response
typedef struct {
    char* body;
} Response;

// A simple representation of an Environment
typedef struct {
    Request request;
    const char* template_str;
} Environment;

// Helper function to replace all occurrences of a substring
// Note: This function allocates memory for the new string, which must be freed by the caller.
char* str_replace(const char* orig, const char* rep, const char* with) {
    char* result; 
    char* ins;    
    char* tmp;   
    int len_rep;  
    int len_with; 
    int len_front;
    int count;   

    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; 
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = (char*)orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = (char*)malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;
    }
    strcpy(tmp, orig);
    return result;
}


/**
 * The render_response_from_env function takes an environment as an argument
 * and gets the name parameter from the request. The function also gets the
 * template from the environment. The function renders the template with the
 * name parameter. The function returns a response with the rendered template.
 * @param env A pointer to the Environment struct.
 * @return A Response struct with the rendered content.
 */
Response render_response_from_env(const Environment* env) {
    const char* name = env->request.name ? env->request.name : "";
    const char* template_str = env->template_str;
    
    Response res;
    res.body = str_replace(template_str, "{name}", name);
    
    return res;
}

/**
 * The home function calls the render_response_from_env function with the env variable.
 * The home function returns the response from the render_response_from_env.
 * @param env A pointer to the Environment struct.
 * @return A Response struct.
 */
Response home(const Environment* env) {
    return render_response_from_env(env);
}

// The Lse130 class concept is not applicable in C. We use a standard main function.
int main() {
    // Test Case 1
    Environment env1 = { {"Alice"}, "Hello, {name}!" };
    Response res1 = home(&env1);
    printf("Test Case 1: Response{body='%s'}\n", res1.body);
    free(res1.body);

    // Test Case 2
    Environment env2 = { {"Bob"}, "Welcome, {name}. How are you?" };
    Response res2 = home(&env2);
    printf("Test Case 2: Response{body='%s'}\n", res2.body);
    free(res2.body);

    // Test Case 3
    Environment env3 = { {"Charlie"}, "Goodbye, {name}." };
    Response res3 = home(&env3);
    printf("Test Case 3: Response{body='%s'}\n", res3.body);
    free(res3.body);

    // Test Case 4: No name parameter (NULL name)
    Environment env4 = { {NULL}, "Hello, {name}!" };
    Response res4 = home(&env4);
    printf("Test Case 4: Response{body='%s'}\n", res4.body);
    free(res4.body);
    
    // Test Case 5: Multiple placeholders
    Environment env5 = { {"David"}, "{name}, we are happy to see you, {name}." };
    Response res5 = home(&env5);
    printf("Test Case 5: Response{body='%s'}\n", res5.body);
    free(res5.body);

    return 0;
}