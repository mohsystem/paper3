import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.*;

public class Task64 {

    // Public API: fetch JSON from a URL string and return as Java objects (Map/List/String/Number/Boolean/null)
    public static Object fetchJsonFromUrl(String urlStr) throws Exception {
        final int MAX_BYTES = 5 * 1024 * 1024; // 5 MB limit
        final int CONNECT_TIMEOUT_MS = 10000;
        final int READ_TIMEOUT_MS = 15000;
        final int MAX_REDIRECTS = 5;

        URL currentUrl = new URL(urlStr);
        for (int redirectCount = 0; redirectCount <= MAX_REDIRECTS; redirectCount++) {
            // Validate scheme
            String protocol = currentUrl.getProtocol();
            if (!"http".equalsIgnoreCase(protocol) && !"https".equalsIgnoreCase(protocol)) {
                throw new IOException("Only http and https schemes are allowed");
            }
            // Validate host safety (anti-SSRF)
            assertSafeHost(currentUrl);

            HttpURLConnection conn = (HttpURLConnection) currentUrl.openConnection();
            conn.setInstanceFollowRedirects(false);
            conn.setConnectTimeout(CONNECT_TIMEOUT_MS);
            conn.setReadTimeout(READ_TIMEOUT_MS);
            conn.setRequestMethod("GET");
            conn.setRequestProperty("User-Agent", "Task64/1.0 (+https://example.com/security)");
            conn.setRequestProperty("Accept", "application/json, text/plain;q=0.5, */*;q=0.1");

            int code;
            try {
                code = conn.getResponseCode();
            } catch (IOException ex) {
                // Attempt to read error stream to consume it, then rethrow
                try (InputStream es = conn.getErrorStream()) {
                    if (es != null) {
                        drainWithLimit(es, 8192);
                    }
                } catch (Exception ignore) {}
                throw ex;
            }

            if (code >= 300 && code <= 399) {
                String location = conn.getHeaderField("Location");
                if (location == null || location.isEmpty()) {
                    throw new IOException("Redirect without Location header");
                }
                URL newUrl = new URL(currentUrl, location);
                currentUrl = newUrl;
                // continue loop with next redirect step
                continue;
            } else if (code == HttpURLConnection.HTTP_OK) {
                long contentLength = conn.getContentLengthLong();
                if (contentLength > MAX_BYTES) {
                    throw new IOException("Content-Length exceeds limit");
                }
                byte[] data;
                try (InputStream is = new BufferedInputStream(conn.getInputStream())) {
                    data = readAllWithLimit(is, MAX_BYTES);
                }
                String text = new String(data, StandardCharsets.UTF_8);
                return Json.parse(text);
            } else {
                // Read and discard error to free connection
                try (InputStream es = conn.getErrorStream()) {
                    if (es != null) {
                        drainWithLimit(es, 8192);
                    }
                } catch (Exception ignore) {}
                throw new IOException("HTTP error: " + code);
            }
        }
        throw new IOException("Too many redirects");
    }

    private static void assertSafeHost(URL url) throws Exception {
        String host = url.getHost();
        if (host == null || host.isEmpty()) {
            throw new IOException("URL host is empty");
        }
        String lowerHost = host.toLowerCase(Locale.ROOT);
        if ("localhost".equals(lowerHost)) {
            throw new IOException("Access to localhost is not allowed");
        }
        InetAddress[] addrs = InetAddress.getAllByName(host);
        if (addrs == null || addrs.length == 0) {
            throw new IOException("Could not resolve host");
        }
        for (InetAddress addr : addrs) {
            if (!isPublicAddress(addr)) {
                throw new IOException("Host resolves to a non-public address: " + addr.getHostAddress());
            }
        }
    }

