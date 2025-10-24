
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
    typedef int SOCKET;
#endif

// Security: Define maximum limits to prevent resource exhaustion
constexpr size_t MAX_REQUEST_SIZE = 65536;  // 64KB max request
constexpr size_t MAX_RESPONSE_SIZE = 65536; // 64KB max response
constexpr size_t MAX_METHOD_NAME_LEN = 256;
constexpr size_t MAX_PARAM_COUNT = 32;
constexpr int SERVER_PORT = 8080;
constexpr int MAX_PENDING_CONNECTIONS = 5;

// Security: Simple XML-RPC request parser with strict validation
// Prevents XXE attacks by not parsing external entities
class XMLRPCRequest {
private:
    std::string methodName;
    std::vector<std::string> params;
    
    // Security: Extract text between tags with bounds checking
    std::string extractTagContent(const std::string& xml, const std::string& tag, size_t startPos, bool& found) {
        found = false;
        std::string openTag = "<" + tag + ">";
        std::string closeTag = "</" + tag + ">";
        
        size_t openPos = xml.find(openTag, startPos);
        if (openPos == std::string::npos || openPos > xml.length() - openTag.length()) {
            return "";
        }
        
        size_t contentStart = openPos + openTag.length();
        size_t closePos = xml.find(closeTag, contentStart);
        
        if (closePos == std::string::npos || closePos > xml.length()) {
            return "";
        }
        
        // Security: Validate extraction boundaries
        if (contentStart > closePos || closePos - contentStart > MAX_REQUEST_SIZE) {
            return "";
        }
        
        found = true;
        return xml.substr(contentStart, closePos - contentStart);
    }
    
public:
    // Security: Parse XML-RPC request with strict validation
    // No external entity processing to prevent XXE (CWE-611)
    bool parse(const std::string& xmlRequest) {
        if (xmlRequest.empty() || xmlRequest.length() > MAX_REQUEST_SIZE) {
            return false;
        }
        
        // Security: Check for DOCTYPE declaration which could indicate XXE attempt
        if (xmlRequest.find("<!DOCTYPE") != std::string::npos ||
            xmlRequest.find("<!ENTITY") != std::string::npos) {
            return false; // Reject requests with DOCTYPE/ENTITY declarations
        }
        
        bool found = false;
        methodName = extractTagContent(xmlRequest, "methodName", 0, found);
        
        // Security: Validate method name length and characters
        if (!found || methodName.empty() || methodName.length() > MAX_METHOD_NAME_LEN) {
            return false;
        }
        
        // Security: Only allow alphanumeric and underscore in method names
        for (char c : methodName) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '.') {
                return false;
            }
        }
        
        // Parse parameters
        size_t paramsStart = xmlRequest.find("<params>");
        if (paramsStart != std::string::npos) {
            size_t pos = paramsStart;
            size_t paramCount = 0;
            
            while (paramCount < MAX_PARAM_COUNT) {
                bool valueFound = false;
                std::string value = extractTagContent(xmlRequest, "value", pos, valueFound);
                
                if (!valueFound) {
                    break;
                }
                
                // Security: Validate parameter value
                if (value.length() > MAX_REQUEST_SIZE / MAX_PARAM_COUNT) {
                    return false;
                }
                
                params.push_back(value);
                pos = xmlRequest.find("</value>", pos) + 8;
                paramCount++;
                
                if (pos >= xmlRequest.length()) {
                    break;
                }
            }
        }
        
        return true;
    }
    
    const std::string& getMethodName() const { return methodName; }
    const std::vector<std::string>& getParams() const { return params; }
};

// Security: XML-RPC response builder with safe string handling
class XMLRPCResponse {
public:
    // Security: Build response with proper XML escaping
    static std::string buildSuccess(const std::string& result) {
        // Security: Validate result size
        if (result.length() > MAX_RESPONSE_SIZE / 2) {
            return buildFault(1, "Result too large");
        }
        
        std::string escaped = xmlEscape(result);
        
        std::ostringstream oss;
        oss << "<?xml version=\\"1.0\\"?>\\r\\n"
            << "<methodResponse>\\r\\n"
            << "  <params>\\r\\n"
            << "    <param>\\r\\n"
            << "      <value><string>" << escaped << "</string></value>\\r\\n"
            << "    </param>\\r\\n"
            << "  </params>\\r\\n"
            << "</methodResponse>\\r\\n";
        
        return oss.str();
    }
    
