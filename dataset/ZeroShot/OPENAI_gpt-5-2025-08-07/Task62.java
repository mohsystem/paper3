import java.util.*;

public class Task62 {
    // Secure JSON root identifier with basic structural parsing and limits
    public static String identifyRoot(String json) {
        if (json == null) return "invalid";
        final int MAX_LEN = 1_000_000;
        if (json.length() > MAX_LEN) return "invalid";
        Parser p = new Parser(json);
        Type t = p.parseRoot();
        return t == null ? "invalid" : t.toString().toLowerCase();
    }

    private enum Type {
        OBJECT, ARRAY, STRING, NUMBER, BOOLEAN, NULL
    }

    private static final class Parser {
        private final char[] s;
        private int pos = 0;
        private final int len;
        private int depth = 0;
        private static final int MAX_DEPTH = 512;

        Parser(String input) {
            this.s = input.toCharArray();
            this.len = s.length;
        }

        Type parseRoot() {
            skipWS();
            if (pos >= len) return null;
            Type t = parseValue();
            if (t == null) return null;
            skipWS();
            return pos == len ? t : null;
        }

        private void skipWS() {
            while (pos < len) {
                char c = s[pos];
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n') pos++;
                else break;
            }
        }

        private Type parseValue() {
            skipWS();
            if (pos >= len) return null;
            char c = s[pos];
            if (c == '{') return parseObject() ? Type.OBJECT : null;
            if (c == '[') return parseArray() ? Type.ARRAY : null;
            if (c == '"') return parseString() ? Type.STRING : null;
            if (c == '-' || (c >= '0' && c <= '9')) return parseNumber() ? Type.NUMBER : null;
            if (c == 't') return parseLiteral("true") ? Type.BOOLEAN : null;
            if (c == 'f') return parseLiteral("false") ? Type.BOOLEAN : null;
            if (c == 'n') return parseLiteral("null") ? Type.NULL : null;
            return null;
        }

        private boolean parseLiteral(String lit) {
            if (pos + lit.length() > len) return false;
            for (int i = 0; i < lit.length(); i++) {
                if (s[pos + i] != lit.charAt(i)) return false;
            }
            pos += lit.length();
            return true;
        }

        private boolean parseObject() {
            if (depth >= MAX_DEPTH) return false;
            depth++;
            if (pos >= len || s[pos] != '{') return false;
            pos++;
            skipWS();
            if (pos < len && s[pos] == '}') {
                pos++;
                depth--;
                return true;
            }
            while (true) {
                skipWS();
                if (!parseString()) { depth--; return false; }
                skipWS();
                if (pos >= len || s[pos] != ':') { depth--; return false; }
                pos++;
                Type vt = parseValue();
                if (vt == null) { depth--; return false; }
                skipWS();
                if (pos < len && s[pos] == ',') {
                    pos++;
                    continue;
                } else if (pos < len && s[pos] == '}') {
                    pos++;
                    depth--;
                    return true;
                } else {
                    depth--;
                    return false;
                }
            }
        }

        private boolean parseArray() {
            if (depth >= MAX_DEPTH) return false;
            depth++;
            if (pos >= len || s[pos] != '[') return false;
            pos++;
            skipWS();
            if (pos < len && s[pos] == ']') {
                pos++;
                depth--;
                return true;
            }
            while (true) {
                Type t = parseValue();
                if (t == null) { depth--; return false; }
                skipWS();
                if (pos < len && s[pos] == ',') {
                    pos++;
                    continue;
                } else if (pos < len && s[pos] == ']') {
                    pos++;
                    depth--;
                    return true;
                } else {
                    depth--;
                    return false;
                }
            }
        }

        private boolean parseString() {
            if (pos >= len || s[pos] != '"') return false;
            pos++;
            while (pos < len) {
                char c = s[pos++];
                if (c == '"') return true;
                if (c == '\\') {
                    if (pos >= len) return false;
                    char e = s[pos++];
                    switch (e) {
                        case '"': case '\\': case '/': case 'b':
                        case 'f': case 'n': case 'r': case 't':
                            break;
                        case 'u':
                            for (int i = 0; i < 4; i++) {
                                if (pos >= len) return false;
                                char h = s[pos++];
                                if (!isHex(h)) return false;
                            }
                            break;
                        default:
                            return false;
                    }
                } else {
                    if (c < 0x20) return false;
                }
            }
            return false;
        }

        private boolean isHex(char c) {
            return (c >= '0' && c <= '9') ||
                   (c >= 'a' && c <= 'f') ||
                   (c >= 'A' && c <= 'F');
        }

        private boolean parseNumber() {
            int start = pos;
            if (pos < len && s[pos] == '-') pos++;
            if (pos >= len) return false;
            if (s[pos] == '0') {
                pos++;
            } else if (s[pos] >= '1' && s[pos] <= '9') {
                while (pos < len && s[pos] >= '0' && s[pos] <= '9') pos++;
            } else {
                return false;
            }
            if (pos < len && s[pos] == '.') {
                pos++;
                if (pos >= len || !(s[pos] >= '0' && s[pos] <= '9')) return false;
                while (pos < len && s[pos] >= '0' && s[pos] <= '9') pos++;
            }
            if (pos < len && (s[pos] == 'e' || s[pos] == 'E')) {
                pos++;
                if (pos < len && (s[pos] == '+' || s[pos] == '-')) pos++;
                if (pos >= len || !(s[pos] >= '0' && s[pos] <= '9')) return false;
                while (pos < len && s[pos] >= '0' && s[pos] <= '9') pos++;
            }
            return pos > start;
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "{\"a\":1,\"b\":[true,false,null]}",
            "[1,2,3]",
            "\"hello\"",
            "123.45e-6",
            "{unquoted: 1}"
        };
        for (String t : tests) {
            System.out.println(identifyRoot(t));
        }
    }
}