    private static boolean isPublicAddress(InetAddress addr) {
        // Basic checks
        if (addr.isAnyLocalAddress()
                || addr.isLoopbackAddress()
                || addr.isLinkLocalAddress()
                || addr.isSiteLocalAddress()
                || addr.isMulticastAddress()) {
            return false;
        }
        byte[] raw = addr.getAddress();
        if (addr instanceof java.net.Inet6Address) {
            if (raw == null || raw.length != 16) {
                return false;
            }
            int b0 = raw[0] & 0xFF;
            // Unique local IPv6 fc00::/7
            if ((b0 & 0xFE) == 0xFC) {
                return false;
            }
            // Unspecified ::/128
            boolean allZero = true;
            for (byte b : raw) {
                if (b != 0) { allZero = false; break; }
            }
            if (allZero) return false;
            // Loopback ::1/128 already caught by isLoopbackAddress
            return true;
        } else {
            if (raw == null || raw.length != 4) {
                return false;
            }
            int a = raw[0] & 0xFF;
            int b = raw[1] & 0xFF;
            int c = raw[2] & 0xFF;
            int d = raw[3] & 0xFF;
            // 0.0.0.0/8
            if (a == 0) return false;
            // 10.0.0.0/8
            if (a == 10) return false;
            // 127.0.0.0/8
            if (a == 127) return false;
            // 169.254.0.0/16
            if (a == 169 && b == 254) return false;
            // 172.16.0.0/12
            if (a == 172 && (b >= 16 && b <= 31)) return false;
            // 192.168.0.0/16
            if (a == 192 && b == 168) return false;
            // 100.64.0.0/10
            if (a == 100 && (b >= 64 && b <= 127)) return false;
            // 192.0.0.0/24
            if (a == 192 && b == 0 && c == 0) return false;
            // 198.18.0.0/15 (benchmark)
            if (a == 198 && (b == 18 || b == 19)) return false;
            // 224.0.0.0/4 multicast, 240.0.0.0/4 reserved
            if (a >= 224) return false;
            // Broadcast 255.255.255.255
            if (a == 255 && b == 255 && c == 255 && d == 255) return false;
            return true;
        }
    }

    private static void drainWithLimit(InputStream is, int limit) throws IOException {
        byte[] buf = new byte[4096];
        int total = 0, r;
        while ((r = is.read(buf, 0, Math.min(buf.length, limit - total))) > 0) {
            total += r;
            if (total >= limit) break;
        }
    }

