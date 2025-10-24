
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <regex>
#include <sstream>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

class Task111 {
private:
    int port;
    std::map<std::string, std::function<std::string(std::vector<std::string>)>> methods;
    
public:
    Task111(int p = 8080) : port(p) {
        methods["add"] = [](std::vector<std::string> params) -> std::string {
            int a = std::stoi(params[0]);
            int b = std::stoi(params[1]);
            return std::to_string(a + b);
        };
        
        methods["subtract"] = [](std::vector<std::string> params) -> std::string {
            int a = std::stoi(params[0]);
            int b = std::stoi(params[1]);
            return std::to_string(a - b);
        };
        
        methods["multiply"] = [](std::vector<std::string> params) -> std::string {
            int a = std::stoi(params[0]);
            int b = std::stoi(params[1]);
            return std::to_string(a * b);
        };
        
        methods["greet"] = [](std::vector<std::string> params) -> std::string {
            return "Hello, " + params[0] + "!";
        };
        
        methods["concat"] = [](std::vector<std::string> params) -> std::string {
            return params[0] + params[1];
        };
    }
    
    std::string extractMethodName(const std::string& xml) {
        std::regex pattern("<methodName>(.*?)</methodName>");
        std::smatch match;
        if (std::regex_search(xml, match, pattern)) {
            return match[1].str();
        }
        return "";
    }
    
    std::vector<std::string> extractParams(const std::string& xml) {
        std::vector<std::string> params;
        std::regex paramPattern("<param>.*?<value>(.*?)</value>.*?</param>");
        
        auto params_begin = std::sregex_iterator(xml.begin(), xml.end(), paramPattern);
        auto params_end = std::sregex_iterator();
        
        for (std::sregex_iterator i = params_begin; i != params_end; ++i) {
            std::smatch match = *i;
            std::string valueContent = match[1].str();
            
            std::regex intPattern("<(?:int|i4)>(.*?)</(?:int|i4)>");
            std::smatch intMatch;
            if (std::regex_search(valueContent, intMatch, intPattern)) {
                params.push_back(intMatch[1].str());
                continue;
            }
            
            std::regex stringPattern("<string>(.*?)</string>");
            std::smatch stringMatch;
            if (std::regex_search(valueContent, stringMatch, stringPattern)) {
                params.push_back(stringMatch[1].str());
                continue;
            }
            
            params.push_back(valueContent);
        }
        
        return params;
    }
    
    std::string buildSuccessResponse(const std::string& result, bool isInt = false) {
        std::stringstream response;
        response << "<?xml version=\\"1.0\\"?>\\n";
        response << "<methodResponse>\\n";
        response << "  <params>\\n";
        response << "    <param>\\n";
        response << "      <value>";
        
        if (isInt) {
            response << "<int>" << result << "</int>";
        } else {
            try {
                std::stoi(result);
                response << "<int>" << result << "</int>";
            } catch (...) {
                response << "<string>" << result << "</string>";
            }
        }
        
        response << "</value>\\n";
        response << "    </param>\\n";
        response << "  </params>\\n";
        response << "</methodResponse>";
        
        return response.str();
    }
    
    std::string buildFaultResponse(int faultCode, const std::string& faultString) {
        std::stringstream response;
        response << "<?xml version=\\"1.0\\"?>\\n";
        response << "<methodResponse>\\n";
        response << "  <fault>\\n";
        response << "    <value>\\n";
        response << "      <struct>\\n";
        response << "        <member>\\n";
        response << "          <name>faultCode</name>\\n";
        response << "          <value><int>" << faultCode << "</int></value>\\n";
        response << "        </member>\\n";
        response << "        <member>\\n";
        response << "          <name>faultString</name>\\n";
        response << "          <value><string>" << faultString << "</string></value>\\n";
        response << "        </member>\\n";
        response << "      </struct>\\n";
        response << "    </value>\\n";
        response << "  </fault>\\n";
        response << "</methodResponse>";
        
        return response.str();
    }
    
    std::string processXMLRPC(const std::string& xmlRequest) {
        try {
            std::string methodName = extractMethodName(xmlRequest);
            std::vector<std::string> params = extractParams(xmlRequest);
            
            if (methods.find(methodName) != methods.end()) {
                std::string result = methods[methodName](params);
                return buildSuccessResponse(result);
            } else {
                return buildFaultResponse(-1, "Method not found: " + methodName);
            }
        } catch (const std::exception& e) {
            return buildFaultResponse(-2, "Error processing request: " + std::string(e.what()));
        }
    }
};

int main() {
    Task111 server;
    
    std::cout << "Test Case 1: Process add request" << std::endl;
    std::string request1 = "<?xml version=\\"1.0\\"?><methodCall><methodName>add</methodName><params><param><value><int>5</int></value></param><param><value><int>3</int></value></param></params></methodCall>";
    std::cout << server.processXMLRPC(request1) << std::endl;
    
    std::cout << "\\nTest Case 2: Process subtract request" << std::endl;
    std::string request2 = "<?xml version=\\"1.0\\"?><methodCall><methodName>subtract</methodName><params><param><value><int>10</int></value></param><param><value><int>4</int></value></param></params></methodCall>";
    std::cout << server.processXMLRPC(request2) << std::endl;
    
    std::cout << "\\nTest Case 3: Process multiply request" << std::endl;
    std::string request3 = "<?xml version=\\"1.0\\"?><methodCall><methodName>multiply</methodName><params><param><value><int>6</int></value></param><param><value><int>7</int></value></param></params></methodCall>";
    std::cout << server.processXMLRPC(request3) << std::endl;
    
    std::cout << "\\nTest Case 4: Process greet request" << std::endl;
    std::string request4 = "<?xml version=\\"1.0\\"?><methodCall><methodName>greet</methodName><params><param><value><string>Alice</string></value></param></params></methodCall>";
    std::cout << server.processXMLRPC(request4) << std::endl;
    
    std::cout << "\\nTest Case 5: Process invalid method request" << std::endl;
    std::string request5 = "<?xml version=\\"1.0\\"?><methodCall><methodName>invalidMethod</methodName><params></params></methodCall>";
    std::cout << server.processXMLRPC(request5) << std::endl;
    
    return 0;
}
