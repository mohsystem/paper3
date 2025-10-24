// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement a function that takes a URL string, securely fetches JSON content from the URL, parses it, and returns the resulting JSON object.
// 2) Security requirements: Validate URL scheme (http/https), set timeouts, handle redirects safely with a limit, enforce a maximum download size, use proper character decoding, and avoid code injection risks during parsing.
// 3) Secure coding generation: Implement safe network I/O with HttpURLConnection, enforce size/timeouts, validate redirects, and parse JSON using Nashorn JavaScript engine in a safe manner by passing the JSON as a variable.
// 4) Code review: Ensure streams are closed, exceptions are handled, unsafe schemes rejected, redirect loops prevented, and parsing done safely.
// 5) Secure code output: Final code mitigates identified issues and includes a main method with 5 test cases; the function accepts input as a parameter and returns the parsed object.

import javax.net.ssl.HttpsURLConnection;
import java.io.*;
import java.net.*;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.*;
import javax.script.*;

public class Task64 {

    // Fetch JSON from URL and parse into Java Maps/Lists/primitive types.
    // Returns: Object representing JSON (Map, List, String, Number, Boolean, or null).
    public static Object fetchAndParseJson(String urlStr) throws Exception {
        Objects.requireNonNull(urlStr, "URL must not be null");
        URL url = new URL(urlStr);
        String scheme = url.getProtocol();
        if (!"http".equalsIgnoreCase(scheme) && !"https".equalsIgnoreCase(scheme)) {
            throw new MalformedURLException("Only http and https schemes are allowed");
        }

        String jsonText = fetchTextFromUrlWithRedirects(url, 5, 10_000, 10_000, 5_000_000);

        // Parse JSON securely using Nashorn to Java-compatible structures.
        // This avoids third-party dependencies. If Nashorn is unavailable, throw an exception.
        ScriptEngine engine = new ScriptEngineManager().getEngineByName("nashorn");
        if (engine == null) {
            // Try generic "JavaScript" engine name as a fallback
            engine = new ScriptEngineManager().getEngineByName("JavaScript");
        }
        if (engine == null) {
            throw new UnsupportedOperationException("No JavaScript engine available to parse JSON");
        }

        // Safe evaluation: pass the JSON as a bound variable; do not concatenate into code.
        engine.put("json", jsonText);
        // Java.asJSONCompatible converts JS objects/arrays into Java Map/List/primitive
        Object result = engine.eval("Java.asJSONCompatible(JSON.parse(json))");
        return result;
    }

    private static String fetchTextFromUrlWithRedirects(URL url,
                                                        int maxRedirects,
                                                        int connectTimeoutMs,
                                                        int readTimeoutMs,
                                                        int maxBytes) throws IOException {
        int redirects = 0;
        URL current = url;
        while (true) {
            HttpURLConnection conn = (HttpURLConnection) current.openConnection();
            conn.setInstanceFollowRedirects(false);
            conn.setConnectTimeout(connectTimeoutMs);
            conn.setReadTimeout(readTimeoutMs);
            conn.setRequestMethod("GET");
            conn.setRequestProperty("User-Agent", "Task64/1.0 (+https://example.com)");
            conn.setRequestProperty("Accept", "application/json, text/json, */*;q=0.8");

            int code;
            try {
                code = conn.getResponseCode();
            } catch (IOException e) {
                // Attempt to read error stream for more context then rethrow
                InputStream es = conn.getErrorStream();
                if (es != null) {
                    try { es.close(); } catch (IOException ignore) {}
                }
                conn.disconnect();
                throw e;
            }

            if (code >= 300 && code < 400) {
                String location = conn.getHeaderField("Location");
                conn.disconnect();
                if (location == null) {
                    throw new IOException("Redirect without Location header");
                }
                URL next = resolveRedirect(current, location);
                String scheme = next.getProtocol();
                if (!"http".equalsIgnoreCase(scheme) && !"https".equalsIgnoreCase(scheme)) {
                    throw new IOException("Redirected to disallowed scheme: " + scheme);
                }
                if (++redirects > maxRedirects) {
                    throw new IOException("Too many redirects");
                }
                current = next;
                continue;
            }

            InputStream is = null;
            try {
                if (code >= 200 && code < 300) {
                    is = conn.getInputStream();
                } else {
                    is = conn.getErrorStream();
                    if (is == null) {
                        throw new IOException("HTTP error: " + code);
                    }
                }

                String contentType = conn.getContentType();
                Charset charset = extractCharset(contentType);
                byte[] data = readAllWithLimit(is, maxBytes);
                if (!(code >= 200 && code < 300)) {
                    throw new IOException("HTTP error: " + code + " Body: " +
                            previewForError(data, charset));
                }
                return new String(data, charset);
            } finally {
                if (is != null) {
                    try { is.close(); } catch (IOException ignore) {}
                }
                conn.disconnect();
            }
        }
    }

