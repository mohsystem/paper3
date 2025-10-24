#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>

int terminate_process(int pid) {
    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (!hProc) return 0;
    BOOL ok = TerminateProcess(hProc, 1);
    CloseHandle(hProc);
    return ok ? 1 : 0;
}

int spawn_dummy_process() {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    char cmd[] = "cmd.exe /c ping -n 30 127.0.0.1 > nul";
    BOOL ok = CreateProcessA(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );
    if (!ok) return -1;
    DWORD pid = GetProcessId(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int)pid;
}

#else
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

static void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

int terminate_process(pid_t pid) {
    if (kill(pid, SIGTERM) == -1) {
        if (errno == ESRCH) return 1; // already gone
        if (errno == EPERM) return 0;
    }
    for (int i = 0; i < 20; ++i) {
        if (kill(pid, 0) == -1 && errno == ESRCH) return 1;
        sleep_ms(100);
    }
    kill(pid, SIGKILL);
    for (int i = 0; i < 20; ++i) {
        if (kill(pid, 0) == -1 && errno == ESRCH) return 1;
        sleep_ms(100);
    }
    return 0;
}

int spawn_dummy_process() {
    pid_t pid = fork();
    if (pid == 0) {
        sleep(30);
        _exit(0);
    }
    return (int)pid;
}
#endif

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            char* endptr = NULL;
            long val = strtol(argv[i], &endptr, 10);
            if (endptr == argv[i] || *endptr != '\0') {
                printf("Invalid PID: %s\n", argv[i]);
                continue;
            }
            int pid = (int)val;
            int ok = terminate_process(pid);
            printf("PID %d terminated: %s\n", pid, ok ? "true" : "false");
        }
        return 0;
    }

    // 5 test cases
    int pids[5];
    for (int i = 0; i < 5; ++i) {
        int pid = spawn_dummy_process();
        pids[i] = pid;
        printf("Spawned dummy process PID: %d\n", pid);
    }
    for (int i = 0; i < 5; ++i) {
        int ok = terminate_process(pids[i]);
        printf("Test terminate PID %d: %s\n", pids[i], ok ? "true" : "false");
    }
    return 0;
}