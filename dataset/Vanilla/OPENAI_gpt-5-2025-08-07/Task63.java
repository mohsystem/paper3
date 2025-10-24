import java.util.*;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.io.StringWriter;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.XMLConstants;
import org.w3c.dom.Document;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.*;

public class Task63 {

    public static Object processRequest(byte[] rawDataUtf8Base64) throws Exception {
        String base64Str = new String(rawDataUtf8Base64, StandardCharsets.UTF_8);
        byte[] decodedBytes = Base64.getDecoder().decode(base64Str);
        String decodedStr = new String(decodedBytes, StandardCharsets.UTF_8).trim();
        Object deserialized = tryDeserialize(decodedStr);
        Object stored = deserialized; // store in variable
        return stored;
    }

    private static Object tryDeserialize(String s) throws Exception {
        String t = s.trim();
        if (t.isEmpty()) return t;
        char c = t.charAt(0);
        if (c == '{' || c == '[') {
            return new JsonParser(t).parse();
        } else if (c == '<') {
            return parseXml(t);
        } else {
            return t;
        }
    }

    private static Document parseXml(String xml) throws Exception {
        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        dbf.setNamespaceAware(true);
        dbf.setExpandEntityReferences(false);
        dbf.setXIncludeAware(false);
        dbf.setAttribute(XMLConstants.ACCESS_EXTERNAL_DTD, "");
        dbf.setAttribute(XMLConstants.ACCESS_EXTERNAL_SCHEMA, "");
        DocumentBuilder db = dbf.newDocumentBuilder();
        return db.parse(new java.io.ByteArrayInputStream(xml.getBytes(StandardCharsets.UTF_8)));
    }

    private static String documentToString(Document doc) {
        try {
            TransformerFactory tf = TransformerFactory.newInstance();
            tf.setAttribute(XMLConstants.ACCESS_EXTERNAL_DTD, "");
            tf.setAttribute(XMLConstants.ACCESS_EXTERNAL_STYLESHEET, "");
            Transformer transformer = tf.newTransformer();
            transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
            StringWriter sw = new StringWriter();
            transformer.transform(new DOMSource(doc), new StreamResult(sw));
            return sw.toString();
        } catch (Exception e) {
            return "XML(Document root=" + (doc.getDocumentElement() != null ? doc.getDocumentElement().getTagName() : "null") + ")";
        }
    }

    private static String toPrintable(Object obj) {
        if (obj == null) return "null";
        if (obj instanceof String) return (String) obj;
        if (obj instanceof Number || obj instanceof Boolean) return String.valueOf(obj);
        if (obj instanceof Map) {
            StringBuilder sb = new StringBuilder();
            sb.append("{");
            boolean first = true;
            for (Object eObj : ((Map<?, ?>) obj).entrySet()) {
                Map.Entry<?, ?> e = (Map.Entry<?, ?>) eObj;
                if (!first) sb.append(", ");
                first = false;
                sb.append(String.valueOf(e.getKey())).append(": ").append(toPrintable(e.getValue()));
            }
            sb.append("}");
            return sb.toString();
        }
        if (obj instanceof List) {
            StringBuilder sb = new StringBuilder();
            sb.append("[");
            boolean first = true;
            for (Object v : (List<?>) obj) {
                if (!first) sb.append(", ");
                first = false;
                sb.append(toPrintable(v));
            }
            sb.append("]");
            return sb.toString();
        }
        if (obj instanceof Document) {
            return documentToString((Document) obj);
        }
        return String.valueOf(obj);
    }

    private static String b64(String s) {
        return Base64.getEncoder().encodeToString(s.getBytes(StandardCharsets.UTF_8));
    }

    public static void main(String[] args) throws Exception {
        List<String> payloads = new ArrayList<>();
        payloads.add("{\"name\":\"Alice\",\"age\":30,\"skills\":[\"Java\",\"Python\"],\"active\":true}");
        payloads.add("<user><name>Bob</name><age>25</age></user>");
        payloads.add("[1,2,3,\"x\"]");
        payloads.add("Hello, world!");
        payloads.add("{\"a\":{\"b\":[1,{\"c\":\"d\"}]},\"n\":null}");

        int i = 1;
        for (String p : payloads) {
            String base64 = b64(p);
            Object out = processRequest(base64.getBytes(StandardCharsets.UTF_8));
            System.out.println("Test " + (i++) + ": " + toPrintable(out));
        }
    }

    // Minimal JSON parser for objects, arrays, strings, numbers, booleans, and null.
    static class JsonParser {
        private final String s;
        private int pos;

