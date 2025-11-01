
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <algorithm>
#include <ctime>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define MAX_FILE_SIZE 10485760  // 10MB
#define UPLOAD_DIR "uploads"

class Lse65 {
private:
    static bool isPDF(const char* data, size_t len) {
        if (len < 4) return false;
        return data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46;
    }
    
    static std::string sanitizeFilename(const std::string& filename) {
        std::string safe;
        for (char c : filename) {
            if (isalnum(c) || c == '.' || c == '_' || c == '-') {
                safe += c;
            } else {
                safe += '_';
            }
        }
        return safe;
    }
    
    static std::string extractFileData(const std::string& request) {
        size_t headerEnd = request.find("\\r\\n\\r\\n");
        if (headerEnd == std::string::npos) return "";
        
        size_t dataStart = headerEnd + 4;
        size_t boundaryPos = request.find("\\r\\n--", dataStart);
        if (boundaryPos == std::string::npos) return "";
        
        return request.substr(dataStart, boundaryPos - dataStart);
    }
    
public:
    static std::string handleUpload(const std::string& request) {
        std::string fileData = extractFileData(request);
        
        if (fileData.empty()) return "HTTP/1.1 400 Bad Request\\r\\n\\r\\nNo file data";
        if (fileData.size() > MAX_FILE_SIZE) return "HTTP/1.1 400 Bad Request\\r\\n\\r\\nFile too large";
        if (!isPDF(fileData.c_str(), fileData.size())) return "HTTP/1.1 400 Bad Request\\r\\n\\r\\nInvalid PDF";
        
        std::string filename = sanitizeFilename("upload_" + std::to_string(time(nullptr)) + ".pdf");
        std::string filepath = std::string(UPLOAD_DIR) + "/" + filename;
        
        std::ofstream outFile(filepath, std::ios::binary);
        if (!outFile) return "HTTP/1.1 500 Internal Server Error\\r\\n\\r\\nFailed to save file";
        
        outFile.write(fileData.c_str(), fileData.size());
        outFile.close();
        
        return "HTTP/1.1 200 OK\\r\\n\\r\\nFile uploaded: " + filename;
    }
    
    static std::string getFormHTML() {
        return "HTTP/1.1 200 OK\\r\\nContent-Type: text/html\\r\\n\\r\\n"
               "<!DOCTYPE html><html><body>"
               "<form action='/upload' method='post' enctype='multipart/form-data'>"
               "<input type='file' name='file' accept='.pdf'>"
               "<input type='submit' value='Upload PDF'></form></body></html>";
    }
};

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
    
    mkdir(UPLOAD_DIR, 0755);
    
    std::cout << "CPP server example - File upload handling implemented" << std::endl;
    std::cout << "Note: Full server implementation requires platform-specific socket code" << std::endl;
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    return 0;
}
