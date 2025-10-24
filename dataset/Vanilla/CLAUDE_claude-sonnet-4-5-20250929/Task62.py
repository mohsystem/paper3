
class JSONElement:
    def __init__(self, element_type, value):
        self.type = element_type
        self.value = value
    
    def __repr__(self):
        return f"JSONElement(type='{self.type}', value={self.value})"

def parse_and_get_root(json_string):
    if not json_string or not json_string.strip():
        return JSONElement("null", None)
    
    json_string = json_string.strip()
    result, _ = parse_value(json_string, 0)
    return result

def parse_value(json_str, start):
    while start < len(json_str) and json_str[start].isspace():
        start += 1
    
    if start >= len(json_str):
        return JSONElement("null", None), start
    
    ch = json_str[start]
    
    if ch == '{':
        return parse_object(json_str, start)
    elif ch == '[':
        return parse_array(json_str, start)
    elif ch == '"':
        return parse_string(json_str, start)
    elif ch in 'tf':
        return parse_boolean(json_str, start)
    elif ch == 'n':
        return parse_null(json_str, start)
    elif ch == '-' or ch.isdigit():
        return parse_number(json_str, start)
    
    return JSONElement("null", None), start

def parse_object(json_str, start):
    obj = {}
    i = start + 1
    
    while i < len(json_str):
        while i < len(json_str) and json_str[i].isspace():
            i += 1
        
        if i < len(json_str) and json_str[i] == '}':
            return JSONElement("object", obj), i + 1
        
        key_element, i = parse_string(json_str, i)
        key = key_element.value
        
        while i < len(json_str) and json_str[i].isspace():
            i += 1
        
        if i < len(json_str) and json_str[i] == ':':
            i += 1
        
        value_element, i = parse_value(json_str, i)
        obj[key] = value_element
        
        while i < len(json_str) and json_str[i].isspace():
            i += 1
        
        if i < len(json_str) and json_str[i] == ',':
            i += 1
    
    return JSONElement("object", obj), i

def parse_array(json_str, start):
    arr = []
    i = start + 1
    
    while i < len(json_str):
        while i < len(json_str) and json_str[i].isspace():
            i += 1
        
        if i < len(json_str) and json_str[i] == ']':
            return JSONElement("array", arr), i + 1
        
        value_element, i = parse_value(json_str, i)
        arr.append(value_element)
        
        while i < len(json_str) and json_str[i].isspace():
            i += 1
        
        if i < len(json_str) and json_str[i] == ',':
            i += 1
    
    return JSONElement("array", arr), i

def parse_string(json_str, start):
    sb = []
    i = start + 1
    
    while i < len(json_str) and json_str[i] != '"':
        if json_str[i] == '\\\\' and i + 1 < len(json_str):\n            i += 1\n        sb.append(json_str[i])\n        i += 1\n    \n    return JSONElement("string", ''.join(sb)), i + 1\n\ndef parse_number(json_str, start):\n    i = start\n    while i < len(json_str) and (json_str[i].isdigit() or json_str[i] in '-.eE+'):\n        i += 1\n    \n    num_str = json_str[start:i]\n    return JSONElement("number", num_str), i\n\ndef parse_boolean(json_str, start):\n    if json_str[start:start+4] == "true":\n        return JSONElement("boolean", "true"), start + 4\n    elif json_str[start:start+5] == "false":\n        return JSONElement("boolean", "false"), start + 5\n    return JSONElement("null", None), start\n\ndef parse_null(json_str, start):\n    if json_str[start:start+4] == "null":\n        return JSONElement("null", None), start + 4\n    return JSONElement("null", None), start\n\nif __name__ == "__main__":\n    # Test case 1: Simple object\n    test1 = '{"name": "John", "age": 30}'\n    print("Test 1:", parse_and_get_root(test1))\n    \n    # Test case 2: Array\n    test2 = '[1, 2, 3, 4, 5]'\n    print("Test 2:", parse_and_get_root(test2))\n    \n    # Test case 3: Nested object\n    test3 = '{"person": {"name": "Alice", "age": 25}, "active": true}'\n    print("Test 3:", parse_and_get_root(test3))\n    \n    # Test case 4: String value\n    test4 = '"Hello World"'\n    print("Test 4:", parse_and_get_root(test4))\n    \n    # Test case 5: Complex nested structure\n    test5 = '{"users": [{"id": 1, "name": "Bob"}, {"id": 2, "name": "Carol"}], "count": 2}'
    print("Test 5:", parse_and_get_root(test5))
