#include <iostream>
#include <string>
#include <stdexcept>

// Platform-specific includes
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#else
#include <signal.h>
#include <sys/types.h>
#include <cerrno> // For errno
#endif

// Define a cross-platform PID type
#ifdef _WIN32
typedef DWORD pid_type;
#else
typedef pid_t pid_type;
#endif

/**
 * @brief Terminates a process identified by the given PID.
 *
 * This function is cross-platform. It uses TerminateProcess on Windows
 * and kill(pid, SIGKILL) on POSIX-compliant systems.
 *
 * @param pid The process ID to terminate.
 * @return true if the termination signal was sent successfully, false otherwise.
 */
bool terminateProcess(pid_type pid) {
#ifdef _WIN32
    // Windows implementation
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        std::cerr << "Error: Could not open process with PID " << pid << ". It may not exist or you may lack permissions. GetLastError(): " << GetLastError() << std::endl;
        return false;
    }

    // Terminate the process. A non-zero exit code is conventional for forced termination.
    if (!TerminateProcess(hProcess, 1)) {
        std::cerr << "Error: Failed to terminate process with PID " << pid << ". GetLastError(): " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    std::cout << "Successfully requested termination for PID: " << pid << std::endl;
    CloseHandle(hProcess);
    return true;

#else
    // POSIX (Linux, macOS) implementation
    if (kill(pid, SIGKILL) == 0) {
        std::cout << "Successfully sent SIGKILL to PID: " << pid << std::endl;
        return true;
    } else {
        // kill() failed. Check errno for the reason.
        switch (errno) {
            case ESRCH:
                std::cerr << "Error: No process found with PID: " << pid << std::endl;
                break;
            case EPERM:
                std::cerr << "Error: Permission denied. Not allowed to send signal to PID: " << pid << std::endl;
                break;
            default:
                std::cerr << "Error: Failed to send signal to PID " << pid << ". errno: " << errno << std::endl;
                break;
        }
        return false;
    }
#endif
}

int main(int argc, char *argv[]) {
    /*
     --- Test Cases ---
     To run the test cases, compile this file (e.g., g++ -o terminate main.cpp) and run from the command line.
     You will need to find a valid PID on your system to test case 1.
     - On Linux/macOS, you can run `sleep 100 &` to get a background process and its PID.
     - On Windows, you can open Notepad and find its PID in Task Manager.

     1. Kill a valid process (e.g., a process with PID 1234):
        ./terminate 1234

     2. Attempt to kill a non-existent process:
        ./terminate 999999

     3. Attempt to kill a privileged process (e.g., PID 1 on Linux):
        ./terminate 1
        (This should fail with a permission error)

     4. Provide non-numeric input:
        ./terminate abc

     5. Provide an invalid PID number:
        ./terminate 0
    */
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <PID>" << std::endl;
        return 1;
    }

    try {
        std::string pid_str = argv[1];
        size_t pos;
        long long ll_pid = std::stoll(pid_str, &pos);

        // Ensure the entire string was a valid number
        if (pos != pid_str.length()) {
            throw std::invalid_argument("PID contains non-numeric characters.");
        }
        
        if (ll_pid <= 0) {
            std::cerr << "Error: PID must be a positive integer." << std::endl;
            return 1;
        }

        pid_type pid = static_cast<pid_type>(ll_pid);
        terminateProcess(pid);

    } catch (const std::invalid_argument&) {
        std::cerr << "Error: Invalid PID provided. Not a valid number." << std::endl;
        return 1;
    } catch (const std::out_of_range&) {
        std::cerr << "Error: PID provided is out of range." << std::endl;
        return 1;
    }

    return 0;
}