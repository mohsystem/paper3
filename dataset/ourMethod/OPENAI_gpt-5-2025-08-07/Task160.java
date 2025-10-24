import java.io.IOException;
import java.net.*;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.time.Duration;

public class Task160 {

    private static final int MAX_URL_LENGTH = 2048;
    private static final int CONNECT_TIMEOUT_SECONDS = 10;
    private static final int MAX_BODY_BYTES = 1024 * 1024; // 1 MiB

    public static void main(String[] args) {
        String[] tests = new String[] {
                "https://example.com",
                "http://example.com",
                "ftp://example.com",
                "http://127.0.0.1",
                "http://user:pass@example.com"
        };
        for (String t : tests) {
            System.out.println("URL: " + t);
            System.out.println(fetchUrlSafely(t));
            System.out.println("----");
        }
    }

    public static String fetchUrlSafely(String rawUrl) {
        try {
            URI uri = validateAndNormalizeUrl(rawUrl);
            if (!isHostPublicAndRoutable(uri.getHost())) {
                return "ERROR: Host resolves to a private, loopback, link-local, multicast, or unspecified address";
            }

            HttpClient client = HttpClient.newBuilder()
                    .followRedirects(HttpClient.Redirect.NEVER)
                    .connectTimeout(Duration.ofSeconds(CONNECT_TIMEOUT_SECONDS))
                    .build();

            HttpRequest req = HttpRequest.newBuilder()
                    .uri(uri)
                    .timeout(Duration.ofSeconds(CONNECT_TIMEOUT_SECONDS))
                    .GET()
                    .header("User-Agent", "Task160-Client/1.0")
                    .header("Accept-Encoding", "identity")
                    .build();

            HttpResponse<byte[]> resp = client.send(req, HttpResponse.BodyHandlers.ofByteArray());

            byte[] body = resp.body();
            int used = Math.min(body.length, MAX_BODY_BYTES);
            String bodyText = new String(body, 0, used, StandardCharsets.UTF_8);
            if (body.length > MAX_BODY_BYTES) {
                bodyText += "\n[Truncated to " + MAX_BODY_BYTES + " bytes]";
            }

            StringBuilder sb = new StringBuilder();
            sb.append("STATUS: ").append(resp.statusCode()).append("\n");
            sb.append("HEADERS:\n");
            resp.headers().map().forEach((k, v) -> sb.append("  ").append(k).append(": ").append(String.join(", ", v)).append("\n"));
            sb.append("BODY:\n").append(bodyText);
            return sb.toString();
        } catch (SecurityException se) {
            return "ERROR: " + se.getMessage();
        } catch (IOException | InterruptedException e) {
            return "ERROR: " + safeMessage(e);
        }
    }

    private static URI validateAndNormalizeUrl(String rawUrl) {
        if (rawUrl == null) throw new SecurityException("URL is null");
        rawUrl = rawUrl.trim();
        if (rawUrl.length() == 0 || rawUrl.length() > MAX_URL_LENGTH) {
            throw new SecurityException("URL length is invalid");
        }

        URI uri;
        try {
            uri = new URI(rawUrl);
        } catch (URISyntaxException e) {
            throw new SecurityException("Malformed URL");
        }

        String scheme = uri.getScheme();
        if (scheme == null) throw new SecurityException("Missing scheme");
        scheme = scheme.toLowerCase();
        if (!scheme.equals("http") && !scheme.equals("https")) {
            throw new SecurityException("Only http and https schemes are allowed");
        }

        String host = uri.getHost();
        if (host == null || host.length() == 0) {
            throw new SecurityException("Missing host");
        }

        // Disallow userinfo (credentials) for safety
        if (uri.getUserInfo() != null) {
            throw new SecurityException("Credentials in URL are not allowed");
        }

        int port = uri.getPort();
        if (port != -1 && (port < 1 || port > 65535)) {
            throw new SecurityException("Invalid port");
        }

        // Normalize and rebuild to ensure consistency
        try {
            uri = new URI(
                    scheme,
                    null, // no userinfo
                    host,
                    port,
                    uri.getPath(),
                    uri.getQuery(),
                    uri.getFragment()
            );
        } catch (URISyntaxException e) {
            throw new SecurityException("Failed to normalize URL");
        }

        return uri;
    }

    private static boolean isHostPublicAndRoutable(String host) {
        try {
            InetAddress[] addresses = InetAddress.getAllByName(host);
            if (addresses.length == 0) return false;
            for (InetAddress addr : addresses) {
                if (!isPublicAddress(addr)) {
                    return false;
                }
            }
            return true;
        } catch (UnknownHostException e) {
            return false;
        }
    }

    private static boolean isPublicAddress(InetAddress addr) {
        if (addr.isAnyLocalAddress()) return false;
        if (addr.isLoopbackAddress()) return false;
        if (addr.isLinkLocalAddress()) return false;
        if (addr.isSiteLocalAddress()) return false;
        if (addr.isMulticastAddress()) return false;

        // Additional check for IPv6 Unique Local Addresses (fc00::/7)
        byte[] bytes = addr.getAddress();
        if (bytes.length == 16) {
            int first = bytes[0] & 0xFF;
            if ((first & 0xFE) == 0xFC) { // 0xfc or 0xfd
                return false;
            }
        }
        return true;
    }

    private static String safeMessage(Exception e) {
        String m = e.getMessage();
        if (m == null) return e.getClass().getSimpleName();
        // Avoid leaking internal paths or sensitive info
        if (m.length() > 300) m = m.substring(0, 300);
        return m;
    }
}