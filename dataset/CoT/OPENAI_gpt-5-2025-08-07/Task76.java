import javax.net.ssl.HttpsURLConnection;
import java.io.*;
import java.net.*;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.GZIPInputStream;

/*
Chain-of-Through process:
1) Problem understanding:
   - Fetch HTML from a provided HTTPS URL, extract and return the <title> text.
2) Security requirements:
   - Enforce HTTPS only.
   - Limit redirects and disallow downgrade to HTTP.
   - Set reasonable timeouts and limit read size to prevent resource exhaustion.
   - Validate URL format.
3) Secure coding generation:
   - Implement manual redirect handling with checks.
   - Limit download size to MAX_BYTES and set timeouts.
   - Use safe charset detection and robust parsing.
4) Code review:
   - Validate error handling, resource cleanup (try-with-resources), and input validation.
5) Secure code output:
   - Finalized with mitigations for the identified concerns.
*/

public class Task76 {

    private static final int CONNECT_TIMEOUT_MS = 10000;
    private static final int READ_TIMEOUT_MS = 15000;
    private static final int MAX_REDIRECTS = 5;
    private static final int MAX_BYTES = 1_048_576; // 1 MB
    private static final Pattern TITLE_PATTERN =
            Pattern.compile("<title[^>]*>(.*?)</title>", Pattern.CASE_INSENSITIVE | Pattern.DOTALL);

    public static String getPageTitle(String urlStr) {
        if (urlStr == null) return "";
        urlStr = urlStr.trim();
        URL currentUrl;
        try {
            URI uri = new URI(urlStr);
            if (!"https".equalsIgnoreCase(uri.getScheme())) {
                return "";
            }
            currentUrl = uri.toURL();
        } catch (Exception e) {
            return "";
        }

        int redirects = 0;
        while (redirects <= MAX_REDIRECTS) {
            HttpsURLConnection conn = null;
            try {
                conn = (HttpsURLConnection) currentUrl.openConnection();
                conn.setInstanceFollowRedirects(false);
                conn.setRequestMethod("GET");
                conn.setConnectTimeout(CONNECT_TIMEOUT_MS);
                conn.setReadTimeout(READ_TIMEOUT_MS);
                conn.setRequestProperty("User-Agent", "Task76-Client/1.0 (+https)");
                conn.setRequestProperty("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
                conn.setRequestProperty("Accept-Language", "en-US,en;q=0.5");
                conn.setRequestProperty("Accept-Encoding", "gzip");
                conn.connect();

                int code = conn.getResponseCode();
                if (code >= 300 && code <= 399) {
                    String location = conn.getHeaderField("Location");
                    if (location == null || location.isEmpty()) {
                        return "";
                    }
                    URL next = new URL(currentUrl, location);
                    if (!"https".equalsIgnoreCase(next.getProtocol())) {
                        return "";
                    }
                    currentUrl = next;
                    redirects++;
                    continue;
                }

                if (code != HttpURLConnection.HTTP_OK) {
                    return "";
                }

                String contentEncoding = conn.getHeaderField("Content-Encoding");
                InputStream in = safeInputStream(conn);
                if (in == null) return "";

                if (contentEncoding != null && contentEncoding.toLowerCase().contains("gzip")) {
                    in = new GZIPInputStream(in);
                }

                byte[] data = readUpTo(in, MAX_BYTES);
                if (data.length == 0) return "";
                String contentType = conn.getContentType();
                Charset charset = extractCharset(contentType);
                String html = new String(data, charset);

                Matcher m = TITLE_PATTERN.matcher(html);
                if (m.find()) {
                    String title = m.group(1);
                    return title == null ? "" : title.replaceAll("\\s+", " ").trim();
                }
                return "";
            } catch (IOException e) {
                return "";
            } finally {
                if (conn != null) {
                    conn.disconnect();
                }
            }
        }
        return "";
    }

    private static InputStream safeInputStream(URLConnection conn) {
        try {
            return conn.getInputStream();
        } catch (IOException e) {
            try {
                InputStream err = ((HttpURLConnection) conn).getErrorStream();
                if (err != null) err.close();
            } catch (Exception ignore) {}
            return null;
        }
    }

    private static Charset extractCharset(String contentType) {
        if (contentType != null) {
            String[] parts = contentType.split(";");
            for (String p : parts) {
                String s = p.trim().toLowerCase();
                if (s.startsWith("charset=")) {
                    String cs = s.substring(8).trim();
                    try {
                        return Charset.forName(cs);
                    } catch (Exception ignore) {
                    }
                }
            }
        }
        return StandardCharsets.UTF_8;
    }

    private static byte[] readUpTo(InputStream in, int limit) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream(Math.min(limit, 8192));
        byte[] buf = new byte[8192];
        int total = 0;
        int r;
        while ((r = in.read(buf)) != -1) {
            if (total + r > limit) {
                baos.write(buf, 0, limit - total);
                break;
            } else {
                baos.write(buf, 0, r);
                total += r;
            }
        }
        return baos.toByteArray();
    }

    public static void main(String[] args) {
        String[] urls;
        if (args != null && args.length > 0) {
            urls = args;
        } else {
            urls = new String[]{
                    "https://example.com",
                    "https://www.wikipedia.org/",
                    "https://www.mozilla.org/en-US/",
                    "https://www.bbc.com/",
                    "https://www.oracle.com/"
            };
        }
        for (String u : urls) {
            String title = getPageTitle(u);
            System.out.println(u + " -> " + title);
        }
    }
}