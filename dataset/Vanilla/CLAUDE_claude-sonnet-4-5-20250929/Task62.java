
import java.util.*;

public class Task62 {
    static class JSONElement {
        String type;
        Object value;
        
        JSONElement(String type, Object value) {
            this.type = type;
            this.value = value;
        }
        
        @Override
        public String toString() {
            return "JSONElement{type='" + type + "', value=" + value + "}";
        }
    }
    
    public static JSONElement parseAndGetRoot(String jsonString) {
        if (jsonString == null || jsonString.trim().isEmpty()) {
            return new JSONElement("null", null);
        }
        
        jsonString = jsonString.trim();
        return parseValue(jsonString, 0).element;
    }
    
    static class ParseResult {
        JSONElement element;
        int endIndex;
        
        ParseResult(JSONElement element, int endIndex) {
            this.element = element;
            this.endIndex = endIndex;
        }
    }
    
    private static ParseResult parseValue(String json, int start) {
        while (start < json.length() && Character.isWhitespace(json.charAt(start))) {
            start++;
        }
        
        if (start >= json.length()) {
            return new ParseResult(new JSONElement("null", null), start);
        }
        
        char ch = json.charAt(start);
        
        if (ch == '{') {
            return parseObject(json, start);
        } else if (ch == '[') {
            return parseArray(json, start);
        } else if (ch == '"') {
            return parseString(json, start);
        } else if (ch == 't' || ch == 'f') {
            return parseBoolean(json, start);
        } else if (ch == 'n') {
            return parseNull(json, start);
        } else if (ch == '-' || Character.isDigit(ch)) {
            return parseNumber(json, start);
        }
        
        return new ParseResult(new JSONElement("null", null), start);
    }
    
    private static ParseResult parseObject(String json, int start) {
        Map<String, JSONElement> map = new LinkedHashMap<>();
        int i = start + 1;
        
        while (i < json.length()) {
            while (i < json.length() && Character.isWhitespace(json.charAt(i))) {
                i++;
            }
            
            if (i < json.length() && json.charAt(i) == '}') {
                return new ParseResult(new JSONElement("object", map), i + 1);
            }
            
            ParseResult keyResult = parseString(json, i);
            String key = (String) keyResult.element.value;
            i = keyResult.endIndex;
            
            while (i < json.length() && Character.isWhitespace(json.charAt(i))) {
                i++;
            }
            
            if (i < json.length() && json.charAt(i) == ':') {
                i++;
            }
            
            ParseResult valueResult = parseValue(json, i);
            map.put(key, valueResult.element);
            i = valueResult.endIndex;
            
            while (i < json.length() && Character.isWhitespace(json.charAt(i))) {
                i++;
            }
            
            if (i < json.length() && json.charAt(i) == ',') {
                i++;
            }
        }
        
        return new ParseResult(new JSONElement("object", map), i);
    }
    
    private static ParseResult parseArray(String json, int start) {
        List<JSONElement> list = new ArrayList<>();
        int i = start + 1;
        
        while (i < json.length()) {
            while (i < json.length() && Character.isWhitespace(json.charAt(i))) {
                i++;
            }
            
            if (i < json.length() && json.charAt(i) == ']') {
                return new ParseResult(new JSONElement("array", list), i + 1);
            }
            
            ParseResult valueResult = parseValue(json, i);
            list.add(valueResult.element);
            i = valueResult.endIndex;
            
            while (i < json.length() && Character.isWhitespace(json.charAt(i))) {
                i++;
            }
            
            if (i < json.length() && json.charAt(i) == ',') {
                i++;
            }
        }
        
        return new ParseResult(new JSONElement("array", list), i);
    }
    
    private static ParseResult parseString(String json, int start) {
        StringBuilder sb = new StringBuilder();
        int i = start + 1;
        
        while (i < json.length() && json.charAt(i) != '"') {
            if (json.charAt(i) == '\\\\' && i + 1 < json.length()) {\n                i++;\n            }\n            sb.append(json.charAt(i));\n            i++;\n        }\n        \n        return new ParseResult(new JSONElement("string", sb.toString()), i + 1);\n    }\n    \n    private static ParseResult parseNumber(String json, int start) {\n        int i = start;\n        while (i < json.length() && (Character.isDigit(json.charAt(i)) || json.charAt(i) == '-' || json.charAt(i) == '.' || json.charAt(i) == 'e' || json.charAt(i) == 'E' || json.charAt(i) == '+')) {
            i++;
        }
        
        String numStr = json.substring(start, i);
        return new ParseResult(new JSONElement("number", numStr), i);
    }
    
    private static ParseResult parseBoolean(String json, int start) {
        if (json.startsWith("true", start)) {
            return new ParseResult(new JSONElement("boolean", "true"), start + 4);
        } else if (json.startsWith("false", start)) {
            return new ParseResult(new JSONElement("boolean", "false"), start + 5);
        }
        return new ParseResult(new JSONElement("null", null), start);
    }
    
    private static ParseResult parseNull(String json, int start) {
        if (json.startsWith("null", start)) {
            return new ParseResult(new JSONElement("null", null), start + 4);
        }
        return new ParseResult(new JSONElement("null", null), start);
    }
    
    public static void main(String[] args) {
        // Test case 1: Simple object
        String test1 = "{\\"name\\": \\"John\\", \\"age\\": 30}";
        System.out.println("Test 1: " + parseAndGetRoot(test1));
        
        // Test case 2: Array
        String test2 = "[1, 2, 3, 4, 5]";
        System.out.println("Test 2: " + parseAndGetRoot(test2));
        
        // Test case 3: Nested object
        String test3 = "{\\"person\\": {\\"name\\": \\"Alice\\", \\"age\\": 25}, \\"active\\": true}";
        System.out.println("Test 3: " + parseAndGetRoot(test3));
        
        // Test case 4: String value
        String test4 = "\\"Hello World\\"";
        System.out.println("Test 4: " + parseAndGetRoot(test4));
        
        // Test case 5: Complex nested structure
        String test5 = "{\\"users\\": [{\\"id\\": 1, \\"name\\": \\"Bob\\"}, {\\"id\\": 2, \\"name\\": \\"Carol\\"}], \\"count\\": 2}";
        System.out.println("Test 5: " + parseAndGetRoot(test5));
    }
}
