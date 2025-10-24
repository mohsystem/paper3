#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

static const char* ALLOWED[] = {"echo","ls","pwd","whoami","date","uname","cat", NULL};
static const char* PROHIBITED = "<>|;&$`(){}[]";

static int is_allowed_cmd(const char* cmd){
    for (int i=0; ALLOWED[i]!=NULL; i++){
        if (strcmp(ALLOWED[i], cmd)==0) return 1;
    }
    return 0;
}

static char* strdup_s(const char* s){
    size_t n = strlen(s);
    char* p = (char*)malloc(n+1);
    if (!p) return NULL;
    memcpy(p, s, n+1);
    return p;
}

static void push_token(char*** arr, int* count, int* cap, const char* token, size_t len){
    if (*cap == 0){
        *cap = 8;
        *arr = (char**)malloc((*cap) * sizeof(char*));
    } else if (*count >= *cap){
        *cap *= 2;
        *arr = (char**)realloc(*arr, (*cap) * sizeof(char*));
    }
    char* t = (char*)malloc(len+1);
    memcpy(t, token, len);
    t[len] = '\0';
    (*arr)[(*count)++] = t;
}

static void free_tokens(char** tokens, int count){
    if (!tokens) return;
    for (int i=0;i<count;i++) free(tokens[i]);
    free(tokens);
}

static char** tokenize(const char* s, int* out_count, char* errbuf, size_t errsz){
    char** tokens = NULL;
    int count = 0, cap = 0;
    size_t n = strlen(s);
    char* buf = (char*)malloc(n + 1);
    size_t blen = 0;
    int inS = 0, inD = 0, esc = 0;

    for (size_t i=0;i<n;i++){
        char c = s[i];
        if (esc){
            buf[blen++] = c;
            esc = 0;
        } else if (inS){
            if (c=='\'') inS=0;
            else buf[blen++] = c;
        } else if (inD){
            if (c=='"') inD=0;
            else if (c=='\\') esc=1;
            else buf[blen++] = c;
        } else {
            if (c=='\\') esc=1;
            else if (c=='\'') inS=1;
            else if (c=='"') inD=1;
            else if (isspace((unsigned char)c)){
                if (blen > 0){
                    push_token(&tokens, &count, &cap, buf, blen);
                    blen = 0;
                }
            } else {
                buf[blen++] = c;
            }
        }
    }
    if (esc){
        snprintf(errbuf, errsz, "unfinished escape");
        free(buf);
        free_tokens(tokens, count);
        return NULL;
    }
    if (inS || inD){
        snprintf(errbuf, errsz, "unmatched quotes");
        free(buf);
        free_tokens(tokens, count);
        return NULL;
    }
    if (blen > 0){
        push_token(&tokens, &count, &cap, buf, blen);
    }
    free(buf);
    *out_count = count;
    return tokens;
}

static const char* validate_tokens(char** tokens, int count){
    static char errmsg[128];
    if (count == 0) return "empty input";
    if (count > 32) return "too many arguments";
    if (!is_allowed_cmd(tokens[0])) return "command not allowed";
    for (int i=0;i<count;i++){
        if (strpbrk(tokens[i], PROHIBITED) != NULL){
            return "contains prohibited characters";
        }
    }
    if (strcmp(tokens[0], "cat")==0 || strcmp(tokens[0], "ls")==0){
        for (int i=1;i<count;i++){
            const char* a = tokens[i];
            if (a[0] == '-') continue;
            if (a[0] == '/') return "absolute paths are not allowed";
            if (strstr(a, "..") != NULL) return "path traversal is not allowed";
            if (strchr(a, '/') != NULL) return "subdirectories not allowed in arguments";
        }
    }
    (void)errmsg;
    return NULL;
}

char* run_command(const char* command){
    int count = 0;
    char errbuf[128] = {0};
    char** tokens = tokenize(command, &count, errbuf, sizeof(errbuf));
    if (!tokens){
        size_t len = strlen(errbuf) + 18;
        char* res = (char*)malloc(len);
        snprintf(res, len, "Invalid command: %s", errbuf);
        return res;
    }
    const char* verr = validate_tokens(tokens, count);
    if (verr){
        size_t len = strlen(verr) + 18;
        char* res = (char*)malloc(len);
        snprintf(res, len, "Invalid command: %s", verr);
        free_tokens(tokens, count);
        return res;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1){
        char* res = strdup_s("Execution error: pipe failed");
        free_tokens(tokens, count);
        return res;
    }

    pid_t pid = fork();
    if (pid < 0){
        close(pipefd[0]); close(pipefd[1]);
        char* res = strdup_s("Execution error: fork failed");
        free_tokens(tokens, count);
        return res;
    } else if (pid == 0){
        // child
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        // build argv
        char** argv = (char**)malloc((count + 1) * sizeof(char*));
        for (int i=0;i<count;i++) argv[i] = tokens[i];
        argv[count] = NULL;

        execvp(argv[0], argv);
        // exec failed
        const char* msg = "exec failed\n";
        write(STDERR_FILENO, msg, strlen(msg));
        _exit(127);
    } else {
        // parent
        close(pipefd[1]);
        size_t cap = 4096, len = 0;
        char* out = (char*)malloc(cap);
        if (!out){ out = strdup_s("Execution error: no memory"); }
        char buf[4096];
        ssize_t n;
        while ((n = read(pipefd[0], buf, sizeof(buf))) > 0){
            if (len + (size_t)n >= cap){
                cap = (len + (size_t)n) * 2;
                out = (char*)realloc(out, cap);
                if (!out){
                    out = strdup_s("Execution error: no memory");
                    break;
                }
            }
            memcpy(out + len, buf, (size_t)n);
            len += (size_t)n;
        }
        close(pipefd[0]);
        int status=0;
        waitpid(pid, &status, 0);
        if (out){
            // null-terminate
            if (len >= cap){
                out = (char*)realloc(out, cap + 1);
                cap += 1;
            }
            out[len] = '\0';
        }
        free_tokens(tokens, count);
        return out;
    }
}

int main(){
    const char* tests[5] = {
        "echo Hello World",
        "pwd",
        "date",
        "uname -s",
        "ls -1"
    };
    for (int i=0;i<5;i++){
        printf("Command: %s\n", tests[i]);
        char* out = run_command(tests[i]);
        if (out){
            printf("%s", out);
            free(out);
        }
        printf("-----\n");
    }
    return 0;
}