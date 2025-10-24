#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#define MAX_INPUT_LEN 200
#define MAX_TOKEN_LEN 128
#define MAX_OUTPUT_BYTES 8192
#define TIMEOUT_MS 5000

int is_allowed_cmd(const char *cmd){
    const char* allowed[] = {"echo","date","uname","whoami","pwd","ls"};
    for (size_t i=0;i<sizeof(allowed)/sizeof(allowed[0]);++i){
        if (strcmp(cmd, allowed[i])==0) return 1;
    }
    return 0;
}

int in_set(const char* tok, const char* const* set, size_t n){
    for (size_t i=0;i<n;i++){
        if (strcmp(tok, set[i])==0) return 1;
    }
    return 0;
}

int all_chars_allowed(const char* s){
    for (const unsigned char* p=(const unsigned char*)s; *p; ++p){
        unsigned char c=*p;
        if (isalnum(c)) continue;
        switch(c){
            case ' ': case '.': case '_': case '/': case '-':
            case ':': case '=': case '+': case ',':
                continue;
            default:
                return 0;
        }
    }
    return 1;
}

int safe_token(const char* t){
    if (!t || !*t) return 0;
    if (strlen(t) > MAX_TOKEN_LEN) return 0;
    for (const unsigned char* p=(const unsigned char*)t; *p; ++p){
        unsigned char c=*p;
        if (isalnum(c)) continue;
        switch(c){
            case '.': case '_': case '/': case '-':
            case ':': case '=': case '+': case ',':
                continue;
            default:
                return 0;
        }
    }
    return 1;
}

int safe_path(const char* p){
    if (!safe_token(p)) return 0;
    if (p[0]=='/') return 0;
    if (strstr(p, "..")!=NULL) return 0;
    return 1;
}

char* trim_copy(const char* s){
    if (!s) return NULL;
    size_t len = strlen(s);
    size_t i=0; while (i<len && isspace((unsigned char)s[i])) i++;
    if (i==len){
        char* out = (char*)malloc(1); out[0]='\0'; return out;
    }
    size_t j=len-1; while (j>i && isspace((unsigned char)s[j])) j--;
    size_t n = j - i + 1;
    char* out = (char*)malloc(n+1);
    memcpy(out, s+i, n);
    out[n]='\0';
    return out;
}

int split_ws(const char* s, char tokens[][MAX_TOKEN_LEN+1], int max_tokens){
    int count = 0;
    const char* p = s;
    while (*p){
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        // read token
        char tmp[MAX_TOKEN_LEN+1];
        int idx=0;
        while (*p && !isspace((unsigned char)*p)){
            if (idx < MAX_TOKEN_LEN){
                tmp[idx++] = *p;
            } else {
                // token too long
                return -1;
            }
            p++;
        }
        tmp[idx]='\0';
        if (count < max_tokens){
            strcpy(tokens[count++], tmp);
        } else {
            return -1;
        }
    }
    return count;
}

char* read_nonblock_with_timeout(int fd, pid_t childPid){
    char* out = (char*)malloc(MAX_OUTPUT_BYTES + 32);
    size_t out_len = 0;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1) fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    struct timespec ts_start;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    int childExited = 0;
    int status = 0;

    for (;;){
        char buf[512];
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0){
            size_t can_add = (out_len < MAX_OUTPUT_BYTES) ? (MAX_OUTPUT_BYTES - out_len) : 0;
            if (can_add > 0){
                size_t to_add = (size_t)n;
                if (to_add > can_add) to_add = can_add;
                memcpy(out + out_len, buf, to_add);
                out_len += to_add;
                if (out_len >= MAX_OUTPUT_BYTES){
                    const char *tr = "\n[truncated]";
                    size_t trl = strlen(tr);
                    if (out_len + trl > MAX_OUTPUT_BYTES + 31) trl = (MAX_OUTPUT_BYTES + 31) - out_len;
                    memcpy(out + out_len, tr, trl);
                    out_len += trl;
                }
            }
        } else if (n == 0){
            if (childExited) break;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK){
                break;
            }
        }

        if (!childExited){
            pid_t r = waitpid(childPid, &status, WNOHANG);
            if (r == childPid) childExited = 1;
        }

        struct timespec ts_now;
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        long elapsed_ms = (ts_now.tv_sec - ts_start.tv_sec)*1000L + (ts_now.tv_nsec - ts_start.tv_nsec)/1000000L;
        if (elapsed_ms > TIMEOUT_MS){
            if (!childExited){
                kill(childPid, SIGKILL);
                waitpid(childPid, &status, 0);
            }
            const char* msg = "ERROR: timeout";
            size_t ml = strlen(msg);
            if (out_len + ml + 1 > MAX_OUTPUT_BYTES + 32){
                ml = (MAX_OUTPUT_BYTES + 31) - out_len;
            }
            memcpy(out + out_len, msg, ml);
            out_len += ml;
            out[out_len] = '\0';
            return out;
        }

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 10 * 1000 * 1000; // 10ms
        nanosleep(&ts, NULL);

        if (childExited && n == 0){
            break;
        }
    }

    out[out_len] = '\0';
    return out;
}

