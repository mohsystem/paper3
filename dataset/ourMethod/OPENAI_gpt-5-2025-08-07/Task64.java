import java.io.IOException;
import java.math.BigDecimal;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class Task64 {

    // Secure JSON fetcher: requires HTTPS, validates host via default SSL/TLS, limits size, parses JSON.
    public static Object fetchJson(String urlStr) throws Exception {
        validateUrl(urlStr);
        URI uri = toSecureHttpsUri(urlStr);

        HttpClient client = HttpClient.newBuilder()
                .version(HttpClient.Version.HTTP_2)
                .followRedirects(HttpClient.Redirect.NORMAL)
                .connectTimeout(Duration.ofSeconds(10))
                .build();

        HttpRequest request = HttpRequest.newBuilder(uri)
                .timeout(Duration.ofSeconds(20))
                .header("Accept", "application/json, text/plain, */*")
                .header("User-Agent", "Task64/1.0")
                .GET()
                .build();

        HttpResponse<byte[]> response = client.send(request, HttpResponse.BodyHandlers.ofByteArray());
        int status = response.statusCode();
        if (status < 200 || status >= 300) {
            throw new IOException("HTTP error: " + status);
        }

        final long MAX_BYTES = 5_000_000L; // 5 MB limit
        long contentLength = response.headers()
                .firstValue("Content-Length")
                .map(Task64::safeParseLong)
                .orElse(-1L);
        if (contentLength > MAX_BYTES) {
            throw new IOException("Response too large");
        }

        byte[] body = response.body();
        if (body == null) {
            throw new IOException("Empty response");
        }
        if (body.length > MAX_BYTES) {
            throw new IOException("Response exceeds size limit");
        }

        String jsonText = new String(body, StandardCharsets.UTF_8);
        MiniJsonParser parser = new MiniJsonParser(jsonText);
        return parser.parse();
    }

    private static long safeParseLong(String s) {
        try {
            return Long.parseLong(s.trim());
        } catch (Exception e) {
            return -1L;
        }
    }

    private static void validateUrl(String url) {
        if (url == null) {
            throw new IllegalArgumentException("URL is null");
        }
        String u = url.trim();
        if (u.isEmpty()) {
            throw new IllegalArgumentException("URL is empty");
        }
        if (u.length() > 2048) {
            throw new IllegalArgumentException("URL too long");
        }
    }

    private static URI toSecureHttpsUri(String urlStr) throws URISyntaxException {
        URI uri = new URI(urlStr);
        String scheme = uri.getScheme();
        if (scheme == null || !scheme.equalsIgnoreCase("https")) {
            throw new IllegalArgumentException("Only HTTPS URLs are allowed");
        }
        String host = uri.getHost();
        if (host == null || host.isEmpty()) {
            throw new IllegalArgumentException("Invalid host");
        }
        return uri.normalize();
    }

    // Minimal JSON parser for objects, arrays, strings, numbers, booleans, null.
    static final class MiniJsonParser {
        private final String s;
        private int pos = 0;

        MiniJsonParser(String s) {
            this.s = s;
        }

        Object parse() {
            skipWs();
            Object v = parseValue();
            skipWs();
            if (pos != s.length()) {
                throw new IllegalArgumentException("Trailing characters at position " + pos);
            }
            return v;
        }

        private Object parseValue() {
            skipWs();
            if (pos >= s.length()) throw new IllegalArgumentException("Unexpected end of input");
            char c = s.charAt(pos);
            switch (c) {
                case '{': return parseObject();
                case '[': return parseArray();
                case '"': return parseString();
                case 't': return parseLiteral("true", Boolean.TRUE);
                case 'f': return parseLiteral("false", Boolean.FALSE);
                case 'n': return parseLiteral("null", null);
                default:
                    if (c == '-' || (c >= '0' && c <= '9')) {
                        return parseNumber();
                    }
                    throw new IllegalArgumentException("Unexpected character '" + c + "' at " + pos);
            }
        }

        private Map<String, Object> parseObject() {
            expect('{');
            LinkedHashMap<String, Object> map = new LinkedHashMap<>();
            skipWs();
            if (peek('}')) {
                expect('}');
                return map;
            }
            while (true) {
                skipWs();
                String key = parseString();
                skipWs();
                expect(':');
                skipWs();
                Object val = parseValue();
                map.put(key, val);
                skipWs();
                if (peek('}')) {
                    expect('}');
                    break;
                }
                expect(',');
            }
            return map;
        }

        private List<Object> parseArray() {
            expect('[');
            ArrayList<Object> list = new ArrayList<>();
            skipWs();
            if (peek(']')) {
                expect(']');
                return list;
            }
            while (true) {
                skipWs();
                Object val = parseValue();
                list.add(val);
                skipWs();
                if (peek(']')) {
                    expect(']');
                    break;
                }
                expect(',');
            }
            return list;
        }

        private String parseString() {
            expect('"');
            StringBuilder sb = new StringBuilder();
            while (pos < s.length()) {
                char c = s.charAt(pos++);
                if (c == '"') {
                    return sb.toString();
                } else if (c == '\\') {
                    if (pos >= s.length()) throw new IllegalArgumentException("Unterminated escape");
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
                            if (pos + 4 > s.length()) throw new IllegalArgumentException("Invalid unicode escape");
                            int code = hex4(s.substring(pos, pos + 4));
                            sb.append((char) code);
                            pos += 4;
                            break;
                        default:
                            throw new IllegalArgumentException("Invalid escape \\" + e + " at " + (pos - 1));
                    }
                } else {
                    if (c < 0x20) throw new IllegalArgumentException("Control char in string");
                    sb.append(c);
                }
            }
            throw new IllegalArgumentException("Unterminated string");
        }

        private static int hex4(String h) {
            try {
                return Integer.parseInt(h, 16);
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException("Invalid hex: " + h);
            }
        }

        private Object parseNumber() {
            int start = pos;
            char c = s.charAt(pos);
            if (c == '-') pos++;
            int intStart = pos;
            if (pos >= s.length()) throw new IllegalArgumentException("Invalid number at end");
            if (s.charAt(pos) == '0') {
                pos++;
            } else if (s.charAt(pos) >= '1' && s.charAt(pos) <= '9') {
                while (pos < s.length() && Character.isDigit(s.charAt(pos))) pos++;
            } else {
                throw new IllegalArgumentException("Invalid number at " + pos);
            }
            boolean isFloat = false;
            if (pos < s.length() && s.charAt(pos) == '.') {
                isFloat = true;
                pos++;
                int fracStart = pos;
                while (pos < s.length() && Character.isDigit(s.charAt(pos))) pos++;
                if (pos == fracStart) throw new IllegalArgumentException("Invalid fraction at " + pos);
            }
            if (pos < s.length() && (s.charAt(pos) == 'e' || s.charAt(pos) == 'E')) {
                isFloat = true;
                pos++;
                if (pos < s.length() && (s.charAt(pos) == '+' || s.charAt(pos) == '-')) pos++;
                int expStart = pos;
                while (pos < s.length() && Character.isDigit(s.charAt(pos))) pos++;
                if (pos == expStart) throw new IllegalArgumentException("Invalid exponent at " + pos);
            }
            String num = s.substring(start, pos);
            try {
                if (isFloat) {
                    return new BigDecimal(num);
                } else {
                    // Fit into long if possible, else BigDecimal
                    long l = Long.parseLong(num);
                    return BigDecimal.valueOf(l);
                }
            } catch (NumberFormatException e) {
                return new BigDecimal(num);
            }
        }

        private Object parseLiteral(String lit, Object value) {
            int end = pos + lit.length();
            if (end <= s.length() && s.regionMatches(pos, lit, 0, lit.length())) {
                pos += lit.length();
                return value;
            }
            throw new IllegalArgumentException("Expected '" + lit + "' at " + pos);
        }

        private void expect(char ch) {
            if (pos >= s.length() || s.charAt(pos) != ch) {
                throw new IllegalArgumentException("Expected '" + ch + "' at " + pos);
            }
            pos++;
        }

        private boolean peek(char ch) {
            return pos < s.length() && s.charAt(pos) == ch;
        }

        private void skipWs() {
            while (pos < s.length()) {
                char c = s.charAt(pos);
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                    pos++;
                } else {
                    break;
                }
            }
        }
    }

    // Utility to render parsed JSON back to text (for testing/printing).
    public static String toJsonString(Object v) {
        if (v == null) return "null";
        if (v instanceof String) return quote((String) v);
        if (v instanceof Boolean) return v.toString();
        if (v instanceof BigDecimal) return ((BigDecimal) v).toPlainString();
        if (v instanceof List) {
            StringBuilder sb = new StringBuilder();
            sb.append('[');
            List<?> list = (List<?>) v;
            for (int i = 0; i < list.size(); i++) {
                if (i > 0) sb.append(',');
                sb.append(toJsonString(list.get(i)));
            }
            sb.append(']');
            return sb.toString();
        }
        if (v instanceof Map) {
            StringBuilder sb = new StringBuilder();
            sb.append('{');
            boolean first = true;
            for (Map.Entry<?, ?> e : ((Map<?, ?>) v).entrySet()) {
                if (!first) sb.append(',');
                first = false;
                sb.append(quote(String.valueOf(e.getKey()))).append(':').append(toJsonString(e.getValue()));
            }
            sb.append('}');
            return sb.toString();
        }
        return quote(String.valueOf(v));
    }

    private static String quote(String s) {
        StringBuilder sb = new StringBuilder();
        sb.append('"');
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
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

    public static void main(String[] args) {
        List<String> tests = new ArrayList<>();
        if (args != null && args.length >= 1) {
            tests.add(args[0]);
        } else {
            tests.add("https://jsonplaceholder.typicode.com/todos/1");
            tests.add("https://httpbin.org/json");
            tests.add("https://api.github.com");
            tests.add("https://raw.githubusercontent.com/typicode/demo/master/db.json");
            tests.add("https://catfact.ninja/fact");
        }
        for (String url : tests) {
            System.out.println("URL: " + url);
            try {
                Object obj = fetchJson(url);
                String out = toJsonString(obj);
                if (out.length() > 300) {
                    System.out.println(out.substring(0, 300) + "... (" + out.length() + " chars)");
                } else {
                    System.out.println(out);
                }
            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
            }
            System.out.println("----");
        }
    }
}