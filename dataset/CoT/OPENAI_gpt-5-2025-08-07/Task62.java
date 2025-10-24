import java.util.*;

/*
Chain-of-Through Secure Implementation
1) Problem understanding: Parse a JSON string to extract its structural representation and identify the root element.
2) Security requirements: No external libraries, strict bounds checks, robust parsing of strings, numbers, booleans, null, objects, arrays; avoid code injection by not executing any code; handle malformed inputs safely.
3) Secure coding generation: Implement a defensive recursive-descent parser, with careful index management and escape sequence handling.
4) Code review: Validate each operation, ensure no unbounded recursion from input size, and avoid exceptions by checking indices before access.
5) Secure code output: Return consistent, deterministic output. For invalid inputs, return "Root=invalid; Structure=invalid".
*/

public class Task62 {

    // Public API: parse and return "Root=<type>; Structure=<structure>"
    public static String extractRootAndStructure(String json) {
        if (json == null) {
            return "Root=invalid; Structure=invalid";
        }
        Parser p = new Parser(json);
        String struct = p.parse();
        if (struct == null) {
            return "Root=invalid; Structure=invalid";
        }
        String root = determineRootFromStructure(struct);
        return "Root=" + root + "; Structure=" + struct;
    }

    private static String determineRootFromStructure(String struct) {
        if (struct == null) return "invalid";
        if (struct.startsWith("{")) return "object";
        if (struct.startsWith("[")) return "array";
        if (struct.equals("string")) return "string";
        if (struct.equals("number")) return "number";
        if (struct.equals("boolean")) return "boolean";
        if (struct.equals("null")) return "null";
        return "invalid";
    }

    // Secure JSON Parser
    static final class Parser {
        private final String s;
        private int i;
        private final int n;
        private static final int MAX_DEPTH = 1000;

        Parser(String s) {
            this.s = s;
            this.i = 0;
            this.n = s.length();
        }

        String parse() {
            skipWS();
            String v = parseValue(0);
            if (v == null) return null;
            skipWS();
            if (i != n) return null; // trailing garbage
            return v;
        }

        private void skipWS() {
            while (i < n) {
                char c = s.charAt(i);
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n') i++;
                else break;
            }
        }

        private String parseValue(int depth) {
            if (depth > MAX_DEPTH) return null; // prevent excessive nesting
            skipWS();
            if (i >= n) return null;
            char c = s.charAt(i);
            if (c == '{') return parseObject(depth + 1);
            if (c == '[') return parseArray(depth + 1);
            if (c == '"') {
                String str = parseString();
                if (str == null) return null;
                return "string";
            }
            if (c == '-' || (c >= '0' && c <= '9')) {
                return parseNumber() ? "number" : null;
            }
            if (matchLiteral("true")) return "boolean";
            if (matchLiteral("false")) return "boolean";
            if (matchLiteral("null")) return "null";
            return null;
        }

        private boolean matchLiteral(String lit) {
            int len = lit.length();
            if (i + len > n) return false;
            if (s.regionMatches(i, lit, 0, len)) {
                i += len;
                return true;
            }
            return false;
        }

        private String parseObject(int depth) {
            if (i >= n || s.charAt(i) != '{') return null;
            i++; // consume '{'
            skipWS();
            if (i < n && s.charAt(i) == '}') {
                i++;
                return "{}";
            }
            List<String> keys = new ArrayList<>();
            List<String> vals = new ArrayList<>();
            while (true) {
                skipWS();
                if (i >= n || s.charAt(i) != '"') return null;
                String key = parseString();
                if (key == null) return null;
                skipWS();
                if (i >= n || s.charAt(i) != ':') return null;
                i++; // ':'
                skipWS();
                String v = parseValue(depth);
                if (v == null) return null;
                keys.add(key);
                vals.add(v);
                skipWS();
                if (i >= n) return null;
                char c = s.charAt(i);
                if (c == ',') {
                    i++;
                    continue;
                } else if (c == '}') {
                    i++;
                    break;
                } else {
                    return null;
                }
            }
            StringBuilder sb = new StringBuilder();
            sb.append('{');
            for (int k = 0; k < keys.size(); k++) {
                if (k > 0) sb.append(", ");
                sb.append('"').append(escapeForOutput(keys.get(k))).append('"').append(": ").append(vals.get(k));
            }
            sb.append('}');
            return sb.toString();
        }

