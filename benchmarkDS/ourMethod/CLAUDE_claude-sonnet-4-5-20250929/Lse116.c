
#include <stdio.h>
#include <stdlib.h>

/* WARNING: Using system() is inherently dangerous and should be avoided.
 * This example uses a hardcoded command only. Never pass user input to system().
 * For production code, use safer alternatives like fork/exec or platform-specific APIs.
 */

void executeCommand(void) {
    /* system() executes via shell and is vulnerable to command injection
     * Only safe when the command is completely hardcoded with no user input
     */
    const char* command = "ls";
    
    /* Execute the hardcoded command */
    int result = system(command);
    
    if (result == -1) {
        fprintf(stderr, "Error: Failed to execute command\\n");
    } else {
        printf("Command exited with code: %d\\n", WEXITSTATUS(result));
    }
}

int main(void) {
    /* Test cases - all execute the same hardcoded safe command */
    printf("Test 1:\\n");
    executeCommand();
    
    printf("\\nTest 2:\\n");
    executeCommand();
    
    printf("\\nTest 3:\\n");
    executeCommand();
    
    printf("\\nTest 4:\\n");
    executeCommand();
    
    printf("\\nTest 5:\\n");
    executeCommand();
    
    return 0;
}
