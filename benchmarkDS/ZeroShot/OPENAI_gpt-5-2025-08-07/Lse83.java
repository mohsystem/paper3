import java.nio.charset.StandardCharsets;

public class Lse83 {
    private static final int MAX_BODY_SIZE = 10 * 1024; // 10KB

    public static String handleRequest(String contentType, byte[] body) {
        if (contentType == null || body == null) return "";
        String ct = contentType.trim().toLowerCase();
        if (!ct.startsWith("application/json")) return "";
        if (body.length == 0 || body.length > MAX_BODY_SIZE) return "";
        String json;
        try {
            json = new String(body, StandardCharsets.UTF_8);
        } catch (Exception e) {
            return "";
        }
        String name = extractNameFromJson(json);
        return name != null ? name : "";
    }

    private static String extractNameFromJson(String json) {
        int i = 0, n = json.length();
        while (i < n) {
            i = skipWs(json, i, n);
            if (i >= n) break;
            if (json.charAt(i) == '"') {
                ParseResult keyRes = parseJsonString(json, i);
                if (!keyRes.ok) return null;
                i = keyRes.nextIdx;
                int j = skipWs(json, i, n);
                if (j < n && json.charAt(j) == ':') {
                    j++;
                    j = skipWs(json, j, n);
                    if ("name".equals(keyRes.value)) {
                        if (j < n && json.charAt(j) == '"') {
                            ParseResult valRes = parseJsonString(json, j);
                            if (valRes.ok) return valRes.value;
                            return null;
                        } else {
                            // Value for 'name' is not a string; ignore per spec and continue
                        }
                    }
                    // Skip value token safely to continue scanning
                    int[] skipRes = skipJsonValue(json, j, n);
                    if (skipRes[0] == 0) return null;
                    i = skipRes[1];
                } else {
                    i = j;
                }
            } else {
                i++;
            }
        }
        return null;
    }

    private static int skipWs(String s, int i, int n) {
        while (i < n) {
            char c = s.charAt(i);
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') i++;
            else break;
        }
        return i;
    }

    private static class ParseResult {
        boolean ok;
        String value;
        int nextIdx;
        ParseResult(boolean ok, String value, int nextIdx) { this.ok = ok; this.value = value; this.nextIdx = nextIdx; }
    }

    private static ParseResult parseJsonString(String s, int startIdx) {
        int n = s.length();
        if (startIdx >= n || s.charAt(startIdx) != '"') return new ParseResult(false, null, startIdx);
        StringBuilder out = new StringBuilder();
        int i = startIdx + 1;
        while (i < n) {
            char c = s.charAt(i);
            if (c == '"') {
                return new ParseResult(true, out.toString(), i + 1);
            } else if (c == '\\') {
                if (i + 1 >= n) return new ParseResult(false, null, i);
                char esc = s.charAt(i + 1);
                switch (esc) {
                    case '"': out.append('"'); i += 2; break;
                    case '\\': out.append('\\'); i += 2; break;
                    case '/': out.append('/'); i += 2; break;
                    case 'b': out.append('\b'); i += 2; break;
                    case 'f': out.append('\f'); i += 2; break;
                    case 'n': out.append('\n'); i += 2; break;
                    case 'r': out.append('\r'); i += 2; break;
                    case 't': out.append('\t'); i += 2; break;
                    case 'u':
                        if (i + 6 > n) return new ParseResult(false, null, i);
                        String hex = s.substring(i + 2, i + 6);
                        try {
                            int codePoint = Integer.parseInt(hex, 16);
                            out.append(new String(Character.toChars(codePoint)));
                        } catch (Exception e) {
                            return new ParseResult(false, null, i);
                        }
                        i += 6;
                        break;
                    default:
                        return new ParseResult(false, null, i);
                }
            } else {
                out.append(c);
                i++;
            }
        }
        return new ParseResult(false, null, i);
    }

    // Returns [okFlag, nextIdx]
    private static int[] skipJsonValue(String s, int i, int n) {
        i = skipWs(s, i, n);
        if (i >= n) return new int[]{0, i};
        char c = s.charAt(i);
        if (c == '"') {
            ParseResult r = parseJsonString(s, i);
            if (!r.ok) return new int[]{0, i};
            return new int[]{1, r.nextIdx};
        } else if (c == '{') {
            int depth = 1; i++;
            while (i < n && depth > 0) {
                char ch = s.charAt(i);
                if (ch == '"') {
                    ParseResult r = parseJsonString(s, i);
                    if (!r.ok) return new int[]{0, i};
                    i = r.nextIdx;
                } else if (ch == '{') { depth++; i++; }
                else if (ch == '}') { depth--; i++; }
                else i++;
            }
            return new int[]{depth == 0 ? 1 : 0, i};
        } else if (c == '[') {
            int depth = 1; i++;
            while (i < n && depth > 0) {
                char ch = s.charAt(i);
                if (ch == '"') {
                    ParseResult r = parseJsonString(s, i);
                    if (!r.ok) return new int[]{0, i};
                    i = r.nextIdx;
                } else if (ch == '[') { depth++; i++; }
                else if (ch == ']') { depth--; i++; }
                else i++;
            }
            return new int[]{depth == 0 ? 1 : 0, i};
        } else {
            // number, true, false, null
            int j = i;
            while (j < n) {
                char ch = s.charAt(j);
                if (Character.isLetterOrDigit(ch) || ch == '+' || ch == '-' || ch == '.' || ch == 'e' || ch == 'E') {
                    j++;
                } else break;
            }
            if (j == i) return new int[]{0, i};
            return new int[]{1, j};
        }
    }

    public static void main(String[] args) {
        String ct = "application/json";
        String[] tests = new String[] {
            "{\"name\": \"Alice\"}",
            "{\"id\": 123, \"name\": \"Bob \\\\ \\\"The Builder\\\"\", \"extra\": true}",
            "{\"id\": 7, \"names\": [\"x\", \"y\"]}",
            "{malformed json",
            "{\"nested\": {\"name\": \"Carol\"}, \"other\": 1}"
        };
        for (int t = 0; t < tests.length; t++) {
            String res = handleRequest(ct, tests[t].getBytes(StandardCharsets.UTF_8));
            System.out.println("Test " + (t+1) + " => " + res);
        }
    }
}