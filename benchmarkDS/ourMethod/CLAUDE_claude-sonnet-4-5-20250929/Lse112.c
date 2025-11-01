
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>

/* Maximum allowed hostname length per RFC 1035 */
#define MAX_HOST_LENGTH 253
/* Maximum output buffer size to prevent memory exhaustion */
#define MAX_OUTPUT_SIZE 65536
/* Read buffer size */
#define BUFFER_SIZE 4096

/* Validates hostname format to prevent command injection */
/* Only allows alphanumeric characters, dots, and hyphens */
int isValidHost(const char* host) {
    size_t len;
    size_t i;
    char c;
    
    if (host == NULL) {
        return 0;
    }
    
    len = strlen(host);
    
    /* Check length to prevent buffer issues */
    if (len == 0 || len > MAX_HOST_LENGTH) {
        return 0;
    }
    
    /* Whitelist validation: only safe characters allowed */
    /* Rejects shell metacharacters like ; | & $ ( ) ` etc. */
    for (i = 0; i < len; i++) {
        c = host[i];
        if (!isalnum(c) && c != '.' && c != '-') {
            return 0;
        }
    }
    
    return 1;
}

/* Executes ping command safely without shell involvement */
/* Returns dynamically allocated string with result or NULL on error */
char* executePing(const char* host) {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    size_t totalBytes = 0;
    char* output = NULL;
    size_t outputSize = 0;
    size_t outputCapacity = 8192;
    int status;
    
    /* Create pipe for reading command output */
    if (pipe(pipefd) == -1) {
        return strdup("Error: Pipe creation failed");
    }
    
    /* Initialize output buffer */
    output = (char*)calloc(outputCapacity, sizeof(char));
    if (output == NULL) {
        close(pipefd[0]);
        close(pipefd[1]);
        return strdup("Error: Memory allocation failed");
    }
    
    /* Add header to output */
    snprintf(output, outputCapacity, "Ping result for %s:\\n", host);
    outputSize = strlen(output);
    
    pid = fork();
    
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        free(output);
        return strdup("Error: Fork failed");
    }
    
    if (pid == 0) {
        /* Child process: execute ping */
        close(pipefd[0]); /* Close read end */
        
        /* Redirect stdout and stderr to pipe */
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        
        /* Execute ping with separate arguments - no shell involved */
        /* This prevents command injection as execl does not invoke a shell */
        /* Arguments are passed directly to the ping binary */
        execl("/bin/ping", "ping", "-c", "4", host, (char*)NULL);
        
        /* If execl fails, exit child process */
        _exit(1);
    } else {
        /* Parent process: read output */
        close(pipefd[1]); /* Close write end */
        
        /* Read output with size limit to prevent memory exhaustion */
        while ((bytesRead = read(pipefd[0], buffer, BUFFER_SIZE)) > 0) {
            totalBytes += bytesRead;
            
            /* Check size limit */
            if (totalBytes > MAX_OUTPUT_SIZE) {
                close(pipefd[0]);
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0);
                free(output);
                return strdup("Error: Output too large");
            }
            
            /* Grow buffer if needed */
            if (outputSize + bytesRead >= outputCapacity) {
                size_t newCapacity = outputCapacity * 2;
                char* newOutput = (char*)realloc(output, newCapacity);
                if (newOutput == NULL) {
                    close(pipefd[0]);
                    kill(pid, SIGKILL);
                    waitpid(pid, NULL, 0);
                    free(output);
                    return strdup("Error: Memory reallocation failed");
                }
                output = newOutput;
                outputCapacity = newCapacity;
            }
            
            /* Append to output buffer */
            memcpy(output + outputSize, buffer, bytesRead);
            outputSize += bytesRead;
            output[outputSize] = '\\0'; /* Ensure null termination */
        }
        
        close(pipefd[0]);
        
        /* Wait for child process to complete */
        waitpid(pid, &status, 0);
        
        return output;
    }
}

/* Simulated ping handler for demonstration */
char* pingHandler(const char* url) {
    if (url == NULL || strlen(url) == 0) {
        /* Input validation: reject empty input */
        return strdup("Error: Missing 'url' parameter");
    }
    
    if (!isValidHost(url)) {
        /* Input validation: reject invalid hostnames to prevent command injection */
        return strdup("Error: Invalid URL format");
    }
    
    /* Execute ping with validated input */
    return executePing(url);
}

int main(void) {
    char* result = NULL;
    
    printf("Ping service simulation\\n");
    printf("Test cases:\\n\\n");
    
    /* Test case 1: Valid hostname */
    printf("Test 1 - localhost:\\n");
    result = pingHandler("localhost");
    if (result != NULL) {
        printf("%s\\n\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 2: Valid IP address */
    printf("Test 2 - 127.0.0.1:\\n");
    result = pingHandler("127.0.0.1");
    if (result != NULL) {
        printf("%s\\n\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 3: Valid domain */
    printf("Test 3 - google.com:\\n");
    result = pingHandler("google.com");
    if (result != NULL) {
        printf("%s\\n\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 4: Empty input */
    printf("Test 4 - empty string:\\n");
    result = pingHandler("");
    if (result != NULL) {
        printf("%s\\n\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 5: Command injection attempt (blocked) */
    printf("Test 5 - injection attempt (localhost;ls):\\n");
    result = pingHandler("localhost;ls");
    if (result != NULL) {
        printf("%s\\n\\n", result);
        free(result);
        result = NULL;
    }
    
    return 0;
}
