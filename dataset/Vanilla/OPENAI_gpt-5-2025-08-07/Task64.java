import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.*;

public class Task64 {

    public static Object fetchJsonFromUrl(String urlStr) throws IOException {
        HttpURLConnection conn = null;
        InputStream in = null;
        try {
            URL url = new URL(urlStr);
            conn = (HttpURLConnection) url.openConnection();
            conn.setRequestProperty("Accept", "application/json");
            conn.setConnectTimeout(10000);
            conn.setReadTimeout(15000);
            conn.connect();
            int code = conn.getResponseCode();
            in = code >= 200 && code < 300 ? conn.getInputStream() : conn.getErrorStream();
            if (in == null) throw new IOException("No response stream. HTTP code: " + code);
            String jsonText = readAll(in);
            JSONParser parser = new JSONParser(jsonText);
            return parser.parse();
        } finally {
            if (in != null) try { in.close(); } catch (IOException ignored) {}
            if (conn != null) conn.disconnect();
        }
    }

    private static String readAll(InputStream in) throws IOException {
        StringBuilder sb = new StringBuilder();
        try (Reader r = new BufferedReader(new InputStreamReader(in, StandardCharsets.UTF_8))) {
            char[] buf = new char[4096];
            int n;
            while ((n = r.read(buf)) != -1) {
                sb.append(buf, 0, n);
            }
        }
        // Strip UTF-8 BOM if present
        String s = sb.toString();
        if (s.startsWith("\uFEFF")) s = s.substring(1);
        return s;
    }

    // Helper to pretty-print parsed JSON back to a string
    public static String toJsonString(Object value) {
        StringBuilder sb = new StringBuilder();
        toJsonString(value, sb);
        return sb.toString();
    }

    @SuppressWarnings("unchecked")
    private static void toJsonString(Object v, StringBuilder sb) {
        if (v == null) {
            sb.append("null");
        } else if (v instanceof String) {
            sb.append('"').append(escapeString((String) v)).append('"');
        } else if (v instanceof Number || v instanceof Boolean) {
            sb.append(String.valueOf(v));
        } else if (v instanceof Map) {
            sb.append('{');
            boolean first = true;
            for (Map.Entry<String, Object> e : ((Map<String, Object>) v).entrySet()) {
                if (!first) sb.append(',');
                first = false;
                sb.append('"').append(escapeString(e.getKey())).append('"').append(':');
                toJsonString(e.getValue(), sb);
            }
            sb.append('}');
        } else if (v instanceof List) {
            sb.append('[');
            boolean first = true;
            for (Object o : (List<?>) v) {
                if (!first) sb.append(',');
                first = false;
                toJsonString(o, sb);
            }
            sb.append(']');
        } else {
            // Fallback
            sb.append('"').append(escapeString(String.valueOf(v))).append('"');
        }
    }

