import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.InetAddress;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpHeaders;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.Optional;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Task76 {

    private static final int MAX_BYTES = 1_048_576; // 1 MiB
    private static final Duration TIMEOUT = Duration.ofSeconds(10);

    public static String fetchTitle(String urlStr) {
        try {
            if (urlStr == null) return "";
            URI uri = new URI(urlStr.trim());

            // Enforce HTTPS
            if (!"https".equalsIgnoreCase(uri.getScheme())) {
                return "";
            }

            String host = uri.getHost();
            if (host == null) {
                return "";
            }
            String hostLower = host.toLowerCase();
            if ("localhost".equals(hostLower)) {
                return "";
            }

            // Resolve and block local/private addresses
            InetAddress[] addrs = InetAddress.getAllByName(host);
            for (InetAddress addr : addrs) {
                if (addr.isAnyLocalAddress()
                        || addr.isLoopbackAddress()
                        || addr.isSiteLocalAddress()
                        || addr.isLinkLocalAddress()
                        || addr.isMulticastAddress()) {
                    return "";
                }
            }

            HttpClient client = HttpClient.newBuilder()
                    .followRedirects(HttpClient.Redirect.NORMAL)
                    .connectTimeout(TIMEOUT)
                    .build();

            HttpRequest request = HttpRequest.newBuilder(uri)
                    .GET()
                    .timeout(TIMEOUT)
                    .header("User-Agent", "Task76/1.0 (+https://example.invalid)")
                    .header("Accept-Encoding", "identity")
                    .build();

            HttpResponse<InputStream> resp = client.send(request, HttpResponse.BodyHandlers.ofInputStream());
            int status = resp.statusCode();
            if (status != 200) {
                return "";
            }

            String charsetName = getCharsetFromContentType(resp.headers()).orElse("UTF-8");
            Charset cs;
            try {
                cs = Charset.forName(charsetName);
            } catch (Exception e) {
                cs = StandardCharsets.UTF_8;
            }

            byte[] bodyBytes = readLimited(resp.body(), MAX_BYTES);
            String html = new String(bodyBytes, cs);

            return extractTitle(html);
        } catch (Exception e) {
            return "";
        }
    }

    private static Optional<String> getCharsetFromContentType(HttpHeaders headers) {
        Optional<String> ct = headers.firstValue("content-type");
        if (ct.isEmpty()) return Optional.empty();
        String v = ct.get();
        int i = v.toLowerCase().indexOf("charset=");
        if (i >= 0) {
            String ch = v.substring(i + 8).trim();
            int sc = ch.indexOf(';');
            if (sc >= 0) ch = ch.substring(0, sc);
            ch = ch.replace("\"", "").replace("'", "").trim();
            if (!ch.isEmpty()) return Optional.of(ch);
        }
        return Optional.empty();
    }

    private static byte[] readLimited(InputStream is, int limit) throws Exception {
        try (InputStream in = is; ByteArrayOutputStream out = new ByteArrayOutputStream()) {
            byte[] buf = new byte[8192];
            int total = 0;
            int r;
            while ((r = in.read(buf, 0, Math.min(buf.length, limit - total))) > 0) {
                out.write(buf, 0, r);
                total += r;
                if (total >= limit) break;
            }
            return out.toByteArray();
        }
    }

    private static String extractTitle(String html) {
        if (html == null) return "";
        Pattern p = Pattern.compile("<title\\s*[^>]*>(.*?)</title>", Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
        Matcher m = p.matcher(html);
        if (m.find()) {
            String t = m.group(1);
            t = t.replaceAll("\\s+", " ").trim();
            return t;
        }
        return "";
    }

    public static void main(String[] args) {
        String[] tests;
        if (args != null && args.length > 0) {
            tests = args;
        } else {
            tests = new String[] {
                "https://example.com",
                "https://httpbin.org/html",
                "https://www.iana.org/domains/reserved",
                "https://www.wikipedia.org",
                "https://www.gnu.org"
            };
        }
        for (String u : tests) {
            String title = fetchTitle(u);
            System.out.println(u + " => " + title);
        }
    }
}