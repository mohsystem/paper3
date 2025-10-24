import java.util.*;

public class Task62 {
    public static String getJsonRootType(String json) {
        if (json == null) return "invalid";
        int n = json.length();
        int i = 0;
        while (i < n && Character.isWhitespace(json.charAt(i))) i++;
        if (i >= n) return "invalid";
        char c = json.charAt(i);
        if (c == '{') return "object";
        if (c == '[') return "array";
        if (c == '"') return "string";
        if (c == 't' || c == 'f') return "boolean";
        if (c == 'n') return "null";
        if (c == '-' || (c >= '0' && c <= '9')) return "number";
        return "invalid";
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "{\"name\":\"Alice\",\"age\":30}",
            "[{\"id\":1},{\"id\":2}]",
            "\"sample\"",
            "-42.0e10",
            "true"
        };
        for (String t : tests) {
            System.out.println(getJsonRootType(t));
        }
    }
}