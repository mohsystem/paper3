
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_PARAMS 10
#define MAX_PARAM_LEN 256
#define MAX_RESPONSE_LEN 2048

typedef struct {
    char params[MAX_PARAMS][MAX_PARAM_LEN];
    int param_count;
} ParamList;

int extract_between_tags(const char* xml, const char* start_tag, const char* end_tag, char* result) {
    char* start = strstr(xml, start_tag);
    if (!start) return 0;
    
    start += strlen(start_tag);
    char* end = strstr(start, end_tag);
    if (!end) return 0;
    
    int len = end - start;
    strncpy(result, start, len);
    result[len] = '\\0';
    return 1;
}

void extract_method_name(const char* xml, char* method_name) {
    extract_between_tags(xml, "<methodName>", "</methodName>", method_name);
}

void extract_params(const char* xml, ParamList* params) {
    params->param_count = 0;
    const char* current = xml;
    char temp[MAX_PARAM_LEN];
    
    while (params->param_count < MAX_PARAMS) {
        const char* param_start = strstr(current, "<param>");
        if (!param_start) break;
        
        const char* param_end = strstr(param_start, "</param>");
        if (!param_end) break;
        
        if (extract_between_tags(param_start, "<int>", "</int>", temp) ||
            extract_between_tags(param_start, "<i4>", "</i4>", temp)) {
            strcpy(params->params[params->param_count], temp);
            params->param_count++;
        } else if (extract_between_tags(param_start, "<string>", "</string>", temp)) {
            strcpy(params->params[params->param_count], temp);
            params->param_count++;
        }
        
        current = param_end + 8;
    }
}

void build_success_response(const char* result, int is_int, char* response) {
    sprintf(response, 
        "<?xml version=\\"1.0\\"?>\\n"
        "<methodResponse>\\n"
        "  <params>\\n"
        "    <param>\\n"
        "      <value>%s%s%s</value>\\n"
        "    </param>\\n"
        "  </params>\\n"
        "</methodResponse>",
        is_int ? "<int>" : "<string>",
        result,
        is_int ? "</int>" : "</string>");
}

void build_fault_response(int fault_code, const char* fault_string, char* response) {
    sprintf(response,
        "<?xml version=\\"1.0\\"?>\\n"
        "<methodResponse>\\n"
        "  <fault>\\n"
        "    <value>\\n"
        "      <struct>\\n"
        "        <member>\\n"
        "          <name>faultCode</name>\\n"
        "          <value><int>%d</int></value>\\n"
        "        </member>\\n"
        "        <member>\\n"
        "          <name>faultString</name>\\n"
        "          <value><string>%s</string></value>\\n"
        "        </member>\\n"
        "      </struct>\\n"
        "    </value>\\n"
        "  </fault>\\n"
        "</methodResponse>",
        fault_code, fault_string);
}

void process_xmlrpc(const char* xml_request, char* response) {
    char method_name[256] = {0};
    ParamList params;
    char result[256];
    
    extract_method_name(xml_request, method_name);
    extract_params(xml_request, &params);
    
    if (strcmp(method_name, "add") == 0 && params.param_count == 2) {
        int a = atoi(params.params[0]);
        int b = atoi(params.params[1]);
        sprintf(result, "%d", a + b);
        build_success_response(result, 1, response);
    } else if (strcmp(method_name, "subtract") == 0 && params.param_count == 2) {
        int a = atoi(params.params[0]);
        int b = atoi(params.params[1]);
        sprintf(result, "%d", a - b);
        build_success_response(result, 1, response);
    } else if (strcmp(method_name, "multiply") == 0 && params.param_count == 2) {
        int a = atoi(params.params[0]);
        int b = atoi(params.params[1]);
        sprintf(result, "%d", a * b);
        build_success_response(result, 1, response);
    } else if (strcmp(method_name, "greet") == 0 && params.param_count == 1) {
        sprintf(result, "Hello, %s!", params.params[0]);
        build_success_response(result, 0, response);
    } else if (strcmp(method_name, "concat") == 0 && params.param_count == 2) {
        sprintf(result, "%s%s", params.params[0], params.params[1]);
        build_success_response(result, 0, response);
    } else {
        char fault_msg[256];
        sprintf(fault_msg, "Method not found: %s", method_name);
        build_fault_response(-1, fault_msg, response);
    }
}

int main() {
    char response[MAX_RESPONSE_LEN];
    
    printf("Test Case 1: Process add request\\n");
    const char* request1 = "<?xml version=\\"1.0\\"?><methodCall><methodName>add</methodName><params><param><value><int>5</int></value></param><param><value><int>3</int></value></param></params></methodCall>";
    process_xmlrpc(request1, response);
    printf("%s\\n", response);
    
    printf("\\nTest Case 2: Process subtract request\\n");
    const char* request2 = "<?xml version=\\"1.0\\"?><methodCall><methodName>subtract</methodName><params><param><value><int>10</int></value></param><param><value><int>4</int></value></param></params></methodCall>";
    process_xmlrpc(request2, response);
    printf("%s\\n", response);
    
    printf("\\nTest Case 3: Process multiply request\\n");
    const char* request3 = "<?xml version=\\"1.0\\"?><methodCall><methodName>multiply</methodName><params><param><value><int>6</int></value></param><param><value><int>7</int></value></param></params></methodCall>";
    process_xmlrpc(request3, response);
    printf("%s\\n", response);
    
    printf("\\nTest Case 4: Process greet request\\n");
    const char* request4 = "<?xml version=\\"1.0\\"?><methodCall><methodName>greet</methodName><params><param><value><string>Alice</string></value></param></params></methodCall>";
    process_xmlrpc(request4, response);
    printf("%s\\n", response);
    
    printf("\\nTest Case 5: Process invalid method request\\n");
    const char* request5 = "<?xml version=\\"1.0\\"?><methodCall><methodName>invalidMethod</methodName><params></params></methodCall>";
    process_xmlrpc(request5, response);
    printf("%s\\n", response);
    
    return 0;
}
