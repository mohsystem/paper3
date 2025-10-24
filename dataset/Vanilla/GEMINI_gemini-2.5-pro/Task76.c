#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Note: This implementation depends on the 'curl' command-line tool.
// It must be installed and available in the system's PATH to function.

#if defined(_WIN32) || defined(_WIN64)
    #define POPEN _popen
    #define PCLOSE _pclose
    // A simple implementation of strdup for Windows C compilers not supporting it by default
    char* strdup(const char* s) {
        size_t len = strlen(s) + 1;
        void* new_mem = malloc(len);
        if (new_mem == NULL) return NULL;
        return (char*)memcpy(new_mem, s, len);
    }
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

/**
 * Case-insensitive search for a substring.
 * @param haystack The string to search in.
 * @param needle The substring to search for.
 * @return A pointer to the beginning of the substring, or NULL if not found.
 */
char* stristr(const char* haystack, const char* needle) {
    if (!needle || !*needle) return (char*)haystack;
    for (; *haystack; ++haystack) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            const char* h = haystack;
            const char* n = needle;
            for (; *h && *n; ++h, ++n) {
                if (tolower((unsigned char)*h) != tolower((unsigned char)*n)) break;
            }
            if (!*n) return (char*)haystack;
        }
    }
    return NULL;
}

/**
 * Fetches content via curl and extracts the page title. The caller must free the returned string.
 * @param url The HTTPS URL to fetch.
 * @return A dynamically allocated string with the title, or an error/not found message.
 */
char* getPageTitle(const char* url) {
    char command[1024];
    snprintf(command, sizeof(command), "curl -sL \"%s\"", url);

    FILE* pipe = POPEN(command, "r");
    if (!pipe) return strdup("Error: popen() failed.");

    size_t capacity = 8192, size = 0;
    char* content = (char*)malloc(capacity);
    if (!content) { PCLOSE(pipe); return strdup("Error: malloc failed."); }
    
    size_t bytes_read;
    while ((bytes_read = fread(content + size, 1, capacity - size -1, pipe)) > 0) {
        size += bytes_read;
        if (size + 1 >= capacity) {
            capacity *= 2;
            char* new_content = (char*)realloc(content, capacity);
            if (!new_content) { free(content); PCLOSE(pipe); return strdup("Error: realloc failed."); }
            content = new_content;
        }
    }
    content[size] = '\0';
    PCLOSE(pipe);
    
    if (size == 0) {
        free(content);
        return strdup("Error: Failed to retrieve page content (is curl installed?).");
    }

    char* title = NULL;
    char* title_start_tag = stristr(content, "<title");
    if (title_start_tag) {
        char* title_start = strchr(title_start_tag, '>');
        if (title_start) {
            title_start++;
            char* title_end_tag = stristr(title_start, "</title>");
            if (title_end_tag) {
                size_t title_len = title_end_tag - title_start;
                title = (char*)malloc(title_len + 1);
                if (title) {
                    strncpy(title, title_start, title_len);
                    title[title_len] = '\0';

                    char* start = title;
                    while (isspace((unsigned char)*start)) start++;
                    char* end = start + strlen(start);
                    while (end > start && isspace((unsigned char)*(end - 1))) end--;
                    *end = '\0';
                    
                    if (start != title) memmove(title, start, strlen(start) + 1);
                }
            }
        }
    }
    
    free(content);
    return title ? title : strdup("Title not found.");
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("Processing URL from command line: %s\n", argv[1]);
        char* title = getPageTitle(argv[1]);
        if (title) {
            printf("Title: %s\n", title);
            free(title);
        }
        printf("\n--- Running built-in test cases ---\n");
    }

    const char* testUrls[] = {
        "https://www.google.com",
        "https://github.com",
        "https://www.oracle.com/java/",
        "https://www.python.org",
        "https://isocpp.org"
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Fetching title for: %s\n", testUrls[i]);
        char* title = getPageTitle(testUrls[i]);
        if (title) {
            printf("-> Title: %s\n\n", title);
            free(title);
        } else {
            printf("-> Failed to get title.\n\n");
        }
    }

    return 0;
}