    private static byte[] readAllWithLimit(InputStream is, int maxBytes) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream(Math.min(maxBytes, 8192));
        byte[] buf = new byte[8192];
        int total = 0;
        int r;
        while ((r = is.read(buf)) != -1) {
            total += r;
            if (total > maxBytes) throw new IOException("Response too large");
            bos.write(buf, 0, r);
        }
        return bos.toByteArray();
    }

    // Minimal JSON parser and serializer
    static final class Json {
        static Object parse(String s) {
            if (s == null) throw new IllegalArgumentException("Input is null");
            Parser p = new Parser(s);
            Object v = p.parseValue();
            p.skipWs();
            if (!p.eof()) {
                throw new IllegalArgumentException("Trailing data at position " + p.pos);
            }
            return v;
        }

        static String stringify(Object v) {
            StringBuilder sb = new StringBuilder();
            writeJson(sb, v);
            return sb.toString();
        }

        private static void writeJson(StringBuilder sb, Object v) {
            if (v == null) {
                sb.append("null");
            } else if (v instanceof String) {
                writeString(sb, (String) v);
            } else if (v instanceof Number) {
                if (v instanceof Double) {
                    double d = (Double) v;
                    if (Double.isNaN(d) || Double.isInfinite(d)) {
                        sb.append("null");
                    } else {
                        sb.append(stripTrailingZero(Double.toString(d)));
                    }
                } else if (v instanceof Float) {
                    float f = (Float) v;
                    if (Float.isNaN(f) || Float.isInfinite(f)) {
                        sb.append("null");
                    } else {
                        sb.append(stripTrailingZero(Float.toString(f)));
                    }
                } else {
                    sb.append(v.toString());
                }
            } else if (v instanceof Boolean) {
                sb.append(((Boolean) v) ? "true" : "false");
            } else if (v instanceof Map) {
                @SuppressWarnings("unchecked")
                Map<String, Object> map = (Map<String, Object>) v;
                sb.append("{");
                boolean first = true;
                for (Map.Entry<String, Object> e : map.entrySet()) {
                    if (!first) sb.append(",");
                    first = false;
                    writeString(sb, e.getKey());
                    sb.append(":");
                    writeJson(sb, e.getValue());
                }
                sb.append("}");
            } else if (v instanceof List) {
                @SuppressWarnings("unchecked")
                List<Object> list = (List<Object>) v;
                sb.append("[");
                boolean first = true;
                for (Object o : list) {
                    if (!first) sb.append(",");
                    first = false;
                    writeJson(sb, o);
                }
                sb.append("]");
            } else {
                // Fallback to string
                writeString(sb, v.toString());
            }
        }

        private static void writeString(StringBuilder sb, String s) {
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
                            sb.append(String.format("\\u%04x", (int) c));
                        } else {
                            sb.append(c);
                        }
                }
            }
            sb.append('"');
        }

        private static String stripTrailingZero(String s) {
            if (s.indexOf('E') >= 0 || s.indexOf('e') >= 0) return s;
            if (s.indexOf('.') >= 0) {
                // Remove trailing zeros and optional dot
                int end = s.length();
                while (end > 0 && s.charAt(end - 1) == '0') end--;
                if (end > 0 && s.charAt(end - 1) == '.') end--;
                return s.substring(0, end);
            }
            return s;
        }

        static final class Parser {
            final String s;
            final int n;
            int pos;

            Parser(String s) {
                this.s = s;
                this.n = s.length();
                this.pos = 0;
            }

            boolean eof() { return pos >= n; }

            void skipWs() {
                while (!eof()) {
                    char c = s.charAt(pos);
                    if (c == ' ' || c == '\n' || c == '\r' || c == '\t') pos++;
                    else break;
                }
            }

            Object parseValue() {
                skipWs();
                if (eof()) throw new IllegalArgumentException("Unexpected end of input");
                char c = s.charAt(pos);
                if (c == '{') return parseObject();
                if (c == '[') return parseArray();
                if (c == '"') return parseString();
                if (c == 't' || c == 'f') return parseBoolean();
                if (c == 'n') return parseNull();
                return parseNumber();
            }

            Map<String, Object> parseObject() {
                expect('{');
                skipWs();
                Map<String, Object> obj = new LinkedHashMap<>();
                if (peek('}')) { pos++; return obj; }
                while (true) {
                    skipWs();
                    if (!peek('"')) throw new IllegalArgumentException("Expected string key at position " + pos);
                    String key = parseString();
                    skipWs();
                    expect(':');
                    Object val = parseValue();
                    obj.put(key, val);
                    skipWs();
                    if (peek('}')) { pos++; break; }
                    expect(',');
                }
                return obj;
            }

            List<Object> parseArray() {
                expect('[');
                skipWs();
                List<Object> arr = new ArrayList<>();
                if (peek(']')) { pos++; return arr; }
                while (true) {
                    Object v = parseValue();
                    arr.add(v);
                    skipWs();
                    if (peek(']')) { pos++; break; }
                    expect(',');
                }
                return arr;
            }

            String parseString() {
                expect('"');
                StringBuilder sb = new StringBuilder();
                while (!eof()) {
                    char c = s.charAt(pos++);
                    if (c == '"') {
                        return sb.toString();
                    } else if (c == '\\') {
                        if (eof()) throw new IllegalArgumentException("Invalid escape at end of string");
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
                                if (pos + 4 > n) throw new IllegalArgumentException("Invalid unicode escape");
                                int code = 0;
                                for (int i = 0; i < 4; i++) {
                                    char h = s.charAt(pos++);
                                    int v;
                                    if (h >= '0' && h <= '9') v = h - '0';
                                    else if (h >= 'a' && h <= 'f') v = h - 'a' + 10;
                                    else if (h >= 'A' && h <= 'F') v = h - 'A' + 10;
                                    else throw new IllegalArgumentException("Invalid hex in unicode escape");
                                    code = (code << 4) | v;
                                }
                                sb.append((char) code);
                                break;
                            default:
                                throw new IllegalArgumentException("Invalid escape sequence: \\" + e);
                        }
                    } else {
                        if (c < 0x20) throw new IllegalArgumentException("Control char in string");
                        sb.append(c);
                    }
                }
                throw new IllegalArgumentException("Unterminated string");
            }

            Object parseBoolean() {
                if (match("true")) return Boolean.TRUE;
                if (match("false")) return Boolean.FALSE;
                throw new IllegalArgumentException("Invalid boolean at position " + pos);
            }

            Object parseNull() {
                if (match("null")) return null;
                throw new IllegalArgumentException("Invalid null at position " + pos);
            }

            Object parseNumber() {
                int start = pos;
                if (peek('-')) pos++;
                if (peek('0')) {
                    pos++;
                } else if (isDigit(peekChar())) {
                    while (isDigit(peekChar())) pos++;
                } else {
                    throw new IllegalArgumentException("Invalid number at position " + pos);
                }
                boolean isFloat = false;
                if (peek('.')) {
                    isFloat = true;
                    pos++;
                    if (!isDigit(peekChar())) throw new IllegalArgumentException("Invalid fraction");
                    while (isDigit(peekChar())) pos++;
                }
                if (peek('e') || peek('E')) {
                    isFloat = true;
                    pos++;
                    if (peek('+') || peek('-')) pos++;
                    if (!isDigit(peekChar())) throw new IllegalArgumentException("Invalid exponent");
                    while (isDigit(peekChar())) pos++;
                }
                String num = s.substring(start, pos);
                try {
                    if (isFloat) {
                        return Double.parseDouble(num);
                    } else {
                        // Fit into Long, otherwise use Double
                        long v = Long.parseLong(num);
                        return v;
                    }
                } catch (NumberFormatException ex) {
                    // Fallback to Double
                    try {
                        return Double.parseDouble(num);
                    } catch (NumberFormatException ex2) {
                        throw new IllegalArgumentException("Invalid number: " + num);
                    }
                }
            }

            boolean peek(char c) {
                return !eof() && s.charAt(pos) == c;
            }

            char peekChar() {
                return eof() ? '\0' : s.charAt(pos);
            }

            void expect(char c) {
                if (eof() || s.charAt(pos) != c) {
                    throw new IllegalArgumentException("Expected '" + c + "' at position " + pos);
                }
                pos++;
            }

            boolean isDigit(char c) {
                return c >= '0' && c <= '9';
            }

            boolean match(String lit) {
                int end = pos + lit.length();
                if (end > n) return false;
                if (s.regionMatches(pos, lit, 0, lit.length())) {
                    pos = end;
                    return true;
                }
                return false;
            }
        }
    }

    // Helper to print parsed object as JSON
    private static String toJsonString(Object obj) {
        return Json.stringify(obj);
    }

    public static void main(String[] args) {
        String[] tests;
        if (args != null && args.length > 0) {
            tests = args;
        } else {
            tests = new String[] {
                    "https://jsonplaceholder.typicode.com/todos/1",
                    "https://jsonplaceholder.typicode.com/users/1",
                    "https://jsonplaceholder.typicode.com/todos/2",
                    "https://api.github.com/",
                    "https://httpbin.org/json"
            };
        }
        for (int i = 0; i < tests.length && i < 5; i++) {
            String url = tests[i];
            System.out.println("Fetching: " + url);
            try {
                Object obj = fetchJsonFromUrl(url);
                System.out.println(toJsonString(obj));
            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
            }
            System.out.println("----");
        }
    }
}