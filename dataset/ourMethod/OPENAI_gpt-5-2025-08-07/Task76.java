import javax.net.ssl.HttpsURLConnection;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URI;
import java.net.URL;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task76 {

    private static final int TIMEOUT_MS = 10000;
    private static final int MAX_REDIRECTS = 5;
    private static final int MAX_READ_CHARS = 1_048_576; // 1 MB cap
    private static final String USER_AGENT = "SecureTitleFetcher/1.0 (+https://example.com)";

    public static void main(String[] args) {
        String[] tests;
        if (args != null && args.length > 0) {
            tests = args;
        } else {
            tests = new String[]{
                "https://example.com",
                "https://www.wikipedia.org",
                "https://www.iana.org/domains/reserved",
                "https://www.gnu.org",
                "https://httpbin.org/html"
            };
        }
        for (String url : tests) {
            String title = fetchTitle(url);
            System.out.println(url + " => " + title);
        }
    }

    public static String fetchTitle(String urlStr) {
        try {
            if (urlStr == null) return "ERROR: URL is null";
            urlStr = urlStr.trim();
            if (urlStr.isEmpty()) return "ERROR: URL is empty";
            if (urlStr.length() > 2048) return "ERROR: URL too long";
            URI initialUri = new URI(urlStr);
            if (!"https".equalsIgnoreCase(initialUri.getScheme())) {
                return "ERROR: Only HTTPS URLs are allowed";
            }

            URL currentUrl = initialUri.toURL();
            int redirects = 0;
            while (redirects <= MAX_REDIRECTS) {
                HttpURLConnection conn = (HttpURLConnection) currentUrl.openConnection();
                if (!(conn instanceof HttpsURLConnection)) {
                    // Enforce HTTPS on every hop
                    conn.disconnect();
                    return "ERROR: Redirected to non-HTTPS URL";
                }
                HttpsURLConnection https = (HttpsURLConnection) conn;
                https.setInstanceFollowRedirects(false); // manual redirect handling
                https.setRequestMethod("GET");
                https.setConnectTimeout(TIMEOUT_MS);
                https.setReadTimeout(TIMEOUT_MS);
                https.setUseCaches(false);
                https.setRequestProperty("User-Agent", USER_AGENT);
                https.setRequestProperty("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
                https.setRequestProperty("Accept-Language", "en-US,en;q=0.7");
                int code;
                try {
                    code = https.getResponseCode();
                } catch (Exception e) {
                    https.disconnect();
                    return "ERROR: " + sanitizeMsg(e.getMessage());
                }

                if (code >= 300 && code < 400) {
                    String loc = https.getHeaderField("Location");
                    https.disconnect();
                    if (loc == null || loc.isEmpty()) {
                        return "ERROR: Redirect without Location header";
                    }
                    URL next = new URL(currentUrl, loc);
                    if (!"https".equalsIgnoreCase(next.getProtocol())) {
                        return "ERROR: Redirected to non-HTTPS URL";
                    }
                    currentUrl = next;
                    redirects++;
                    continue;
                }

                if (code != HttpURLConnection.HTTP_OK) {
                    String msg = https.getResponseMessage();
                    https.disconnect();
                    return "ERROR: HTTP " + code + (msg != null ? (" " + msg) : "");
                }

                String contentType = https.getHeaderField("Content-Type");
                Charset cs = extractCharset(contentType);
                String title = readAndExtractTitle(https, cs);
                https.disconnect();
                return title != null && !title.isEmpty() ? title : "ERROR: Title not found";
            }
            return "ERROR: Too many redirects";
        } catch (Exception ex) {
            return "ERROR: " + sanitizeMsg(ex.getMessage());
        }
    }

    private static Charset extractCharset(String contentType) {
        if (contentType != null) {
            // Example: text/html; charset=UTF-8
            Pattern p = Pattern.compile("charset=\\s*([^;]+)", Pattern.CASE_INSENSITIVE);
            Matcher m = p.matcher(contentType);
            if (m.find()) {
                String cs = m.group(1).trim().replace("\"", "");
                try {
                    return Charset.forName(cs);
                } catch (Exception ignored) {
                    // fallback to UTF-8
                }
            }
        }
        return StandardCharsets.UTF_8;
    }

    private static String readAndExtractTitle(HttpsURLConnection https, Charset charset) {
        StringBuilder sb = new StringBuilder(4096);
        char[] buf = new char[4096];
        try (InputStream in = https.getInputStream();
             InputStreamReader isr = new InputStreamReader(in, charset);
             BufferedReader br = new BufferedReader(isr)) {
            int total = 0;
            int r;
            while ((r = br.read(buf)) != -1) {
                if (r > 0) {
                    int toAppend = Math.min(r, Math.max(0, MAX_READ_CHARS - total));
                    if (toAppend > 0) {
                        sb.append(buf, 0, toAppend);
                        total += toAppend;
                    }
                    // Stop if we found closing title tag or reached limit
                    String lower = sb.toString().toLowerCase(Locale.ROOT);
                    if (lower.contains("</title>") || total >= MAX_READ_CHARS) {
                        break;
                    }
                }
            }
        } catch (Exception e) {
            return "ERROR: " + sanitizeMsg(e.getMessage());
        }
        return extractTitleFromHtml(sb.toString());
    }

    private static String extractTitleFromHtml(String html) {
        if (html == null || html.isEmpty()) return "";
        String lower = html.toLowerCase(Locale.ROOT);
        int startIdx = lower.indexOf("<title");
        if (startIdx < 0) return "";
        int gt = lower.indexOf('>', startIdx);
        if (gt < 0) return "";
        int endIdx = lower.indexOf("</title>", gt + 1);
        if (endIdx < 0) return "";
        String raw = html.substring(gt + 1, endIdx);
        // Remove any inner tags
        String noTags = raw.replaceAll("(?is)<[^>]*>", "");
        // Collapse whitespace
        String title = noTags.replace('\r', ' ').replace('\n', ' ').replace('\t', ' ').trim();
        while (title.contains("  ")) title = title.replace("  ", " ");
        return title;
    }

    private static String sanitizeMsg(String msg) {
        if (msg == null) return "Unexpected error";
        // Remove new lines and excessive length to avoid log injection or noisy output
        msg = msg.replaceAll("[\\r\\n]+", " ").trim();
        if (msg.length() > 200) msg = msg.substring(0, 200) + "...";
        return msg;
    }
}