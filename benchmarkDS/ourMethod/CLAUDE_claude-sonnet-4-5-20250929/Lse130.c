#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LENGTH 64
#define MAX_TEMPLATE_SIZE 512
#define MAX_RESPONSE_SIZE 1024

/* Environment structure to hold request and template data */
typedef struct {
    const char *request_name;
    const char *template_string;
} Environment;

/* Response structure */
typedef struct {
    char *content;
    size_t length;
} Response;

/* Validate name parameter - allow only alphanumeric and safe characters */
static int validate_name(const char *name, size_t max_len) {
    if (name == NULL) {
        return 0;
    }

    /* Check length - prevent excessive input */
    size_t len = strnlen(name, max_len + 1);
    if (len == 0 || len > max_len) {
        return 0;
    }

    /* Allow only alphanumeric, spaces, hyphens, and underscores - prevent injection */
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)name[i]) &&
            name[i] != ' ' &&
            name[i] != '-' &&
            name[i] != '_') {
            return 0;
        }
    }

    return 1;
}

/* Sanitize name by copying only safe characters to prevent template injection */
static int sanitize_name(const char *input, char *output, size_t output_size) {
    if (input == NULL || output == NULL || output_size == 0) {
        return 0;
    }

    size_t input_len = strnlen(input, MAX_NAME_LENGTH);
    size_t j = 0;

    /* Copy only validated characters - defense in depth */
    for (size_t i = 0; i < input_len && j < output_size - 1; i++) {
        if (isalnum((unsigned char)input[i]) ||
            input[i] == ' ' ||
            input[i] == '-' ||
            input[i] == '_') {
            output[j++] = input[i];
        }
    }

    /* Ensure null termination */
    output[j] = '\0';
    return 1;
}

/* Render response from environment - validates input and renders template */
Response *render_response_from_env(const Environment *env) {
    Response *response = NULL;
    char *content = NULL;
    char sanitized_name[MAX_NAME_LENGTH + 1];

    /* Validate environment pointer */
    if (env == NULL) {
        return NULL;
    }

    /* Initialize sanitized name buffer */
    memset(sanitized_name, 0, sizeof(sanitized_name));

    /* Get name parameter from request - treat as untrusted input */
    const char *name = env->request_name;

    /* Validate name parameter before use - reject invalid input */
    if (!validate_name(name, MAX_NAME_LENGTH)) {
        /* Return error response with generic message - don't leak details */
        response = (Response *)malloc(sizeof(Response));
        if (response == NULL) {
            return NULL;
        }

        content = (char *)malloc(MAX_RESPONSE_SIZE);
        if (content == NULL) {
            free(response);
            return NULL;
        }

        /* Use constant format string - prevent format string vulnerability */
        snprintf(content, MAX_RESPONSE_SIZE, "Error: Invalid input provided");

        response->content = content;
        response->length = strnlen(content, MAX_RESPONSE_SIZE);
        return response;
    }

    /* Sanitize name to prevent template injection - defense in depth */
    if (!sanitize_name(name, sanitized_name, sizeof(sanitized_name))) {
        return NULL;
    }

    /* Allocate response structure */
    response = (Response *)malloc(sizeof(Response));
    if (response == NULL) {
        return NULL;
    }

    /* Allocate buffer for rendered content - check allocation result */
    content = (char *)malloc(MAX_RESPONSE_SIZE);
    if (content == NULL) {
        free(response);
        return NULL;
    }

    /* Initialize content buffer */
    memset(content, 0, MAX_RESPONSE_SIZE);

    /* Get template from environment - validate template exists */
    const char *template_str = env->template_string;
    if (template_str == NULL) {
        snprintf(content, MAX_RESPONSE_SIZE, "Error: Template not found");
        response->content = content;
        response->length = strnlen(content, MAX_RESPONSE_SIZE);
        return response;
    }

    /* Render template with sanitized name parameter - use snprintf for bounds checking */
    int written = snprintf(content, MAX_RESPONSE_SIZE, template_str, sanitized_name);

    /* Check for truncation or error - verify snprintf result */
    if (written < 0 || written >= (int)MAX_RESPONSE_SIZE) {
        snprintf(content, MAX_RESPONSE_SIZE, "Error: Response too large");
        response->content = content;
        response->length = strnlen(content, MAX_RESPONSE_SIZE);
        return response;
    }

    /* Set response fields */
    response->content = content;
    response->length = (size_t)written;
    return response;
}

/* Home function - entry point that calls render_response_from_env */
Response *home(const Environment *env) {
    /* Validate input pointer */
    if (env == NULL) {
        return NULL;
    }

    /* Call render function and return result */
    return render_response_from_env(env);
}

/* Free response memory - ensures proper cleanup */
void free_response(Response *response) {
    if (response != NULL) {
        if (response->content != NULL) {
            /* Clear sensitive data before free if needed */
            memset(response->content, 0, response->length);
            free(response->content);
        }
        free(response);
    }
}

/* Main function with test cases */
int main(void) {
    Response *resp = NULL;

    /* Test case 1: Valid name */
    Environment env1 = {
        .request_name = "John_Doe",
        .template_string = "Hello, %s! Welcome to our site."
    };
    resp = home(&env1);
    if (resp != NULL) {
        printf("Test 1: %s\n", resp->content);
        free_response(resp);
    }

    /* Test case 2: Name with spaces */
    Environment env2 = {
        .request_name = "Jane Smith",
        .template_string = "Hello, %s! Welcome to our site."
    };
    resp = home(&env2);
    if (resp != NULL) {
        printf("Test 2: %s\n", resp->content);
        free_response(resp);
    }

    /* Test case 3: Invalid name with special characters */
    Environment env3 = {
        .request_name = "Alice<script>alert('xss')</script>",
        .template_string = "Hello, %s! Welcome to our site."
    };
    resp = home(&env3);
    if (resp != NULL) {
        printf("Test 3: %s\n", resp->content);
        free_response(resp);
    }

    /* Test case 4: Empty name */
    Environment env4 = {
        .request_name = "",
        .template_string = "Hello, %s! Welcome to our site."
    };
    resp = home(&env4);
    if (resp != NULL) {
        printf("Test 4: %s\n", resp->content);
        free_response(resp);
    }

    /* Test case 5: NULL name */
    Environment env5 = {
        .request_name = NULL,
        .template_string = "Hello, %s! Welcome to our site."
    };
    resp = home(&env5);
    if (resp != NULL) {
        printf("Test 5: %s\n", resp->content);
        free_response(resp);
    }

    return 0;
}
