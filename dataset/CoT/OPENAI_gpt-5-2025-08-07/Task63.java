import java.io.StringReader;
import java.math.BigDecimal;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.*;
import java.util.*;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import org.xml.sax.InputSource;

/*
Chain-of-Through process in code generation:
1) Problem understanding:
   - Implement a function that takes raw request data (bytes), ensures it's UTF-8 text containing a Base64 string.
   - Decode Base64 to recover original content (expected UTF-8 text of JSON or XML).
   - Deserialize to a structured representation and return it.
2) Security requirements:
   - Enforce UTF-8 validation.
   - Limit input sizes to prevent DoS.
   - Use strict Base64 decoding.
   - Disable XML external entities and DTDs (prevent XXE).
   - Implement bounded JSON parsing with depth limits and escape handling.
3) Secure coding generation:
   - Defensive checks, exceptions on invalid inputs.
   - Safe XML parsing configuration.
   - Strict, bounded JSON parser.
4) Code review:
   - Checked for unchecked external entity resolution.
   - Checked for unbounded recursion and input size.
   - Proper error handling and immutable charset usage.
5) Secure code output:
   - Final code below mitigates identified risks.
*/

public class Task63 {
    // Limits to mitigate resource abuse
    private static final int MAX_RAW_BYTES = 1_000_000;     // 1 MB raw UTF-8 bytes (Base64 text)
    private static final int MAX_DECODED_BYTES = 1_000_000; // 1 MB decoded payload
    private static final int MAX_JSON_DEPTH = 200;

    // Public API: process incoming request raw_data (UTF-8 bytes containing Base64)
    public static Object processRequest(byte[] rawDataUtf8) throws Exception {
        if (rawDataUtf8 == null) {
            throw new IllegalArgumentException("rawDataUtf8 is null");
        }
        if (rawDataUtf8.length == 0) {
            throw new IllegalArgumentException("rawDataUtf8 is empty");
        }
        if (rawDataUtf8.length > MAX_RAW_BYTES) {
            throw new IllegalArgumentException("rawDataUtf8 exceeds maximum allowed size");
        }

        // 1) Ensure raw_data is encoded in UTF-8 (strict validation)
        String base64Text = strictUtf8Decode(rawDataUtf8).trim();
        if (base64Text.isEmpty()) {
            throw new IllegalArgumentException("Base64 text is empty after trimming");
        }

        // 2) Decode the raw_data using Base64 (strict)
        byte[] decoded = strictBase64Decode(base64Text);
        if (decoded.length > MAX_DECODED_BYTES) {
            throw new IllegalArgumentException("Decoded data exceeds maximum allowed size");
        }

        // 3) Decode decoded bytes as UTF-8 (JSON/XML are text formats)
        String payload = strictUtf8Decode(decoded).trim();
        if (payload.isEmpty()) {
            throw new IllegalArgumentException("Decoded payload is empty");
        }

        // 4) Deserialize (JSON or XML). Store in variable.
        Object deserialized;
        String trimmed = ltrim(payload);
        if (trimmed.startsWith("{") || trimmed.startsWith("[")) {
            deserialized = new JsonParser(payload, MAX_JSON_DEPTH).parse();
        } else if (trimmed.startsWith("<")) {
            deserialized = parseXmlToMap(payload);
        } else {
            // If neither JSON nor XML, return the plain text payload as String
            deserialized = payload;
        }

        // 5) Return the stored data
        return deserialized;
    }

    // Helper: strict UTF-8 validation and decoding
    private static String strictUtf8Decode(byte[] data) {
        CharsetDecoder dec = StandardCharsets.UTF_8.newDecoder();
        dec.onMalformedInput(CodingErrorAction.REPORT);
        dec.onUnmappableCharacter(CodingErrorAction.REPORT);
        try {
            CharBuffer cb = dec.decode(ByteBuffer.wrap(data));
            return cb.toString();
        } catch (CharacterCodingException e) {
            throw new IllegalArgumentException("Input is not valid UTF-8", e);
        }
    }

    // Helper: strict Base64 with whitespace tolerance only
    private static byte[] strictBase64Decode(String base64) {
        // Remove ASCII whitespace to support multi-line base64 safely
        String cleaned = base64.replaceAll("\\s+", "");
        try {
            return java.util.Base64.getDecoder().decode(cleaned);
        } catch (IllegalArgumentException e) {
            throw new IllegalArgumentException("Invalid Base64 input", e);
        }
    }

    // Left trim utility
    private static String ltrim(String s) {
        int i = 0;
        while (i < s.length() && Character.isWhitespace(s.charAt(i))) i++;
        return s.substring(i);
    }

    // XML parsing with XXE prevention and conversion to Map structure
    private static Object parseXmlToMap(String xml) throws Exception {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();

        // Secure settings to prevent XXE
        dbf.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
        dbf.setFeature("http://xml.org/sax/features/external-general-entities", false);
        dbf.setFeature("http://xml.org/sax/features/external-parameter-entities", false);
        dbf.setXIncludeAware(false);
        dbf.setExpandEntityReferences(false);
        dbf.setNamespaceAware(true);

        DocumentBuilder builder = dbf.newDocumentBuilder();
        InputSource is = new InputSource(new StringReader(xml));
        Document doc = builder.parse(is);
        Element root = doc.getDocumentElement();
        return elementToMap(root);
    }

