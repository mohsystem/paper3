#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Executes 'getent passwd <username>' using system() call and captures the output.
 *
 * The command's output is redirected to a temporary file, which is then read into a
 * dynamically allocated string. The temporary file is deleted afterwards.
 *
 * @param username The username to look up.
 * @return A dynamically allocated string containing the command's output. The caller
 *         is responsible for freeing this memory. Returns NULL on failure or if no
 *         output is generated.
 */
char* getUserInfo(const char* username) {
    // Basic sanitization
    if (username == NULL || strpbrk(username, "`$\\;&|<>") != NULL) {
        return NULL;
    }

    char command[512];
    const char* temp_filename = "c_getent_output.tmp";

    // Construct the command, quoting the username to handle it as a single token
    snprintf(command, sizeof(command), "getent passwd \"%s\" > %s", username, temp_filename);

    // Execute the command
    system(command);

    FILE* fp = fopen(temp_filename, "r");
    if (fp == NULL) {
        remove(temp_filename);
        return NULL;
    }

    // Read file content into a buffer
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fsize <= 0) {
        fclose(fp);
        remove(temp_filename);
        return NULL;
    }

    char* content = (char*)malloc(fsize + 1);
    if (content == NULL) {
        fclose(fp);
        remove(temp_filename);
        return NULL;
    }

    fread(content, 1, fsize, fp);
    content[fsize] = '\0';

    fclose(fp);
    remove(temp_filename);

    return content;
}

int main() {
    const char* test_usernames[5];
    test_usernames[0] = "root";
    test_usernames[1] = "daemon";
    test_usernames[2] = "nonexistentuser12345";
    
    const char* currentUser = getenv("USER");
    if (currentUser != NULL) {
        test_usernames[3] = currentUser;
    } else {
        test_usernames[3] = "nobody"; // Fallback
    }

    test_usernames[4] = "";

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < 5; ++i) {
        printf("Testing for user: \"%s\"\n", test_usernames[i]);
        char* output = getUserInfo(test_usernames[i]);
        if (output == NULL || strlen(output) == 0) {
            printf("Output: <No output or user not found>\n");
        } else {
            printf("Output: %s", output);
        }
        
        // IMPORTANT: Free the memory allocated by getUserInfo
        free(output);
        output = NULL;

        printf("-------------------------\n");
    }

    return 0;
}