char* runSafeCommand(const char* input){
    if (!input) {
        char* r = (char*)malloc(18); strcpy(r, "INVALID: empty"); return r;
    }
    char* s = trim_copy(input);
    if (!s || s[0]=='\0'){
        if (!s){ s = (char*)malloc(1); s[0]='\0'; }
        free(s);
        char* r = (char*)malloc(18); strcpy(r, "INVALID: empty"); return r;
    }
    if (strlen(s) > MAX_INPUT_LEN){
        free(s);
        char* r = (char*)malloc(19); strcpy(r, "INVALID: too long"); return r;
    }
    if (!all_chars_allowed(s)){
        free(s);
        char* r = (char*)malloc(37); strcpy(r, "INVALID: contains disallowed characters"); return r;
    }

    char tokens[32][MAX_TOKEN_LEN+1];
    int ntok = split_ws(s, tokens, 32);
    if (ntok <= 0){
        free(s);
        char* r = (char*)malloc(18); strcpy(r, "INVALID: empty"); return r;
    }
    if (!is_allowed_cmd(tokens[0])){
        free(s);
        char* r = (char*)malloc(30); strcpy(r, "INVALID: command not allowed"); return r;
    }

    const char* UNAME_OPTS[] = {"-a","-s","-r","-m","-n"};
    const char* LS_OPTS[] = {"-l","-a","-la","-al","-lah","-hal","-hla","-alh"};

    // Build argv
    const char* argv[34];
    int argc = 0;
    argv[argc++] = tokens[0];

    if (strcmp(tokens[0], "echo")==0){
        for (int i=1;i<ntok;i++){
            if (tokens[i][0]=='-'){
                free(s);
                char* r = (char*)malloc(29); strcpy(r, "INVALID: echo options not allowed"); return r;
            }
            if (!safe_token(tokens[i])){
                free(s);
                char* r = (char*)malloc(24); strcpy(r, "INVALID: unsafe token"); return r;
            }
            argv[argc++] = tokens[i];
        }
    } else if (strcmp(tokens[0], "date")==0){
        if (ntok > 1){
            free(s);
            char* r = (char*)malloc(31); strcpy(r, "INVALID: date takes no arguments"); return r;
        }
    } else if (strcmp(tokens[0], "uname")==0){
        if (ntok > 2){
            free(s);
            char* r = (char*)malloc(36); strcpy(r, "INVALID: too many args for uname"); return r;
        }
        if (ntok == 2){
            if (!in_set(tokens[1], UNAME_OPTS, sizeof(UNAME_OPTS)/sizeof(UNAME_OPTS[0]))){
                free(s);
                char* r = (char*)malloc(34); strcpy(r, "INVALID: uname option not allowed"); return r;
            }
            argv[argc++] = tokens[1];
        }
    } else if (strcmp(tokens[0], "whoami")==0 || strcmp(tokens[0], "pwd")==0){
        if (ntok > 1){
            free(s);
            char* r = (char*)malloc(36); strcpy(r, "INVALID: command takes no arguments"); return r;
        }
    } else if (strcmp(tokens[0], "ls")==0){
        int pathSeen = 0;
        for (int i=1;i<ntok;i++){
            if (tokens[i][0]=='-'){
                if (!in_set(tokens[i], LS_OPTS, sizeof(LS_OPTS)/sizeof(LS_OPTS[0]))){
                    free(s);
                    char* r = (char*)malloc(33); strcpy(r, "INVALID: ls option not allowed"); return r;
                }
                argv[argc++] = tokens[i];
            } else {
                if (pathSeen){
                    free(s);
                    char* r = (char*)malloc(31); strcpy(r, "INVALID: only one path allowed"); return r;
                }
                if (!safe_path(tokens[i])){
                    free(s);
                    char* r = (char*)malloc(24); strcpy(r, "INVALID: unsafe path"); return r;
                }
                argv[argc++] = tokens[i];
                pathSeen = 1;
            }
        }
    } else {
        free(s);
        char* r = (char*)malloc(30); strcpy(r, "INVALID: command not allowed"); return r;
    }
    argv[argc] = NULL;

    int pipefd[2];
    if (pipe(pipefd) != 0){
        free(s);
        char* r = (char*)malloc(23); strcpy(r, "ERROR: pipe failed"); return r;
    }

    pid_t pid = fork();
    if (pid < 0){
        close(pipefd[0]); close(pipefd[1]);
        free(s);
        char* r = (char*)malloc(22); strcpy(r, "ERROR: fork failed"); return r;
    }
    if (pid == 0){
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]); close(pipefd[1]);
        execvp(argv[0], (char* const*)argv);
        _exit(127);
    }

    close(pipefd[1]);
    char* out = read_nonblock_with_timeout(pipefd[0], pid);
    close(pipefd[0]);
    free(s);
    return out;
}

int main(void){
    const char* tests[] = {
        "echo hello",
        "date",
        "uname -a",
        "ls -l",
        "pwd"
    };
    for (size_t i=0;i<5;i++){
        char* res = runSafeCommand(tests[i]);
        printf("Command: %s\n", tests[i]);
        printf("Output:\n%s\n", res);
        printf("----\n");
        free(res);
    }
    return 0;
}