    private static Map<String, Object> elementToMap(Element elem) {
        Map<String, Object> map = new LinkedHashMap<>();
        map.put("tag", elem.getTagName());

        // attributes
        Map<String, String> attrs = new LinkedHashMap<>();
        NamedNodeMap nnm = elem.getAttributes();
        for (int i = 0; i < nnm.getLength(); i++) {
            Node attr = nnm.item(i);
            attrs.put(attr.getNodeName(), attr.getNodeValue());
        }
        map.put("attributes", attrs);

        // children and text
        List<Object> children = new ArrayList<>();
        StringBuilder textContent = new StringBuilder();
        NodeList nodes = elem.getChildNodes();
        for (int i = 0; i < nodes.getLength(); i++) {
            Node n = nodes.item(i);
            if (n instanceof Element) {
                children.add(elementToMap((Element) n));
            } else if (n.getNodeType() == Node.TEXT_NODE || n.getNodeType() == Node.CDATA_SECTION_NODE) {
                textContent.append(n.getTextContent());
            }
        }
        String text = textContent.toString().trim();
        map.put("text", text.isEmpty() ? "" : text);
        map.put("children", children);
        return map;
    }

    // Minimal secure JSON parser (objects, arrays, strings, numbers, booleans, null)
    private static final class JsonParser {
        private final String s;
        private final int n;
        private int i;
        private final int maxDepth;

        JsonParser(String s, int maxDepth) {
            this.s = s;
            this.n = s.length();
            this.i = 0;
            this.maxDepth = maxDepth;
        }

        Object parse() {
            skipWs();
            Object v = parseValue(0);
            skipWs();
            if (i != n) {
                throw new IllegalArgumentException("Trailing data after JSON value at position " + i);
            }
            return v;
        }

        private Object parseValue(int depth) {
            if (depth > maxDepth) throw new IllegalArgumentException("JSON nesting too deep");
            skipWs();
            if (i >= n) throw new IllegalArgumentException("Unexpected end of JSON input");
            char c = s.charAt(i);
            if (c == '{') return parseObject(depth + 1);
            if (c == '[') return parseArray(depth + 1);
            if (c == '"') return parseString();
            if (c == 't' || c == 'f') return parseBoolean();
            if (c == 'n') return parseNull();
            if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
            throw new IllegalArgumentException("Unexpected character '" + c + "' at position " + i);
        }

        private Map<String, Object> parseObject(int depth) {
            expect('{');
            LinkedHashMap<String, Object> obj = new LinkedHashMap<>();
            skipWs();
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
                Object val = parseValue(depth);
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

        private List<Object> parseArray(int depth) {
            expect('[');
            ArrayList<Object> arr = new ArrayList<>();
            skipWs();
            if (peek(']')) {
                expect(']');
                return arr;
            }
            while (true) {
                Object v = parseValue(depth);
                arr.add(v);
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
            while (i < n) {
                char c = s.charAt(i++);
                if (c == '"') {
                    return sb.toString();
                } else if (c == '\\') {
                    if (i >= n) throw new IllegalArgumentException("Invalid escape at end of string");
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
                            if (i + 4 > n) throw new IllegalArgumentException("Invalid unicode escape");
                            int code = hex4(s.charAt(i), s.charAt(i+1), s.charAt(i+2), s.charAt(i+3));
                            i += 4;
                            sb.append((char) code);
                            break;
                        default:
                            throw new IllegalArgumentException("Invalid escape character: \\" + e);
                    }
                } else {
                    // Control chars are not allowed in JSON strings
                    if (c <= 0x1F) throw new IllegalArgumentException("Unescaped control character in string");
                    sb.append(c);
                }
            }
            throw new IllegalArgumentException("Unterminated string");
        }

        private static int hex4(char a, char b, char c, char d) {
            return (hex(a) << 12) | (hex(b) << 8) | (hex(c) << 4) | hex(d);
        }

        private static int hex(char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            throw new IllegalArgumentException("Invalid hex digit: " + c);
        }

        private Object parseBoolean() {
            if (match("true")) return Boolean.TRUE;
            if (match("false")) return Boolean.FALSE;
            throw new IllegalArgumentException("Invalid boolean at position " + i);
        }

        private Object parseNull() {
            if (match("null")) return null;
            throw new IllegalArgumentException("Invalid null at position " + i);
        }

        private Object parseNumber() {
            int start = i;
            if (peek('-')) i++;
            if (i >= n) throw new IllegalArgumentException("Invalid number");
            if (s.charAt(i) == '0') {
                i++;
            } else if (s.charAt(i) >= '1' && s.charAt(i) <= '9') {
                while (i < n && Character.isDigit(s.charAt(i))) i++;
            } else {
                throw new IllegalArgumentException("Invalid number at position " + i);
            }
            if (i < n && s.charAt(i) == '.') {
                i++;
                if (i >= n || !Character.isDigit(s.charAt(i))) throw new IllegalArgumentException("Invalid fractional part");
                while (i < n && Character.isDigit(s.charAt(i))) i++;
            }
            if (i < n && (s.charAt(i) == 'e' || s.charAt(i) == 'E')) {
                i++;
                if (i < n && (s.charAt(i) == '+' || s.charAt(i) == '-')) i++;
                if (i >= n || !Character.isDigit(s.charAt(i))) throw new IllegalArgumentException("Invalid exponent part");
                while (i < n && Character.isDigit(s.charAt(i))) i++;
            }
            String num = s.substring(start, i);
            try {
                // Use BigDecimal for precision and safety
                return new BigDecimal(num);
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException("Invalid number format: " + num, e);
            }
        }

        private void skipWs() {
            while (i < n) {
                char c = s.charAt(i);
                if (c == ' ' || c == '\r' || c == '\n' || c == '\t') i++;
                else break;
            }
        }

        private boolean match(String kw) {
            int m = kw.length();
            if (i + m <= n && s.regionMatches(i, kw, 0, m)) {
                i += m;
                return true;
            }
            return false;
        }

        private boolean peek(char c) {
            return i < n && s.charAt(i) == c;
        }

        private void expect(char c) {
            if (i >= n || s.charAt(i) != c) {
                throw new IllegalArgumentException("Expected '" + c + "' at position " + i);
            }
            i++;
        }
    }

