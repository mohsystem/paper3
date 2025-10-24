#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
using namespace std;

static const unordered_set<string> ALLOWED_CMDS = {"echo","date","uname","whoami","pwd","ls"};
static const unordered_set<string> UNAME_OPTS = {"-a","-s","-r","-m","-n"};
static const unordered_set<string> LS_OPTS = {"-l","-a","-la","-al","-lah","-hal","-hla","-alh"};
static const size_t MAX_INPUT_LEN = 200;
static const size_t MAX_TOKEN_LEN = 128;
static const size_t MAX_OUTPUT_BYTES = 8192;
static const int TIMEOUT_MS = 5000;

static bool allCharsAllowed(const string& s){
    for(char c: s){
        if (isalnum(static_cast<unsigned char>(c))) continue;
        switch(c){
            case ' ': case '.': case '_': case '/': case '-':
            case ':': case '=': case '+': case ',':
                continue;
            default:
                return false;
        }
    }
    return true;
}

static bool safeToken(const string& t){
    if (t.empty() || t.size() > MAX_TOKEN_LEN) return false;
    for(char c: t){
        if (isalnum(static_cast<unsigned char>(c))) continue;
        switch(c){
            case '.': case '_': case '/': case '-':
            case ':': case '=': case '+': case ',':
                continue;
            default:
                return false;
        }
    }
    return true;
}

static bool safePath(const string& p){
    if (!safeToken(p)) return false;
    if (!p.empty() && p[0] == '/') return false;
    if (p.find("..") != string::npos) return false;
    return true;
}

static vector<string> splitWS(const string& s){
    vector<string> out;
    string cur;
    istringstream iss(s);
    while (iss >> cur) out.push_back(cur);
    return out;
}

static string readFromFDNonBlocking(int fd, pid_t childPid){
    string out;
    out.reserve(1024);
    // set non-blocking
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags != -1) fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    auto start = chrono::steady_clock::now();
    bool childExited = false;
    int status = 0;

    while (true){
        char buf[512];
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0){
            size_t can_add = MAX_OUTPUT_BYTES - out.size();
            if (can_add == 0){
                // drain but stop collecting
            } else {
                size_t to_add = static_cast<size_t>(n);
                if (to_add > can_add) to_add = can_add;
                out.append(buf, buf + to_add);
                if (out.size() >= MAX_OUTPUT_BYTES){
                    out.append("\n[truncated]");
                }
            }
        } else if (n == 0){
            // EOF
            if (childExited) break;
            // Continue until child exit too
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK){
                break;
            }
            // no data for now
        }

        if (!childExited){
            pid_t r = waitpid(childPid, &status, WNOHANG);
            if (r == childPid) childExited = true;
        }

        auto now = chrono::steady_clock::now();
        auto elapsed_ms = chrono::duration_cast<chrono::milliseconds>(now - start).count();
        if (elapsed_ms > TIMEOUT_MS){
            if (!childExited){
                kill(childPid, SIGKILL);
                waitpid(childPid, &status, 0);
            }
            return "ERROR: timeout";
        }

        // small sleep to avoid busy loop
        this_thread::sleep_for(chrono::milliseconds(10));

        // If child exited and we have no more data available, attempt a final non-block read
        if (childExited && n == 0){
            break;
        }
    }

    return out;
}

string runSafeCommand(const string& input){
    if (input.empty()) return "INVALID: empty";
    string s = input;
    // trim
    auto l = s.find_first_not_of(" \t\r\n");
    auto r = s.find_last_not_of(" \t\r\n");
    if (l == string::npos) return "INVALID: empty";
    s = s.substr(l, r - l + 1);
    if (s.size() > MAX_INPUT_LEN) return "INVALID: too long";
    if (!allCharsAllowed(s)) return "INVALID: contains disallowed characters";

    vector<string> parts = splitWS(s);
    if (parts.empty()) return "INVALID: empty";
    string cmd = parts[0];
    if (!ALLOWED_CMDS.count(cmd)) return "INVALID: command not allowed";

    vector<string> args;
    args.push_back(cmd);
    if (cmd == "echo"){
        for (size_t i = 1; i < parts.size(); ++i){
            const string& t = parts[i];
            if (!t.empty() && t[0] == '-') return "INVALID: echo options not allowed";
            if (!safeToken(t)) return "INVALID: unsafe token";
            args.push_back(t);
        }
    } else if (cmd == "date"){
        if (parts.size() > 1) return "INVALID: date takes no arguments";
    } else if (cmd == "uname"){
        if (parts.size() > 2) return "INVALID: too many args for uname";
        if (parts.size() == 2){
            if (!UNAME_OPTS.count(parts[1])) return "INVALID: uname option not allowed";
            args.push_back(parts[1]);
        }
    } else if (cmd == "whoami" || cmd == "pwd"){
        if (parts.size() > 1) return "INVALID: command takes no arguments";
    } else if (cmd == "ls"){
        bool pathSeen = false;
        for (size_t i = 1; i < parts.size(); ++i){
            const string& t = parts[i];
            if (!t.empty() && t[0] == '-'){
                if (!LS_OPTS.count(t)) return "INVALID: ls option not allowed";
                args.push_back(t);
            } else {
                if (pathSeen) return "INVALID: only one path allowed";
                if (!safePath(t)) return "INVALID: unsafe path";
                args.push_back(t);
                pathSeen = true;
            }
        }
    } else {
        return "INVALID: command not allowed";
    }

    // Build argv
    vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (auto& a : args){
        argv.push_back(const_cast<char*>(a.c_str()));
    }
    argv.push_back(nullptr);

    int pipefd[2];
    if (pipe(pipefd) != 0){
        return string("ERROR: pipe failed");
    }

    pid_t pid = fork();
    if (pid < 0){
        close(pipefd[0]); close(pipefd[1]);
        return string("ERROR: fork failed");
    }
    if (pid == 0){
        // child
        // redirect stdout and stderr
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        // exec without shell
        execvp(argv[0], argv.data());
        _exit(127);
    }

    // parent
    close(pipefd[1]);
    string out = readFromFDNonBlocking(pipefd[0], pid);
    close(pipefd[0]);
    return out;
}

int main(){
    vector<string> tests = {
        "echo hello",
        "date",
        "uname -a",
        "ls -l",
        "pwd"
    };
    for (auto& t : tests){
        cout << "Command: " << t << "\n";
        cout << "Output:\n" << runSafeCommand(t) << "\n";
        cout << "----\n";
    }
    return 0;
}