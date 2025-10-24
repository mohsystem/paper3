
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <limits>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

// Security: Validate that path resolves within base directory and is a regular file
bool validateFilePath(const char* basePath, const char* filePath, char* resolvedPath, size_t resolvedSize) {
    // Security: Validate input parameters
    if (!basePath || !filePath || !resolvedPath || resolvedSize == 0) {
        return false;
    }
    
    // Security: Check path length to prevent buffer overflow
    if (strlen(filePath) > PATH_MAX) {
        return false;
    }
    
    // Security: Resolve canonical paths to prevent directory traversal
    char baseResolved[PATH_MAX];
    char fileResolved[PATH_MAX];
    
    if (!realpath(basePath, baseResolved)) {
        return false;
    }
    
    // Security: Construct full path for validation
    char fullPath[PATH_MAX];
    if (snprintf(fullPath, sizeof(fullPath), "%s/%s", basePath, filePath) >= (int)sizeof(fullPath)) {
        return false;
    }
    
    if (!realpath(fullPath, fileResolved)) {
        return false;
    }
    
    // Security: Verify resolved path is within base directory
    size_t baseLen = strlen(baseResolved);
    if (strncmp(fileResolved, baseResolved, baseLen) != 0 ||
        (fileResolved[baseLen] != '\\0' && fileResolved[baseLen] != '/')) {
        return false;
    }
    
    // Security: Verify it's a regular file, not a symlink or directory\n    struct stat statbuf;\n    if (lstat(fileResolved, &statbuf) != 0) {\n        return false;\n    }\n    \n    if (!S_ISREG(statbuf.st_mode)) {\n        return false;\n    }\n    \n    // Security: Copy validated path with bounds check\n    if (strlen(fileResolved) >= resolvedSize) {\n        return false;\n    }\n    \n    strncpy(resolvedPath, fileResolved, resolvedSize - 1);\n    resolvedPath[resolvedSize - 1] = '\\0';\n    \n    return true;\n}\n\n// Security: Validate XPath ID value - only alphanumeric and underscore allowed\nbool validateXPathId(const char* id) {\n    if (!id || strlen(id) == 0 || strlen(id) > 256) {\n        return false;\n    }\n    \n    for (size_t i = 0; id[i] != '\\0'; i++) {\n        char c = id[i];\n        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || \n              (c >= '0' && c <= '9') || c == '_' || c == '-')) {\n            return false;\n        }\n    }\n    return true;\n}\n\n// Security: Safe XML file reading with size limits\nchar* readXmlFile(const char* filePath, size_t* fileSize) {\n    // Security: Open file with read-only mode\n    FILE* file = fopen(filePath, "rb");\n    if (!file) {\n        return NULL;\n    }\n    \n    // Security: Get file size and validate\n    if (fseek(file, 0, SEEK_END) != 0) {\n        fclose(file);\n        return NULL;\n    }\n    \n    long size = ftell(file);\n    if (size < 0 || size > 10 * 1024 * 1024) { // Security: Max 10MB limit\n        fclose(file);\n        return NULL;\n    }\n    \n    if (fseek(file, 0, SEEK_SET) != 0) {\n        fclose(file);\n        return NULL;\n    }\n    \n    // Security: Allocate buffer with size check\n    char* buffer = (char*)malloc((size_t)size + 1);\n    if (!buffer) {\n        fclose(file);\n        return NULL;\n    }\n    \n    // Security: Read file with bounds check\n    size_t bytesRead = fread(buffer, 1, (size_t)size, file);\n    if (bytesRead != (size_t)size) {\n        // Security: Clear sensitive data before free\n        memset(buffer, 0, (size_t)size + 1);\n        free(buffer);\n        fclose(file);\n        return NULL;\n    }\n    \n    buffer[size] = '\\0';\n    *fileSize = (size_t)size;\n    \n    fclose(file);\n    return buffer;\n}\n\nstd::string executeXPath(const char* xmlFile, const char* xpathId) {\n    // Security: Validate inputs\n    if (!xmlFile || !xpathId) {\n        return "Error: Invalid input parameters";\n    }\n    \n    // Security: Validate XPath ID format\n    if (!validateXPathId(xpathId)) {\n        return "Error: Invalid XPath ID format";\n    }\n    \n    // Security: Validate file path within base directory\n    char resolvedPath[PATH_MAX];\n    char basePath[PATH_MAX];\n    if (!getcwd(basePath, sizeof(basePath))) {\n        return "Error: Cannot determine current directory";\n    }\n    \n    if (!validateFilePath(basePath, xmlFile, resolvedPath, sizeof(resolvedPath))) {\n        return "Error: Invalid file path or file not accessible";\n    }\n    \n    // Security: Read XML file with size limits\n    size_t fileSize = 0;\n    char* xmlContent = readXmlFile(resolvedPath, &fileSize);\n    if (!xmlContent) {\n        return "Error: Cannot read XML file";\n    }\n    \n    // Security: Initialize libxml2 parser\n    LIBXML_TEST_VERSION\n    \n    // Security: Disable external entity processing to prevent XXE attacks\n    xmlParserOption options = (xmlParserOption)(\n        XML_PARSE_NONET |      // Disable network access\n        XML_PARSE_NOENT |      // Do not substitute entities\n        XML_PARSE_DTDLOAD |    // Do not load external DTD\n        XML_PARSE_DTDATTR |    // Do not default DTD attributes\n        XML_PARSE_DTDVALID     // Do not validate with DTD\n    );\n    \n    // Security: Parse XML with secure options\n    xmlDocPtr doc = xmlReadMemory(xmlContent, (int)fileSize, "noname.xml", NULL, options);\n    \n    // Security: Clear XML content from memory after parsing\n    memset(xmlContent, 0, fileSize);\n    free(xmlContent);\n    \n    if (!doc) {\n        xmlCleanupParser();\n        return "Error: Failed to parse XML";\n    }\n    \n    // Security: Create XPath context\n    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);\n    if (!xpathCtx) {\n        xmlFreeDoc(doc);\n        xmlCleanupParser();\n        return "Error: Failed to create XPath context";\n    }\n    \n    // Security: Construct XPath query with proper escaping\n    char xpathQuery[512];\n    int written = snprintf(xpathQuery, sizeof(xpathQuery), "/tag[@id='%s']", xpathId);\n    if (written < 0 || written >= (int)sizeof(xpathQuery)) {\n        xmlXPathFreeContext(xpathCtx);\n        xmlFreeDoc(doc);\n        xmlCleanupParser();\n        return "Error: XPath query too long";\n    }\n    \n    // Security: Execute XPath query\n    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(\n        (const xmlChar*)xpathQuery, xpathCtx);\n    \n    std::string result;\n    \n    if (!xpathObj) {\n        result = "Error: Failed to evaluate XPath";\n    } else {\n        xmlNodeSetPtr nodes = xpathObj->nodesetval;\n        if (!nodes || nodes->nodeNr == 0) {\n            result = "No results found";\n        } else {\n            std::ostringstream oss;\n            for (int i = 0; i < nodes->nodeNr; i++) {\n                xmlNodePtr node = nodes->nodeTab[i];\n                xmlChar* content = xmlNodeGetContent(node);\n                if (content) {\n                    if (i > 0) oss << ", ";\n                    oss << (const char*)content;\n                    xmlFree(content);\n                }\n            }\n            result = oss.str();\n        }\n        xmlXPathFreeObject(xpathObj);\n    }\n    \n    // Security: Clean up resources\n    xmlXPathFreeContext(xpathCtx);\n    xmlFreeDoc(doc);\n    xmlCleanupParser();\n    \n    return result;\n}\n\nint main(int argc, char* argv[]) {\n    // Security: Validate command line argument count\n    if (argc != 3) {\n        std::cerr << "Usage: " << (argv[0] ? argv[0] : "program") \n                  << " <xpath_id> <xml_file>" << std::endl;\n        return 1;\n    }\n    \n    // Security: Validate argument pointers\n    if (!argv[1] || !argv[2]) {\n        std::cerr << "Error: Invalid arguments" << std::endl;
        return 1;
    }
    
    const char* xpathId = argv[1];
    const char* xmlFile = argv[2];
    
    std::string result = executeXPath(xmlFile, xpathId);
    std::cout << result << std::endl;
    
    return 0;
}