        private String parseArray(int depth) {
            if (i >= n || s.charAt(i) != '[') return null;
            i++; // consume '['
            skipWS();
            if (i < n && s.charAt(i) == ']') {
                i++;
                return "[]";
            }
            List<String> elems = new ArrayList<>();
            while (true) {
                String v = parseValue(depth);
                if (v == null) return null;
                elems.add(v);
                skipWS();
                if (i >= n) return null;
                char c = s.charAt(i);
                if (c == ',') {
                    i++;
                    skipWS();
                    continue;
                } else if (c == ']') {
                    i++;
                    break;
                } else {
                    return null;
                }
            }
            // Unique, preserve order
            LinkedHashSet<String> uniq = new LinkedHashSet<>(elems);
            StringBuilder sb = new StringBuilder();
            sb.append('[');
            int idx = 0;
            for (String u : uniq) {
                if (idx++ > 0) sb.append(" | ");
                sb.append(u);
            }
            sb.append(']');
            return sb.toString();
        }

        private String parseString() {
            if (i >= n || s.charAt(i) != '"') return null;
            i++; // consume opening quote
            StringBuilder sb = new StringBuilder();
            while (i < n) {
                char c = s.charAt(i++);
                if (c == '"') {
                    return sb.toString();
                } else if (c == '\\') {
                    if (i >= n) return null;
                    char esc = s.charAt(i++);
                    switch (esc) {
                        case '"': sb.append('"'); break;
                        case '\\': sb.append('\\'); break;
                        case '/': sb.append('/'); break;
                        case 'b': sb.append('\b'); break;
                        case 'f': sb.append('\f'); break;
                        case 'n': sb.append('\n'); break;
                        case 'r': sb.append('\r'); break;
                        case 't': sb.append('\t'); break;
                        case 'u':
                            // Expect 4 hex digits
                            if (i + 4 > n) return null;
                            for (int k = 0; k < 4; k++) {
                                char h = s.charAt(i + k);
                                if (!isHex(h)) return null;
                            }
                            // We won't decode unicode for structure; just skip
                            i += 4;
                            sb.append('?');
                            break;
                        default:
                            return null;
                    }
                } else {
                    // Regular char; JSON disallows control chars
                    if (c >= 0 && c <= 0x1F) return null;
                    sb.append(c);
                }
            }
            return null; // Unterminated
        }

        private boolean parseNumber() {
            int start = i;
            if (i < n && s.charAt(i) == '-') i++;
            if (i >= n) return false;
            if (s.charAt(i) == '0') {
                i++;
            } else if (s.charAt(i) >= '1' && s.charAt(i) <= '9') {
                i++;
                while (i < n && isDigit(s.charAt(i))) i++;
            } else {
                return false;
            }
            // Fraction
            if (i < n && s.charAt(i) == '.') {
                i++;
                if (i >= n || !isDigit(s.charAt(i))) return false;
                while (i < n && isDigit(s.charAt(i))) i++;
            }
            // Exponent
            if (i < n && (s.charAt(i) == 'e' || s.charAt(i) == 'E')) {
                i++;
                if (i < n && (s.charAt(i) == '+' || s.charAt(i) == '-')) i++;
                if (i >= n || !isDigit(s.charAt(i))) return false;
                while (i < n && isDigit(s.charAt(i))) i++;
            }
            return i > start;
        }

        private static boolean isDigit(char c) {
            return c >= '0' && c <= '9';
        }

        private static boolean isHex(char c) {
            return (c >= '0' && c <= '9') ||
                   (c >= 'a' && c <= 'f') ||
                   (c >= 'A' && c <= 'F');
        }

        private static String escapeForOutput(String in) {
            StringBuilder sb = new StringBuilder();
            for (int k = 0; k < in.length(); k++) {
                char c = in.charAt(k);
                if (c == '\\' || c == '"') {
                    sb.append('\\').append(c);
                } else if (c >= 0 && c <= 0x1F) {
                    sb.append('?'); // sanitize control chars
                } else {
                    sb.append(c);
                }
            }
            return sb.toString();
        }
    }

    // 5 test cases in main
    public static void main(String[] args) {
        String[] tests = new String[] {
            "{\"a\":1,\"b\":{\"c\":[1,2,3]},\"d\":[{\"e\":\"x\"},{\"e\":\"y\",\"f\":true}]}",
            "[1,2,{\"a\":[null,false]}]",
            "\"hello\"",
            "123.45e-6",
            "{unclosed"
        };
        for (String t : tests) {
            String out = extractRootAndStructure(t);
            System.out.println(out);
        }
    }
}