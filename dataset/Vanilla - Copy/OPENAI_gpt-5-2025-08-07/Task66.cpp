#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <cerrno>

static const std::set<std::string> ALLOWED = {"echo","ls","pwd","whoami","date","uname","cat"};
static const std::string PROHIBITED = "<>|;&$`(){}[]";

static std::vector<std::string> tokenize(const std::string& s, std::string& err) {
    std::vector<std::string> toks;
    std::string cur;
    bool inS=false, inD=false, esc=false;
    for (size_t i=0;i<s.size();++i){
        char c = s[i];
        if (esc){
            cur.push_back(c);
            esc=false;
        } else if (inS){
            if (c=='\'') inS=false;
            else cur.push_back(c);
        } else if (inD){
            if (c=='"') inD=false;
            else if (c=='\\') esc=true;
            else cur.push_back(c);
        } else {
            if (c=='\\') esc=true;
            else if (c=='\'') inS=true;
            else if (c=='"') inD=true;
            else if (std::isspace(static_cast<unsigned char>(c))){
                if (!cur.empty()){
                    toks.push_back(cur);
                    cur.clear();
                }
            } else {
                cur.push_back(c);
            }
        }
    }
    if (esc){ err="unfinished escape"; return {}; }
    if (inS || inD){ err="unmatched quotes"; return {}; }
    if (!cur.empty()) toks.push_back(cur);
    return toks;
}

static std::string validateTokens(const std::vector<std::string>& tokens){
    if (tokens.empty()) return "empty input";
    if (tokens.size() > 32) return "too many arguments";
    if (!ALLOWED.count(tokens[0])) return "command not allowed";
    for (auto& t : tokens){
        for (char ch : t){
            if (PROHIBITED.find(ch) != std::string::npos){
                return "contains prohibited characters";
            }
        }
    }
    if (tokens[0]=="cat" || tokens[0]=="ls"){
        for (size_t i=1;i<tokens.size();++i){
            const std::string& a = tokens[i];
            if (!a.empty() && a[0]=='-') continue;
            if (!a.empty() && a[0]=='/') return "absolute paths are not allowed";
            if (a.find("..") != std::string::npos) return "path traversal is not allowed";
            if (a.find('/') != std::string::npos) return "subdirectories not allowed in arguments";
        }
    }
    return "";
}

std::string run_command(const std::string& command){
    std::string err;
    auto tokens = tokenize(command, err);
    if (!err.empty()) return "Invalid command: " + err;
    std::string verr = validateTokens(tokens);
    if (!verr.empty()) return "Invalid command: " + verr;
    if (tokens.empty()) return "Invalid command: empty input";

    int pipefd[2];
    if (pipe(pipefd) == -1){
        return std::string("Execution error: pipe failed: ") + std::strerror(errno);
    }

    pid_t pid = fork();
    if (pid < 0){
        close(pipefd[0]); close(pipefd[1]);
        return std::string("Execution error: fork failed: ") + std::strerror(errno);
    } else if (pid == 0){
        // child
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        std::vector<char*> argv;
        argv.reserve(tokens.size()+1);
        for (auto& t : tokens){
            argv.push_back(const_cast<char*>(t.c_str()));
        }
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        // if exec fails
        const char* msg = "exec failed\n";
        write(STDERR_FILENO, msg, std::strlen(msg));
        _exit(127);
    } else {
        // parent
        close(pipefd[1]);
        std::string output;
        char buf[4096];
        ssize_t n;
        while ((n = read(pipefd[0], buf, sizeof(buf))) > 0){
            output.append(buf, buf + n);
        }
        close(pipefd[0]);
        int status=0;
        waitpid(pid, &status, 0);
        return output;
    }
}

int main(){
    std::vector<std::string> tests = {
        "echo Hello World",
        "pwd",
        "date",
        "uname -s",
        "ls -1"
    };
    for (auto& t : tests){
        std::cout << "Command: " << t << std::endl;
        std::cout << run_command(t);
        std::cout << "-----" << std::endl;
    }
    return 0;
}