    private static String escapeString(String s) {
        StringBuilder out = new StringBuilder(s.length() + 16);
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '"': out.append("\\\""); break;
                case '\\': out.append("\\\\"); break;
                case '\b': out.append("\\b"); break;
                case '\f': out.append("\\f"); break;
                case '\n': out.append("\\n"); break;
                case '\r': out.append("\\r"); break;
                case '\t': out.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        out.append(String.format("\\u%04x", (int) c));
                    } else {
                        out.append(c);
                    }
            }
        }
        return out.toString();
    }

    // Minimal JSON parser for objects, arrays, strings, numbers, booleans, null
    static class JSONParser {
        private final String s;
        private int i;

        JSONParser(String s) {
            this.s = s;
            this.i = 0;
        }

        Object parse() {
            skipWs();
            Object v = parseValue();
            skipWs();
            if (i != s.length()) {
                throw error("Trailing characters after JSON value");
            }
            return v;
        }

        private Object parseValue() {
            skipWs();
            if (i >= s.length()) throw error("Unexpected end of input");
            char c = s.charAt(i);
            switch (c) {
                case '{': return parseObject();
                case '[': return parseArray();
                case '"': return parseString();
                case 't': return parseTrue();
                case 'f': return parseFalse();
                case 'n': return parseNull();
                default:
                    if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
                    throw error("Unexpected character: " + c);
            }
        }

        private Map<String, Object> parseObject() {
            expect('{');
            skipWs();
            Map<String, Object> obj = new LinkedHashMap<>();
            if (peek('}')) {
                expect('}');
                return obj;
            }
            while (true) {
                skipWs();
                String key = parseString();
                skipWs();
                expect(':');
                skipWs();
                Object val = parseValue();
                obj.put(key, val);
                skipWs();
                if (peek('}')) {
                    expect('}');
                    break;
                }
                expect(',');
            }
            return obj;
        }

        private List<Object> parseArray() {
            expect('[');
            skipWs();
            List<Object> arr = new ArrayList<>();
            if (peek(']')) {
                expect(']');
                return arr;
            }
            while (true) {
                skipWs();
                Object val = parseValue();
                arr.add(val);
                skipWs();
                if (peek(']')) {
                    expect(']');
                    break;
                }
                expect(',');
            }
            return arr;
        }

        private String parseString() {
            expect('"');
            StringBuilder sb = new StringBuilder();
            while (i < s.length()) {
                char c = s.charAt(i++);
                if (c == '"') return sb.toString();
                if (c == '\\') {
                    if (i >= s.length()) throw error("Unterminated escape");
                    char e = s.charAt(i++);
                    switch (e) {
                        case '"': sb.append('"'); break;
                        case '\\': sb.append('\\'); break;
                        case '/': sb.append('/'); break;
                        case 'b': sb.append('\b'); break;
                        case 'f': sb.append('\f'); break;
                        case 'n': sb.append('\n'); break;
                        case 'r': sb.append('\r'); break;
                        case 't': sb.append('\t'); break;
                        case 'u':
                            if (i + 4 > s.length()) throw error("Invalid unicode escape");
                            int code = hex(s.charAt(i)) << 12 |
                                       hex(s.charAt(i + 1)) << 8 |
                                       hex(s.charAt(i + 2)) << 4 |
                                       hex(s.charAt(i + 3));
                            i += 4;
                            sb.append((char) code);
                            break;
                        default:
                            throw error("Invalid escape: \\" + e);
                    }
                } else {
                    sb.append(c);
                }
            }
            throw error("Unterminated string");
        }

        private int hex(char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
            if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
            throw error("Invalid hex digit: " + c);
        }

        private Object parseNumber() {
            int start = i;
            if (peek('-')) i++;
            if (peek('0')) {
                i++;
            } else {
                if (i >= s.length() || !isDigit(s.charAt(i))) throw error("Invalid number");
                while (i < s.length() && isDigit(s.charAt(i))) i++;
            }
            boolean isFloat = false;
            if (peek('.')) {
                isFloat = true;
                i++;
                if (i >= s.length() || !isDigit(s.charAt(i))) throw error("Invalid number fraction");
                while (i < s.length() && isDigit(s.charAt(i))) i++;
            }
            if (peek('e') || peek('E')) {
                isFloat = true;
                i++;
                if (peek('+') || peek('-')) i++;
                if (i >= s.length() || !isDigit(s.charAt(i))) throw error("Invalid number exponent");
                while (i < s.length() && isDigit(s.charAt(i))) i++;
            }
            String num = s.substring(start, i);
            try {
                if (isFloat) {
                    return Double.valueOf(num);
                } else {
                    // Try parse as Long, fallback to Double if overflow
                    long lv = Long.parseLong(num);
                    return lv;
                }
            } catch (NumberFormatException ex) {
                return Double.valueOf(num);
            }
        }

        private Boolean parseTrue() {
            expect('t'); expect('r'); expect('u'); expect('e');
            return Boolean.TRUE;
        }

        private Boolean parseFalse() {
            expect('f'); expect('a'); expect('l'); expect('s'); expect('e');
            return Boolean.FALSE;
        }

        private Object parseNull() {
            expect('n'); expect('u'); expect('l'); expect('l');
            return null;
        }

        private void skipWs() {
            while (i < s.length()) {
                char c = s.charAt(i);
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t') i++;
                else break;
            }
        }

        private void expect(char c) {
            if (i >= s.length() || s.charAt(i) != c) throw error("Expected '" + c + "'");
            i++;
        }

        private boolean peek(char c) {
            return i < s.length() && s.charAt(i) == c;
        }

        private boolean isDigit(char c) {
            return c >= '0' && c <= '9';
        }

        private RuntimeException error(String msg) {
            return new RuntimeException(msg + " at position " + i);
        }
    }

    public static void main(String[] args) {
        try {
            if (args != null && args.length > 0) {
                Object result = fetchJsonFromUrl(args[0]);
                System.out.println(toJsonString(result));
                return;
            }
            // 5 test cases
            String[] testUrls = new String[] {
                "https://jsonplaceholder.typicode.com/todos/1",
                "https://jsonplaceholder.typicode.com/posts/1",
                "https://jsonplaceholder.typicode.com/users/1",
                "https://jsonplaceholder.typicode.com/comments/1",
                "https://jsonplaceholder.typicode.com/albums/1"
            };
            for (String u : testUrls) {
                System.out.println("URL: " + u);
                Object result = fetchJsonFromUrl(u);
                System.out.println(toJsonString(result));
                System.out.println();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}