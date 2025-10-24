import java.util.*;

public class Task131 {
    private static final int MAX_INPUT = 4096;
    private static final int MAX_FIELD_LEN = 256;

    static class ParseResult {
        String value;
        int index;
        boolean ok;
        ParseResult(String v, int i, boolean o) { value = v; index = i; ok = o; }
    }

    static class NumResult {
        long value;
        int index;
        boolean ok;
        NumResult(long v, int i, boolean o) { value = v; index = i; ok = o; }
    }

    private static boolean isWhitespace(char c) {
        return c == ' ' || c == '\n' || c == '\r' || c == '\t';
    }

    private static int skipWhitespace(String s, int i, int end) {
        while (i < end && isWhitespace(s.charAt(i))) i++;
        return i;
    }

    private static boolean isHex(char c) {
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    }

    private static ParseResult parseJsonString(String s, int i, int end) {
        if (i >= end || s.charAt(i) != '"') return new ParseResult(null, i, false);
        i++;
        StringBuilder sb = new StringBuilder();
        boolean esc = false;
        for (; i < end; i++) {
            char c = s.charAt(i);
            if (esc) {
                switch (c) {
                    case '"': sb.append('"'); break;
                    case '\\': sb.append('\\'); break;
                    case '/': sb.append('/'); break;
                    case 'b': sb.append('\b'); break;
                    case 'f': sb.append('\f'); break;
                    case 'n': sb.append('\n'); break;
                    case 'r': sb.append('\r'); break;
                    case 't': sb.append('\t'); break;
                    case 'u':
                        // For safety and simplicity, reject \u escapes to avoid complex decoding
                        return new ParseResult(null, i, false);
                    default:
                        return new ParseResult(null, i, false);
                }
                esc = false;
            } else {
                if (c == '\\') {
                    esc = true;
                } else if (c == '"') {
                    i++;
                    String val = sb.toString();
                    if (val.length() > MAX_FIELD_LEN) return new ParseResult(null, i, false);
                    return new ParseResult(val, i, true);
                } else {
                    // Avoid control characters
                    if (c < 0x20) return new ParseResult(null, i, false);
                    sb.append(c);
                }
            }
        }
        return new ParseResult(null, i, false);
    }

    private static NumResult parseJsonInt(String s, int i, int end) {
        int start = i;
        boolean neg = false;
        if (i < end && (s.charAt(i) == '+' || s.charAt(i) == '-')) {
            neg = s.charAt(i) == '-';
            i++;
        }
        if (i >= end || !Character.isDigit(s.charAt(i))) return new NumResult(0, i, false);
        long val = 0;
        while (i < end && Character.isDigit(s.charAt(i))) {
            int d = s.charAt(i) - '0';
            val = val * 10 + d;
            if (val > Integer.MAX_VALUE) return new NumResult(0, i, false);
            i++;
        }
        if (neg) val = -val;
        return new NumResult(val, i, true);
    }

