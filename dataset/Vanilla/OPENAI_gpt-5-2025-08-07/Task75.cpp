#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>

bool terminate_process(int pid) {
    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
    if (!hProc) return false;
    BOOL ok = TerminateProcess(hProc, 1);
    CloseHandle(hProc);
    return ok != 0;
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

bool terminate_process(pid_t pid) {
    if (kill(pid, SIGTERM) == -1) {
        if (errno == ESRCH) return true; // not existing
        if (errno == EPERM) return false;
    }
    for (int i = 0; i < 20; ++i) {
        if (kill(pid, 0) == -1 && errno == ESRCH) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    kill(pid, SIGKILL);
    for (int i = 0; i < 20; ++i) {
        if (kill(pid, 0) == -1 && errno == ESRCH) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return false;
}

int spawn_dummy_process() {
    pid_t pid = fork();
    if (pid == 0) {
        // child
        sleep(30);
        _exit(0);
    }
    return (int)pid;
}
#endif

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            try {
                int pid = std::stoi(argv[i]);
                bool ok = terminate_process(pid);
                std::cout << "PID " << pid << " terminated: " << (ok ? "true" : "false") << std::endl;
            } catch (...) {
                std::cout << "Invalid PID: " << argv[i] << std::endl;
            }
        }
        return 0;
    }

    // 5 test cases
    std::vector<int> pids;
    for (int i = 0; i < 5; ++i) {
        int pid = spawn_dummy_process();
        pids.push_back(pid);
        std::cout << "Spawned dummy process PID: " << pid << std::endl;
    }
    for (int pid : pids) {
        bool ok = terminate_process(pid);
        std::cout << "Test terminate PID " << pid << ": " << (ok ? "true" : "false") << std::endl;
    }
    return 0;
}