        JsonParser(String s) {
            this.s = s;
            this.pos = 0;
        }

        Object parse() {
            skipWs();
            Object v = parseValue();
            skipWs();
            if (pos != s.length()) throw error("Extra data after valid JSON");
            return v;
        }

        private Object parseValue() {
            skipWs();
            if (pos >= s.length()) throw error("Unexpected end of input");
            char c = s.charAt(pos);
            if (c == '{') return parseObject();
            if (c == '[') return parseArray();
            if (c == '"') return parseString();
            if (c == 't' || c == 'f') return parseBoolean();
            if (c == 'n') return parseNull();
            if (c == '-' || (c >= '0' && c <= '9')) return parseNumber();
            throw error("Invalid value");
        }

        private Map<String, Object> parseObject() {
            expect('{');
            Map<String, Object> obj = new LinkedHashMap<>();
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
            List<Object> arr = new ArrayList<>();
            skipWs();
            if (peek(']')) {
                expect(']');
                return arr;
            }
            while (true) {
                skipWs();
                arr.add(parseValue());
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
            while (pos < s.length()) {
                char c = s.charAt(pos++);
                if (c == '"') {
                    return sb.toString();
                } else if (c == '\\') {
                    if (pos >= s.length()) throw error("Invalid escape");
                    char e = s.charAt(pos++);
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
                            if (pos + 4 > s.length()) throw error("Invalid unicode escape");
                            String hex = s.substring(pos, pos + 4);
                            pos += 4;
                            try {
                                sb.append((char) Integer.parseInt(hex, 16));
                            } catch (NumberFormatException ex) {
                                throw error("Invalid unicode escape");
                            }
                            break;
                        default:
                            throw error("Invalid escape");
                    }
                } else {
                    sb.append(c);
                }
            }
            throw error("Unterminated string");
        }

        private Object parseNumber() {
            int start = pos;
            if (s.charAt(pos) == '-') pos++;
            if (pos >= s.length()) throw error("Invalid number");
            if (s.charAt(pos) == '0') {
                pos++;
            } else if (isDigit1to9(s.charAt(pos))) {
                while (pos < s.length() && isDigit(s.charAt(pos))) pos++;
            } else {
                throw error("Invalid number");
            }
            if (pos < s.length() && s.charAt(pos) == '.') {
                pos++;
                if (pos >= s.length() || !isDigit(s.charAt(pos))) throw error("Invalid number");
                while (pos < s.length() && isDigit(s.charAt(pos))) pos++;
            }
            if (pos < s.length() && (s.charAt(pos) == 'e' || s.charAt(pos) == 'E')) {
                pos++;
                if (pos < s.length() && (s.charAt(pos) == '+' || s.charAt(pos) == '-')) pos++;
                if (pos >= s.length() || !isDigit(s.charAt(pos))) throw error("Invalid number");
                while (pos < s.length() && isDigit(s.charAt(pos))) pos++;
            }
            String numStr = s.substring(start, pos);
            try {
                if (numStr.indexOf('.') >= 0 || numStr.indexOf('e') >= 0 || numStr.indexOf('E') >= 0) {
                    return Double.parseDouble(numStr);
                } else {
                    long lv = Long.parseLong(numStr);
                    if (lv >= Integer.MIN_VALUE && lv <= Integer.MAX_VALUE) return (int) lv;
                    return lv;
                }
            } catch (NumberFormatException e) {
                return Double.parseDouble(numStr);
            }
        }

        private Boolean parseBoolean() {
            if (s.startsWith("true", pos)) {
                pos += 4;
                return Boolean.TRUE;
            } else if (s.startsWith("false", pos)) {
                pos += 5;
                return Boolean.FALSE;
            }
            throw error("Invalid boolean");
        }

        private Object parseNull() {
            if (s.startsWith("null", pos)) {
                pos += 4;
                return null;
            }
            throw error("Invalid null");
        }

        private void skipWs() {
            while (pos < s.length()) {
                char c = s.charAt(pos);
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t') pos++;
                else break;
            }
        }

        private boolean peek(char ch) {
            return pos < s.length() && s.charAt(pos) == ch;
        }

        private void expect(char ch) {
            if (pos >= s.length() || s.charAt(pos) != ch) {
                throw error("Expected '" + ch + "'");
            }
            pos++;
        }

        private boolean isDigit(char c) {
            return c >= '0' && c <= '9';
        }

        private boolean isDigit1to9(char c) {
            return c >= '1' && c <= '9';
        }

        private RuntimeException error(String msg) {
            return new RuntimeException(msg + " at position " + pos);
        }
    }
}