import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class Task62 {

    private enum NodeType {
        OBJECT, ARRAY, STRING, NUMBER, BOOLEAN, NULL
    }

    public static String identifyRootElement(String json) {
        if (json == null) {
            return "INVALID";
        }
        byte[] bytes = json.getBytes(StandardCharsets.UTF_8);
        if (bytes.length > 1_000_000) { // basic size validation
            return "INVALID";
        }
        Parser p = new Parser(json);
        try {
            p.skipWS();
            NodeType t = p.parseValue();
            p.skipWS();
            if (!p.atEnd()) {
                return "INVALID";
            }
            return nodeTypeToString(t);
        } catch (ParseException ex) {
            return "INVALID";
        }
    }

    private static String nodeTypeToString(NodeType t) {
        switch (t) {
            case OBJECT: return "object";
            case ARRAY: return "array";
            case STRING: return "string";
            case NUMBER: return "number";
            case BOOLEAN: return "boolean";
            case NULL: return "null";
            default: return "INVALID";
        }
    }

    private static final class ParseException extends Exception {
        ParseException(String msg) {
            super(msg);
        }
    }

    private static final class Parser {
        private final String s;
        private final int n;
        private int i;

        Parser(String s) {
            this.s = s;
            this.n = s.length();
            this.i = 0;
        }

        boolean atEnd() {
            return i >= n;
        }

        void skipWS() {
            while (i < n) {
                char c = s.charAt(i);
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                    i++;
                } else {
                    break;
                }
            }
        }

        NodeType parseValue() throws ParseException {
            if (i >= n) throw error("Unexpected end of input");
            char c = s.charAt(i);
            switch (c) {
                case '{':
                    parseObject();
                    return NodeType.OBJECT;
                case '[':
                    parseArray();
                    return NodeType.ARRAY;
                case '"':
                    parseString();
                    return NodeType.STRING;
                case 't':
                    parseTrue();
                    return NodeType.BOOLEAN;
                case 'f':
                    parseFalse();
                    return NodeType.BOOLEAN;
                case 'n':
                    parseNull();
                    return NodeType.NULL;
                default:
                    if (c == '-' || isDigit(c)) {
                        parseNumber();
                        return NodeType.NUMBER;
                    }
                    throw error("Invalid value");
            }
        }

        void parseObject() throws ParseException {
            expect('{');
            skipWS();
            if (peek('}')) {
                i++;
                return;
            }
            while (true) {
                skipWS();
                parseString(); // key
                skipWS();
                expect(':');
                skipWS();
                parseValue();
                skipWS();
                if (peek('}')) {
                    i++;
                    break;
                }
                expect(',');
            }
        }

        void parseArray() throws ParseException {
            expect('[');
            skipWS();
            if (peek(']')) {
                i++;
                return;
            }
            while (true) {
                skipWS();
                parseValue();
                skipWS();
                if (peek(']')) {
                    i++;
                    break;
                }
                expect(',');
            }
        }

        void parseString() throws ParseException {
            expect('"');
            while (i < n) {
                char c = s.charAt(i++);
                if (c == '"') {
                    return;
                }
                if (c == '\\') {
                    if (i >= n) throw error("Unterminated escape");
                    char e = s.charAt(i++);
                    switch (e) {
                        case '"':
                        case '\\':
                        case '/':
                        case 'b':
                        case 'f':
                        case 'n':
                        case 'r':
                        case 't':
                            break;
                        case 'u':
                            for (int k = 0; k < 4; k++) {
                                if (i >= n) throw error("Invalid unicode escape");
                                char h = s.charAt(i++);
                                if (!isHex(h)) throw error("Invalid unicode hex");
                            }
                            break;
                        default:
                            throw error("Invalid escape");
                    }
                } else {
                    if (c < 0x20) throw error("Invalid control in string");
                }
            }
            throw error("Unterminated string");
        }

        void parseTrue() throws ParseException {
            expect('t'); expect('r'); expect('u'); expect('e');
        }

        void parseFalse() throws ParseException {
            expect('f'); expect('a'); expect('l'); expect('s'); expect('e');
        }

        void parseNull() throws ParseException {
            expect('n'); expect('u'); expect('l'); expect('l');
        }

        void parseNumber() throws ParseException {
            int start = i;
            if (peek('-')) i++;
            if (i >= n) throw error("Invalid number");
            if (peek('0')) {
                i++;
            } else {
                if (!isDigit1to9(peekChar())) throw error("Invalid number");
                while (i < n && isDigit(peekChar())) i++;
            }
            if (peek('.')) {
                i++;
                if (i >= n || !isDigit(peekChar())) throw error("Invalid fraction");
                while (i < n && isDigit(peekChar())) i++;
            }
            if (peek('e') || peek('E')) {
                i++;
                if (peek('+') || peek('-')) i++;
                if (i >= n || !isDigit(peekChar())) throw error("Invalid exponent");
                while (i < n && isDigit(peekChar())) i++;
            }
            if (i <= start) throw error("Invalid number");
        }

        boolean peek(char c) {
            return i < n && s.charAt(i) == c;
        }

        char peekChar() {
            return i < n ? s.charAt(i) : '\0';
        }

        void expect(char c) throws ParseException {
            if (i >= n || s.charAt(i) != c) throw error("Expected '" + c + "'");
            i++;
        }

        boolean isDigit(char c) {
            return c >= '0' && c <= '9';
        }

        boolean isDigit1to9(char c) {
            return c >= '1' && c <= '9';
        }

        boolean isHex(char c) {
            return (c >= '0' && c <= '9') ||
                   (c >= 'a' && c <= 'f') ||
                   (c >= 'A' && c <= 'F');
        }

        ParseException error(String msg) {
            return new ParseException(msg);
        }
    }

    public static void main(String[] args) {
        List<String> tests = new ArrayList<>();
        tests.add("{\"a\":1,\"b\":[true,false,null],\"c\":{\"d\":\"x\"}}");
        tests.add("[1,2,3, {\"k\": [\"v\", 2]}]");
        tests.add("\"hello world\"");
        tests.add("-12.3e-5");
        tests.add("true");

        for (String t : tests) {
            String root = identifyRootElement(t);
            System.out.println(root);
        }
    }
}