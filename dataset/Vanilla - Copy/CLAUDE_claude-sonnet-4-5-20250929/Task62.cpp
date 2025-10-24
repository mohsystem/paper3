
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <cctype>

using namespace std;

class JSONElement {
public:
    string type;
    string stringValue;
    map<string, shared_ptr<JSONElement>> objectValue;
    vector<shared_ptr<JSONElement>> arrayValue;
    
    JSONElement(string t, string v = "") : type(t), stringValue(v) {}
    
    void print() {
        cout << "JSONElement{type='" << type << "', value=";
        if (type == "string" || type == "number" || type == "boolean") {
            cout << stringValue;
        } else if (type == "object") {
            cout << "{...}";
        } else if (type == "array") {
            cout << "[...]";
        } else {
            cout << "null";
        }
        cout << "}" << endl;
    }
};

struct ParseResult {
    shared_ptr<JSONElement> element;
    int endIndex;
    
    ParseResult(shared_ptr<JSONElement> e, int i) : element(e), endIndex(i) {}
};

ParseResult parseValue(const string& json, int start);

ParseResult parseObject(const string& json, int start) {
    auto obj = make_shared<JSONElement>("object");
    int i = start + 1;
    
    while (i < json.length()) {
        while (i < json.length() && isspace(json[i])) i++;
        
        if (i < json.length() && json[i] == '}') {
            return ParseResult(obj, i + 1);
        }
        
        int keyStart = i + 1;
        while (i + 1 < json.length() && json[i + 1] != '"') i++;
        string key = json.substr(keyStart, i + 1 - keyStart);
        i += 2;
        
        while (i < json.length() && isspace(json[i])) i++;
        if (i < json.length() && json[i] == ':') i++;
        
        ParseResult valueResult = parseValue(json, i);
        obj->objectValue[key] = valueResult.element;
        i = valueResult.endIndex;
        
        while (i < json.length() && isspace(json[i])) i++;
        if (i < json.length() && json[i] == ',') i++;
    }
    
    return ParseResult(obj, i);
}

ParseResult parseArray(const string& json, int start) {
    auto arr = make_shared<JSONElement>("array");
    int i = start + 1;
    
    while (i < json.length()) {
        while (i < json.length() && isspace(json[i])) i++;
        
        if (i < json.length() && json[i] == ']') {
            return ParseResult(arr, i + 1);
        }
        
        ParseResult valueResult = parseValue(json, i);
        arr->arrayValue.push_back(valueResult.element);
        i = valueResult.endIndex;
        
        while (i < json.length() && isspace(json[i])) i++;
        if (i < json.length() && json[i] == ',') i++;
    }
    
    return ParseResult(arr, i);
}

ParseResult parseString(const string& json, int start) {
    string value;
    int i = start + 1;
    
    while (i < json.length() && json[i] != '"') {
        if (json[i] == '\\\\' && i + 1 < json.length()) i++;\n        value += json[i];\n        i++;\n    }\n    \n    return ParseResult(make_shared<JSONElement>("string", value), i + 1);\n}\n\nParseResult parseNumber(const string& json, int start) {\n    int i = start;\n    while (i < json.length() && (isdigit(json[i]) || json[i] == '-' || json[i] == '.' || json[i] == 'e' || json[i] == 'E' || json[i] == '+')) {\n        i++;\n    }\n    \n    string numStr = json.substr(start, i - start);\n    return ParseResult(make_shared<JSONElement>("number", numStr), i);\n}\n\nParseResult parseBoolean(const string& json, int start) {\n    if (json.substr(start, 4) == "true") {\n        return ParseResult(make_shared<JSONElement>("boolean", "true"), start + 4);\n    } else if (json.substr(start, 5) == "false") {\n        return ParseResult(make_shared<JSONElement>("boolean", "false"), start + 5);\n    }\n    return ParseResult(make_shared<JSONElement>("null"), start);\n}\n\nParseResult parseNull(const string& json, int start) {\n    if (json.substr(start, 4) == "null") {\n        return ParseResult(make_shared<JSONElement>("null"), start + 4);\n    }\n    return ParseResult(make_shared<JSONElement>("null"), start);\n}\n\nParseResult parseValue(const string& json, int start) {\n    while (start < json.length() && isspace(json[start])) start++;\n    \n    if (start >= json.length()) {\n        return ParseResult(make_shared<JSONElement>("null"), start);\n    }\n    \n    char ch = json[start];\n    \n    if (ch == '{') {\n        return parseObject(json, start);\n    } else if (ch == '[') {\n        return parseArray(json, start);\n    } else if (ch == '"') {\n        return parseString(json, start);\n    } else if (ch == 't' || ch == 'f') {\n        return parseBoolean(json, start);\n    } else if (ch == 'n') {\n        return parseNull(json, start);\n    } else if (ch == '-' || isdigit(ch)) {\n        return parseNumber(json, start);\n    }\n    \n    return ParseResult(make_shared<JSONElement>("null"), start);\n}\n\nshared_ptr<JSONElement> parseAndGetRoot(const string& jsonString) {\n    if (jsonString.empty()) {\n        return make_shared<JSONElement>("null");\n    }\n    \n    ParseResult result = parseValue(jsonString, 0);\n    return result.element;\n}\n\nint main() {\n    // Test case 1: Simple object\n    string test1 = "{\\"name\\": \\"John\\", \\"age\\": 30}";
    cout << "Test 1: ";
    parseAndGetRoot(test1)->print();
    
    // Test case 2: Array
    string test2 = "[1, 2, 3, 4, 5]";
    cout << "Test 2: ";
    parseAndGetRoot(test2)->print();
    
    // Test case 3: Nested object
    string test3 = "{\\"person\\": {\\"name\\": \\"Alice\\", \\"age\\": 25}, \\"active\\": true}";
    cout << "Test 3: ";
    parseAndGetRoot(test3)->print();
    
    // Test case 4: String value
    string test4 = "\\"Hello World\\"";
    cout << "Test 4: ";
    parseAndGetRoot(test4)->print();
    
    // Test case 5: Complex nested structure
    string test5 = "{\\"users\\": [{\\"id\\": 1, \\"name\\": \\"Bob\\"}, {\\"id\\": 2, \\"name\\": \\"Carol\\"}], \\"count\\": 2}";
    cout << "Test 5: ";
    parseAndGetRoot(test5)->print();
    
    return 0;
}