    // Utility to convert deserialized object to a readable string for demo
    private static String pretty(Object o) {
        StringBuilder sb = new StringBuilder();
        prettyRec(o, sb);
        return sb.toString();
    }
    @SuppressWarnings("unchecked")
    private static void prettyRec(Object o, StringBuilder sb) {
        if (o == null) {
            sb.append("null");
        } else if (o instanceof String) {
            sb.append('"').append(escape((String) o)).append('"');
        } else if (o instanceof BigDecimal) {
            sb.append(((BigDecimal) o).toPlainString());
        } else if (o instanceof Number || o instanceof Boolean) {
            sb.append(String.valueOf(o));
        } else if (o instanceof Map) {
            Map<String, Object> m = (Map<String, Object>) o;
            sb.append("{");
            boolean first = true;
            for (Map.Entry<String, Object> e : m.entrySet()) {
                if (!first) sb.append(", ");
                first = false;
                sb.append('"').append(escape(e.getKey())).append("\": ");
                prettyRec(e.getValue(), sb);
            }
            sb.append("}");
        } else if (o instanceof List) {
            List<Object> a = (List<Object>) o;
            sb.append("[");
            for (int i = 0; i < a.size(); i++) {
                if (i > 0) sb.append(", ");
                prettyRec(a.get(i), sb);
            }
            sb.append("]");
        } else {
            sb.append(String.valueOf(o));
        }
    }
    private static String escape(String s) {
        StringBuilder out = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '\\': out.append("\\\\"); break;
                case '"': out.append("\\\""); break;
                case '\b': out.append("\\b"); break;
                case '\f': out.append("\\f"); break;
                case '\n': out.append("\\n"); break;
                case '\r': out.append("\\r"); break;
                case '\t': out.append("\\t"); break;
                default:
                    if (c <= 0x1F) {
                        out.append(String.format("\\u%04x", (int) c));
                    } else {
                        out.append(c);
                    }
            }
        }
        return out.toString();
    }

    // Test harness with 5 test cases
    public static void main(String[] args) throws Exception {
        // Helper to build raw input: Base64(UTF-8(payload)) encoded as UTF-8 bytes
        java.util.function.Function<String, byte[]> mkInput = (String payload) ->
            Base64.getEncoder().encodeToString(payload.getBytes(StandardCharsets.UTF_8)).getBytes(StandardCharsets.UTF_8);

        // 1) JSON object
        String json1 = "{\"name\":\"Alice\",\"age\":30,\"active\":true}";
        // 2) JSON array
        String json2 = "[{\"id\":1},{\"id\":2},{\"id\":3}]";
        // 3) XML
        String xml1 = "<root attr=\"v\"><child>text</child><child2 x=\"1\"/></root>";
        // 4) Invalid Base64
        byte[] invalidB64 = "!!not-base64!!".getBytes(StandardCharsets.UTF_8);
        // 5) Non-UTF8 raw input bytes
        byte[] nonUtf8 = new byte[]{ (byte)0xC3, (byte)0x28 }; // invalid UTF-8 sequence

        byte[][] tests = new byte[][] {
            mkInput.apply(json1),
            mkInput.apply(json2),
            mkInput.apply(xml1),
            invalidB64,
            nonUtf8
        };

        for (int t = 0; t < tests.length; t++) {
            System.out.println("Test " + (t+1) + ":");
            try {
                Object result = processRequest(tests[t]);
                System.out.println(pretty(result));
            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
            }
            System.out.println();
        }
    }
}