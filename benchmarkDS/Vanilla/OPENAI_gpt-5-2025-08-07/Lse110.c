#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int run_id_command(const char* username) {
    pid_t pid = fork();
    if (pid < 0) {
        return 1;
    } else if (pid == 0) {
        execlp("id", "id", username, (char*)NULL);
        _exit(127);
    } else {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) return 1;
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
        return 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int status = run_id_command(argv[1]);
        printf("%d\n", status);
    } else {
        const char* tests[5] = {"root", "daemon", "nobody", "bin", "this_user_does_not_exist_12345"};
        for (int i = 0; i < 5; i++) {
            int status = run_id_command(tests[i]);
            printf("Exit status for '%s': %d\n", tests[i], status);
        }
    }
    return 0;
}