    private static String jsonEscape(String s) {
        StringBuilder sb = new StringBuilder();
        sb.append('"');
        for (int j = 0; j < s.length(); j++) {
            char c = s.charAt(j);
            switch (c) {
                case '"': sb.append("\\\""); break;
                case '\\': sb.append("\\\\"); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        // Escape as \u00XX
                        String hex = Integer.toHexString(c);
                        sb.append("\\u");
                        for (int k = hex.length(); k < 4; k++) sb.append('0');
                        sb.append(hex);
                    } else {
                        sb.append(c);
                    }
            }
        }
        sb.append('"');
        return sb.toString();
    }

    private static boolean isValidEmail(String email) {
        if (email == null) return false;
        if (email.length() < 3 || email.length() > MAX_FIELD_LEN) return false;
        int at = email.indexOf('@');
        if (at <= 0 || at >= email.length() - 1) return false;
        if (email.indexOf('@', at + 1) != -1) return false; // only one @
        int dot = email.indexOf('.', at + 1);
        if (dot <= at + 1 || dot >= email.length() - 1) return false;
        for (int i = 0; i < email.length(); i++) {
            char c = email.charAt(i);
            if (c <= 0x1F || c >= 0x7F) return false;
            boolean ok = Character.isLetterOrDigit(c) || c == '.' || c == '-' || c == '_' || c == '+' || c == '@';
            if (!ok) return false;
        }
        return true;
    }

    public static String secureDeserialize(String input) {
        if (input == null) return "";
        if (input.length() > MAX_INPUT) return "";
        int n = input.length();
        int i = 0;
        i = skipWhitespace(input, i, n);
        if (i >= n || input.charAt(i) != '{') return "";
        i++;
        int end = n;
        end = skipWhitespace(input, 0, n);
        // we should trim trailing whitespace for end-1 check
        int j = n - 1;
        while (j >= 0 && isWhitespace(input.charAt(j))) j--;
        if (j < 0) return "";
        if (input.charAt(j) != '}') return "";
        // We'll parse until we hit the closing '}' manually; set limit as j
        int limit = j;
        boolean idSet = false, nameSet = false, emailSet = false, ageSet = false;
        int idVal = 0, ageVal = 0;
        String nameVal = null, emailVal = null;
        boolean expectPair = true;
        boolean first = true;

        i = skipWhitespace(input, i, limit);
        if (i < limit && input.charAt(i) == '}') {
            // empty object not allowed
            return "";
        }

        while (i < limit) {
            i = skipWhitespace(input, i, limit);
            ParseResult keyRes = parseJsonString(input, i, limit);
            if (!keyRes.ok) return "";
            String key = keyRes.value;
            i = keyRes.index;
            i = skipWhitespace(input, i, limit);
            if (i >= limit || input.charAt(i) != ':') return "";
            i++;
            i = skipWhitespace(input, i, limit);

            if ("id".equals(key) || "age".equals(key)) {
                NumResult nr = parseJsonInt(input, i, limit);
                if (!nr.ok) return "";
                long v = nr.value;
                if (v < 0 || v > Integer.MAX_VALUE) return "";
                if ("id".equals(key)) {
                    if (idSet) return "";
                    idVal = (int)v; idSet = true;
                } else {
                    if (ageSet) return "";
                    ageVal = (int)v; ageSet = true;
                }
                i = nr.index;
            } else if ("name".equals(key) || "email".equals(key)) {
                ParseResult vr = parseJsonString(input, i, limit);
                if (!vr.ok) return "";
                if ("name".equals(key)) {
                    if (nameSet) return "";
                    if (vr.value.length() == 0) return "";
                    nameVal = vr.value; nameSet = true;
                } else {
                    if (emailSet) return "";
                    if (!isValidEmail(vr.value)) return "";
                    emailVal = vr.value; emailSet = true;
                }
                i = vr.index;
            } else {
                return "";
            }

            i = skipWhitespace(input, i, limit);
            if (i < limit && input.charAt(i) == ',') {
                i++;
                continue;
            } else if (i == limit) {
                break;
            } else {
                // unexpected char before closing brace
                return "";
            }
        }

        // Must end at limit which is the position of '}'
        if (i != limit) return "";

        if (!(idSet && nameSet && emailSet)) return "";

        StringBuilder out = new StringBuilder();
        out.append('{');
        out.append("\"id\":").append(idVal).append(',');
        out.append("\"name\":").append(jsonEscape(nameVal)).append(',');
        out.append("\"email\":").append(jsonEscape(emailVal));
        if (ageSet) {
            out.append(",\"age\":").append(ageVal);
        }
        out.append('}');
        return out.toString();
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "{\"id\":123,\"name\":\"Alice\",\"email\":\"alice@example.com\",\"age\":30}",
            "{\"id\":1,\"name\":\"Bob\",\"email\":\"bob@example.com\"}",
            "{\"id\":2,\"name\":\"Mallory\",\"email\":\"m@ex.com\",\"role\":\"admin\"}",
            "{\"id\":3,\"name\":\"Eve\",\"email\":\"eve@example.com\",\"age\":\"30\"}",
            "{\"id\":4,\"name\":\"A \\\"quoted\\\" Name\",\"email\":\"a.q@example.com\",\"age\":45}"
        };
        for (String t : tests) {
            String res = secureDeserialize(t);
            System.out.println(res.isEmpty() ? "INVALID" : res);
        }
    }
}