    static std::string buildFault(int code, const std::string& message) {
        std::string escaped = xmlEscape(message);
        
        std::ostringstream oss;
        oss << "<?xml version=\\"1.0\\"?>\\r\\n"
            << "<methodResponse>\\r\\n"
            << "  <fault>\\r\\n"
            << "    <value>\\r\\n"
            << "      <struct>\\r\\n"
            << "        <member>\\r\\n"
            << "          <name>faultCode</name>\\r\\n"
            << "          <value><int>" << code << "</int></value>\\r\\n"
            << "        </member>\\r\\n"
            << "        <member>\\r\\n"
            << "          <name>faultString</name>\\r\\n"
            << "          <value><string>" << escaped << "</string></value>\\r\\n"
            << "        </member>\\r\\n"
            << "      </struct>\\r\\n"
            << "    </value>\\r\\n"
            << "  </fault>\\r\\n"
            << "</methodResponse>\\r\\n";
        
        return oss.str();
    }
    
private:
    // Security: Escape XML special characters to prevent injection
    static std::string xmlEscape(const std::string& input) {
        std::string output;
        output.reserve(input.length() * 2); // Reserve space to avoid frequent reallocations
        
        for (char c : input) {
            switch (c) {
                case '&':  output += "&amp;"; break;
                case '<':  output += "&lt;"; break;
                case '>':  output += "&gt;"; break;
                case '"':  output += "&quot;"; break;
                case '\\'': output += "&apos;"; break;
                default:
                    // Security: Only allow printable ASCII characters
                    if (c >= 32 && c <= 126) {
                        output += c;
                    }
                    break;
            }
        }
        
        return output;
    }
};

// Security: Process XML-RPC methods with input validation
std::string processMethod(const std::string& methodName, const std::vector<std::string>& params) {
    // Security: Whitelist allowed methods
    if (methodName == "add") {
        if (params.size() != 2) {
            return XMLRPCResponse::buildFault(2, "add requires exactly 2 parameters");
        }
        
        // Security: Validate numeric input to prevent injection
        try {
            int a = std::stoi(params[0]);
            int b = std::stoi(params[1]);
            
            // Security: Check for integer overflow
            if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) {
                return XMLRPCResponse::buildFault(3, "Integer overflow");
            }
            
            int result = a + b;
            return XMLRPCResponse::buildSuccess(std::to_string(result));
        } catch (const std::exception&) {
            return XMLRPCResponse::buildFault(4, "Invalid numeric parameters");
        }
    } else if (methodName == "echo") {
        if (params.empty()) {
            return XMLRPCResponse::buildFault(2, "echo requires at least 1 parameter");
        }
        return XMLRPCResponse::buildSuccess(params[0]);
    } else if (methodName == "getTime") {
        time_t now = time(nullptr);
        return XMLRPCResponse::buildSuccess(std::to_string(now));
    } else {
        return XMLRPCResponse::buildFault(1, "Method not found");
    }
}

// Security: Handle client connection with bounds checking
void handleClient(SOCKET clientSocket) {
    // Security: Allocate buffer with fixed size to prevent overflow
    std::vector<char> buffer(MAX_REQUEST_SIZE + 1, 0);
    
    // Security: Receive data with size limit
    int bytesReceived = recv(clientSocket, buffer.data(), MAX_REQUEST_SIZE, 0);
    
    if (bytesReceived <= 0 || bytesReceived > static_cast<int>(MAX_REQUEST_SIZE)) {
        closesocket(clientSocket);
        return;
    }
    
    // Security: Null-terminate received data
    buffer[bytesReceived] = '\\0';
    
    std::string request(buffer.data(), bytesReceived);
    
    // Security: Find HTTP body (XML content) after headers
    size_t bodyStart = request.find("\\r\\n\\r\\n");
    std::string xmlRequest;
    
    if (bodyStart != std::string::npos && bodyStart + 4 < request.length()) {
        xmlRequest = request.substr(bodyStart + 4);
    } else {
        xmlRequest = request;
    }
    
    // Parse and process request
    XMLRPCRequest rpcRequest;
    std::string response;
    
    if (rpcRequest.parse(xmlRequest)) {
        response = processMethod(rpcRequest.getMethodName(), rpcRequest.getParams());
    } else {
        response = XMLRPCResponse::buildFault(100, "Invalid XML-RPC request");
    }
    
    // Build HTTP response with security headers
    std::ostringstream httpResponse;
    httpResponse << "HTTP/1.1 200 OK\\r\\n"
                 << "Content-Type: text/xml; charset=utf-8\\r\\n"
                 << "Content-Length: " << response.length() << "\\r\\n"
                 << "X-Content-Type-Options: nosniff\\r\\n"
                 << "X-Frame-Options: DENY\\r\\n"
                 << "Connection: close\\r\\n"
                 << "\\r\\n"
                 << response;
    
    std::string httpResponseStr = httpResponse.str();
    
    // Security: Send with size validation
    if (httpResponseStr.length() <= MAX_RESPONSE_SIZE) {
        send(clientSocket, httpResponseStr.c_str(), static_cast<int>(httpResponseStr.length()), 0);
    }
    
    closesocket(clientSocket);
}

