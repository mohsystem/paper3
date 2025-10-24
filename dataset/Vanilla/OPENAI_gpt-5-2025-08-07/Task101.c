#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path, mode) mkdir(path, mode)
#endif

static int mkdir_p(const char *path) {
    if (path == NULL || *path == '\0') return 1;

    char *tmp = strdup(path);
    if (!tmp) return 0;

    size_t len = strlen(tmp);
    if (len == 0) {
        free(tmp);
        return 1;
    }

    // Remove trailing slashes
    while (len > 1 && (tmp[len - 1] == '/' || tmp[len - 1] == '\\')) {
        tmp[len - 1] = '\0';
        len--;
    }

    char *p = tmp;
    // Skip leading slash for absolute paths
    if (p[0] == '/' || p[0] == '\\') p++;

    for (; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0';
            if (strlen(tmp) > 0) {
                if (MKDIR(tmp, 0755) != 0) {
                    if (errno != EEXIST) {
                        free(tmp);
                        return 0;
                    }
                }
            }
            *p = '/';
        }
    }
    // Create final directory
    if (MKDIR(tmp, 0755) != 0) {
        if (errno != EEXIST) {
            free(tmp);
            return 0;
        }
    }

    free(tmp);
    return 1;
}

int create_executable_script(const char* directory, const char* content_input) {
    const char *default_content = "#!/bin/bash\necho \"Hello from script.sh\"\n";
    const char *content = (content_input && content_input[0] != '\0') ? content_input : default_content;

    const char *dir = (directory == NULL || directory[0] == '\0' || (directory[0] == '.' && directory[1] == '\0'))
                      ? "."
                      : directory;

    // Build file path dir + "/script.sh"
    size_t dir_len = strlen(dir);
    int needs_sep = (dir_len > 0 && dir[dir_len - 1] != '/' && dir[dir_len - 1] != '\\') ? 1 : 0;

    size_t path_len = dir_len + (needs_sep ? 1 : 0) + strlen("script.sh") + 1;
    char *filepath = (char*)malloc(path_len);
    if (!filepath) return 0;

    if (needs_sep) {
        snprintf(filepath, path_len, "%s/%s", dir, "script.sh");
    } else {
        snprintf(filepath, path_len, "%s%s", dir, "script.sh");
    }

    // Ensure parent directories exist
    // Extract parent directory from filepath
    char *last_sep = strrchr(filepath, '/');
#ifdef _WIN32
    char *last_sep2 = strrchr(filepath, '\\');
    if (!last_sep || (last_sep2 && last_sep2 > last_sep)) last_sep = last_sep2;
#endif
    if (last_sep) {
        size_t parent_len = (size_t)(last_sep - filepath);
        if (parent_len > 0) {
            char *parent = (char*)malloc(parent_len + 1);
            if (!parent) {
                free(filepath);
                return 0;
            }
            memcpy(parent, filepath, parent_len);
            parent[parent_len] = '\0';
            if (!mkdir_p(parent)) {
                free(parent);
                free(filepath);
                return 0;
            }
            free(parent);
        }
    }

    FILE *f = fopen(filepath, "wb");
    if (!f) {
        free(filepath);
        return 0;
    }
    size_t written = fwrite(content, 1, strlen(content), f);
    fclose(f);
    if (written != strlen(content)) {
        free(filepath);
        return 0;
    }

#ifndef _WIN32
    if (chmod(filepath, 0755) != 0) {
        free(filepath);
        return 0;
    }
#else
    // On Windows, no executable bit concept; succeed if file is created.
#endif

    free(filepath);
    return 1;
}

int main(void) {
    struct { const char* dir; const char* content; } tests[5] = {
        {".", "#!/bin/bash\necho \"C test 1\""},
        {"c_out1", "#!/bin/bash\necho \"C test 2\""},
        {"c_out2/nested", "#!/bin/bash\necho \"C test 3\""},
        {"c out3/spaced", "#!/bin/bash\necho \"C test 4\""},
        {"c_out4/a/b", "#!/bin/bash\necho \"C test 5\""}
    };

    for (int i = 0; i < 5; ++i) {
        int ok = create_executable_script(tests[i].dir, tests[i].content);
        printf("Test %d (%s): %s\n", i + 1, tests[i].dir, ok ? "success" : "failed");
    }
    return 0;
}