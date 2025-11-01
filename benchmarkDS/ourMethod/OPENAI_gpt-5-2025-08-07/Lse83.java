import java.nio.charset.StandardCharsets;
import java.util.*;

public class Lse83 {

    // Max limits
    private static final int MAX_PAYLOAD_BYTES = 8192;
    private static final int MAX_NAME_CHARS = 256;

    public static String processPayload(byte[] payload, String contentType) {
        if (payload == null) {
            return jsonError("invalid input");
        }
        if (payload.length > MAX_PAYLOAD_BYTES) {
            return jsonError("payload too large");
        }
        String body = new String(payload, StandardCharsets.UTF_8);
        String name = null;

        String ct = (contentType == null) ? "" : contentType.toLowerCase(Locale.ROOT);
        if (ct.contains("application/json") || (ct.isEmpty() && looksLikeJson(body))) {
            name = extractNameFromJson(body);
        } else if (ct.contains("application/x-www-form-urlencoded") || (ct.isEmpty() && body.contains("="))) {
            name = extractNameFromFormUrlEncoded(body);
        } else {
            // Try both safely if content-type is unknown
            name = extractNameFromJson(body);
            if (name == null) {
                name = extractNameFromFormUrlEncoded(body);
            }
        }

        if (name == null) {
            return jsonError("name field missing");
        }
        if (!isValidName(name)) {
            return jsonError("invalid name");
        }
        return "{\"name\":\"" + escapeJson(name) + "\"}";
    }

    private static boolean looksLikeJson(String s) {
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (!Character.isWhitespace(c)) {
                return c == '{' || c == '[';
            }
        }
        return false;
    }

    private static String extractNameFromFormUrlEncoded(String s) {
        // Parse key=value&key2=value2 ... with percent-decoding
        int i = 0;
        while (i <= s.length()) {
            int amp = s.indexOf('&', i);
            int end = (amp == -1) ? s.length() : amp;
            if (end > i) {
                String pair = s.substring(i, end);
                int eq = pair.indexOf('=');
                String key, val;
                if (eq == -1) {
                    key = pair;
                    val = "";
                } else {
                    key = pair.substring(0, eq);
                    val = pair.substring(eq + 1);
                }
                key = urlDecode(key);
                if (key != null && key.equals("name")) {
                    val = urlDecode(val);
                    if (val == null) return null;
                    return val;
                }
            }
            if (amp == -1) break;
            i = amp + 1;
        }
        return null;
    }

    private static String urlDecode(String s) {
        StringBuilder out = new StringBuilder(Math.min(1024, s.length()));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '+') {
                out.append(' ');
            } else if (c == '%') {
                if (i + 2 >= s.length()) return null;
                int hi = hexVal(s.charAt(i + 1));
                int lo = hexVal(s.charAt(i + 2));
                if (hi < 0 || lo < 0) return null;
                out.append((char) ((hi << 4) | lo));
                i += 2;
            } else {
                out.append(c);
            }
            if (out.length() > MAX_NAME_CHARS * 4) { // hard guard
                return null;
            }
        }
        return out.toString();
    }

    private static int hexVal(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    private static String extractNameFromJson(String s) {
        // Find unescaped "name" then parse a string value after :
        int idx = findUnescapedQuotedLiteral(s, "name", 0);
        if (idx < 0) return null;
        int afterKeyQuote = idx + 1 + "name".length() + 1; // starting quote + key + ending quote
        int i = afterKeyQuote;
        // Skip whitespace
        while (i < s.length() && Character.isWhitespace(s.charAt(i))) i++;
        if (i >= s.length() || s.charAt(i) != ':') return null;
        i++;
        while (i < s.length() && Character.isWhitespace(s.charAt(i))) i++;
        if (i >= s.length()) return null;
        if (s.charAt(i) != '"') {
            // Only accept string values
            return null;
        }
        i++;
        StringBuilder val = new StringBuilder();
        while (i < s.length()) {
            char c = s.charAt(i++);
            if (c == '\\') {
                if (i >= s.length()) return null;
                char e = s.charAt(i++);
                switch (e) {
                    case '"': val.append('"'); break;
                    case '\\': val.append('\\'); break;
                    case '/': val.append('/'); break;
                    case 'b': val.append('\b'); break;
                    case 'f': val.append('\f'); break;
                    case 'n': val.append('\n'); break;
                    case 'r': val.append('\r'); break;
                    case 't': val.append('\t'); break;
                    case 'u':
                        if (i + 4 > s.length()) return null;
                        int code = 0;
                        for (int k = 0; k < 4; k++) {
                            int hv = hexVal(s.charAt(i + k));
                            if (hv < 0) { code = -1; break; }
                            code = (code << 4) | hv;
                        }
                        i += 4;
                        if (code < 0) return null;
                        // Append as char if in BMP; Java uses UTF-16, so handle surrogate pairs simply
                        val.append((char) code);
                        break;
                    default:
                        return null;
                }
            } else if (c == '"') {
                break;
            } else {
                val.append(c);
            }
            if (val.length() > MAX_NAME_CHARS * 4) {
                return null;
            }
        }
        return val.toString();
    }

    private static int findUnescapedQuotedLiteral(String s, String literal, int start) {
        // Find "literal" where both quotes are unescaped
        for (int i = start; i < s.length(); i++) {
            int quotePos = s.indexOf('"', i);
            if (quotePos < 0) return -1;
            if (!isEscaped(s, quotePos)) {
                int end = quotePos + 1 + literal.length();
                if (end < s.length() && s.startsWith(literal, quotePos + 1)) {
                    if (s.charAt(end) == '"' && !isEscaped(s, end)) {
                        return quotePos;
                    }
                }
            }
            i = (quotePos < 0) ? s.length() : quotePos;
        }
        return -1;
    }

    private static boolean isEscaped(String s, int pos) {
        int backslashes = 0;
        for (int i = pos - 1; i >= 0 && s.charAt(i) == '\\'; i--) {
            backslashes++;
        }
        return (backslashes % 2) == 1;
    }

    private static boolean isValidName(String name) {
        if (name == null) return false;
        if (name.length() == 0) return false;
        if (name.length() > MAX_NAME_CHARS) return false;
        for (int i = 0; i < name.length(); i++) {
            char c = name.charAt(i);
            if (c < 0x20) { // control chars not allowed
                return false;
            }
        }
        return true;
    }

    private static String escapeJson(String s) {
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
                        out.append(String.format("\\u%04x", (int)c));
                    } else {
                        out.append(c);
                    }
            }
        }
        return out.toString();
    }

    private static String jsonError(String msg) {
        return "{\"error\":\"" + escapeJson(msg) + "\"}";
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] tests = new String[] {
            "{\"name\":\"Alice\"}",
            "{\"age\":30}",
            "name=Bob%20Builder&role=dev",
            "{\"name\":\"" + "x".repeat(300) + "\"}",
            "invalid_payload"
        };
        String[] cts = new String[] {
            "application/json",
            "application/json",
            "application/x-www-form-urlencoded",
            "application/json",
            ""
        };
        for (int i = 0; i < tests.length; i++) {
            String res = processPayload(tests[i].getBytes(StandardCharsets.UTF_8), cts[i]);
            System.out.println("Test " + (i + 1) + ": " + res);
        }
    }
}