int main() {
    std::cout << "XML-RPC Server Test Cases\\n";
    std::cout << "=========================\\n\\n";
    
    // Test Case 1: Valid add method
    std::cout << "Test 1: Valid add method\\n";
    XMLRPCRequest req1;
    std::string xml1 = "<?xml version=\\"1.0\\"?><methodCall><methodName>add</methodName><params><param><value>5</value></param><param><value>3</value></param></params></methodCall>";
    if (req1.parse(xml1)) {
        std::string result1 = processMethod(req1.getMethodName(), req1.getParams());
        std::cout << "Result: " << (result1.find("<string>8</string>") != std::string::npos ? "PASS" : "FAIL") << "\\n\\n";
    }
    
    // Test Case 2: Echo method
    std::cout << "Test 2: Echo method\\n";
    XMLRPCRequest req2;
    std::string xml2 = "<?xml version=\\"1.0\\"?><methodCall><methodName>echo</methodName><params><param><value>Hello World</value></param></params></methodCall>";
    if (req2.parse(xml2)) {
        std::string result2 = processMethod(req2.getMethodName(), req2.getParams());
        std::cout << "Result: " << (result2.find("Hello World") != std::string::npos ? "PASS" : "FAIL") << "\\n\\n";
    }
    
    // Test Case 3: Invalid method
    std::cout << "Test 3: Invalid method\\n";
    XMLRPCRequest req3;
    std::string xml3 = "<?xml version=\\"1.0\\"?><methodCall><methodName>invalidMethod</methodName><params></params></methodCall>";
    if (req3.parse(xml3)) {
        std::string result3 = processMethod(req3.getMethodName(), req3.getParams());
        std::cout << "Result: " << (result3.find("Method not found") != std::string::npos ? "PASS" : "FAIL") << "\\n\\n";
    }
    
    // Test Case 4: XXE attack prevention
    std::cout << "Test 4: XXE attack prevention\\n";
    XMLRPCRequest req4;
    std::string xml4 = "<?xml version=\\"1.0\\"?><!DOCTYPE foo [<!ENTITY xxe SYSTEM \\"file:///etc/passwd\\">]><methodCall><methodName>echo</methodName><params><param><value>&xxe;</value></param></params></methodCall>";
    bool parsed4 = req4.parse(xml4);
    std::cout << "Result: " << (!parsed4 ? "PASS (rejected)" : "FAIL") << "\\n\\n";
    
    // Test Case 5: Integer overflow protection
    std::cout << "Test 5: Integer overflow protection\\n";
    XMLRPCRequest req5;
    std::string xml5 = "<?xml version=\\"1.0\\"?><methodCall><methodName>add</methodName><params><param><value>2147483647</value></param><param><value>1</value></param></params></methodCall>";
    if (req5.parse(xml5)) {
        std::string result5 = processMethod(req5.getMethodName(), req5.getParams());
        std::cout << "Result: " << (result5.find("overflow") != std::string::npos ? "PASS" : "FAIL") << "\\n\\n";
    }
    
    std::cout << "All test cases completed.\\n";
    std::cout << "Note: Full server requires socket initialization and listening.\\n";
    
    return 0;
}