    private static URL resolveRedirect(URL base, String location) throws MalformedURLException {
        try {
            return new URL(base, location);
        } catch (MalformedURLException e) {
            throw e;
        }
    }

    private static Charset extractCharset(String contentType) {
        // Default to UTF-8 for JSON
        if (contentType != null) {
            // Example: application/json; charset=utf-8
            String[] parts = contentType.split(";");
            for (int i = 1; i < parts.length; i++) {
                String p = parts[i].trim();
                int eq = p.indexOf('=');
                if (eq > 0) {
                    String name = p.substring(0, eq).trim();
                    String value = p.substring(eq + 1).trim().replace("\"", "");
                    if ("charset".equalsIgnoreCase(name)) {
                        try {
                            return Charset.forName(value);
                        } catch (Exception ignore) {
                            // Fallback to UTF-8
                        }
                    }
                }
            }
        }
        return StandardCharsets.UTF_8;
    }

    private static byte[] readAllWithLimit(InputStream is, int maxBytes) throws IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream(Math.min(maxBytes, 8192));
        byte[] buf = new byte[8192];
        int n;
        int total = 0;
        while ((n = is.read(buf)) != -1) {
            total += n;
            if (total > maxBytes) {
                throw new IOException("Response too large (exceeds " + maxBytes + " bytes)");
            }
            out.write(buf, 0, n);
        }
        return out.toByteArray();
    }

    private static String previewForError(byte[] data, Charset cs) {
        String s = new String(data, cs);
        if (s.length() > 200) {
            return s.substring(0, 200) + "...";
        }
        return s;
    }

    // Simple pretty printer for Map/List/primitive to String for demonstration
    private static String toPrettyString(Object obj) {
        if (obj == null) return "null";
        if (obj instanceof Map) {
            StringBuilder sb = new StringBuilder();
            sb.append("{");
            boolean first = true;
            for (Map.Entry<?, ?> e : ((Map<?, ?>) obj).entrySet()) {
                if (!first) sb.append(", ");
                first = false;
                sb.append(String.valueOf(e.getKey())).append(": ").append(toPrettyString(e.getValue()));
            }
            sb.append("}");
            return sb.toString();
        } else if (obj instanceof List) {
            StringBuilder sb = new StringBuilder();
            sb.append("[");
            boolean first = true;
            for (Object v : (List<?>) obj) {
                if (!first) sb.append(", ");
                first = false;
                sb.append(toPrettyString(v));
            }
            sb.append("]");
            return sb.toString();
        } else if (obj instanceof String) {
            return "\"" + obj + "\"";
        } else {
            return String.valueOf(obj);
        }
    }

    // Main with 5 test cases. If an argument is given, it will be included as the first test.
    public static void main(String[] args) {
        List<String> tests = new ArrayList<>();
        if (args != null && args.length > 0) {
            tests.add(args[0]);
        }
        // Add default test URLs (public JSON endpoints)
        tests.add("https://jsonplaceholder.typicode.com/todos/1");
        tests.add("https://jsonplaceholder.typicode.com/users/1");
        tests.add("https://api.github.com/");
        tests.add("https://httpbin.org/json");
        tests.add("https://pokeapi.co/api/v2/pokemon/ditto");

        // Ensure exactly five tests (trim if more)
        if (tests.size() > 5) {
            tests = tests.subList(0, 5);
        }

        for (int i = 0; i < tests.size(); i++) {
            String u = tests.get(i);
            System.out.println("Test " + (i + 1) + ": " + u);
            try {
                Object result = fetchAndParseJson(u);
                System.out.println("Parsed type: " + (result == null ? "null" : result.getClass().getSimpleName()));
                System.out.println("Content preview: " + toPrettyString(result));
            } catch (Exception e) {
                System.out.println("Error: " + e.getMessage());
            }
            System.out.println("----");
        }
    }
}