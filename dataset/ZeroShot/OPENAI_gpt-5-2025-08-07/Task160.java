import java.io.IOException;
import java.net.*;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.time.Duration;

public class Task160 {

    private static final int MAX_BYTES = 1_048_576; // 1 MiB limit
    private static final Duration CONNECT_TIMEOUT = Duration.ofSeconds(10);
    private static final Duration REQUEST_TIMEOUT = Duration.ofSeconds(10);

    public static String safeHttpFetch(String url) {
        try {
            URI uri = new URI(url);
            String scheme = uri.getScheme();
            if (scheme == null || !(scheme.equalsIgnoreCase("http") || scheme.equalsIgnoreCase("https"))) {
                return "ERROR: Only http and https schemes are allowed.";
            }
            if (uri.getUserInfo() != null) {
                return "ERROR: User info in URL is not allowed.";
            }
            String host = uri.getHost();
            if (host == null || host.isBlank()) {
                return "ERROR: URL must include a valid host.";
            }
            int port = uri.getPort();
            if (!isAllowedPort(scheme, port)) {
                return "ERROR: Port not allowed. Only default ports 80 and 443 are permitted.";
            }
            // Validate DNS resolution to public IPs only
            validateHostPublic(host);

            HttpClient client = HttpClient.newBuilder()
                    .connectTimeout(CONNECT_TIMEOUT)
                    .followRedirects(HttpClient.Redirect.NEVER)
                    .build();

            HttpRequest request = HttpRequest.newBuilder(uri)
                    .timeout(REQUEST_TIMEOUT)
                    .header("User-Agent", "Task160/1.0 (+https://example.invalid)")
                    .header("Accept-Encoding", "identity")
                    .header("Connection", "close")
                    .header("Range", "bytes=0-" + (MAX_BYTES - 1))
                    .GET()
                    .build();

            HttpResponse<byte[]> response = client.send(request, HttpResponse.BodyHandlers.ofByteArray());
            int status = response.statusCode();
            if (status >= 300 && status < 400) {
                return "ERROR: HTTP " + status + " (redirects are disallowed)";
            }
            if (status >= 400) {
                return "ERROR: HTTP " + status;
            }
            byte[] body = response.body();
            if (body == null) {
                return "";
            }
            if (body.length > MAX_BYTES) {
                // Shouldn't happen due to Range, but enforce anyway
                byte[] truncated = new byte[MAX_BYTES];
                System.arraycopy(body, 0, truncated, 0, MAX_BYTES);
                return new String(truncated, StandardCharsets.UTF_8);
            }
            return new String(body, StandardCharsets.UTF_8);
        } catch (URISyntaxException e) {
            return "ERROR: Invalid URL format.";
        } catch (SecurityException se) {
            return "ERROR: " + se.getMessage();
        } catch (IOException | InterruptedException e) {
            return "ERROR: " + e.getMessage();
        }
    }

    private static boolean isAllowedPort(String scheme, int port) {
        if (port == -1) return true; // default
        if (scheme.equalsIgnoreCase("http")) return port == 80;
        if (scheme.equalsIgnoreCase("https")) return port == 443;
        return false;
    }

    private static void validateHostPublic(String host) {
        try {
            InetAddress[] addrs = InetAddress.getAllByName(host);
            for (InetAddress addr : addrs) {
                if (!isPublicAddress(addr)) {
                    throw new SecurityException("Resolved to a private or disallowed IP address: " + addr.getHostAddress());
                }
            }
        } catch (UnknownHostException e) {
            throw new SecurityException("Unable to resolve host.");
        }
    }

    private static boolean isPublicAddress(InetAddress addr) {
        if (addr.isAnyLocalAddress()
                || addr.isLoopbackAddress()
                || addr.isLinkLocalAddress()
                || addr.isMulticastAddress()) {
            return false;
        }
        byte[] b = addr.getAddress();
        if (addr instanceof java.net.Inet4Address) {
            int b0 = b[0] & 0xFF;
            int b1 = b[1] & 0xFF;
            int b2 = b[2] & 0xFF;
            // 10.0.0.0/8
            if (b0 == 10) return false;
            // 172.16.0.0/12
            if (b0 == 172 && (b1 >= 16 && b1 <= 31)) return false;
            // 192.168.0.0/16
            if (b0 == 192 && b1 == 168) return false;
            // 127.0.0.0/8
            if (b0 == 127) return false;
            // 169.254.0.0/16
            if (b0 == 169 && b1 == 254) return false;
            // 100.64.0.0/10
            if (b0 == 100 && (b1 >= 64 && b1 <= 127)) return false;
            // 192.0.0.0/24
            if (b0 == 192 && b1 == 0 && b2 == 0) return false;
            // 192.0.2.0/24, 198.51.100.0/24, 203.0.113.0/24 (TEST-NET)
            if ((b0 == 192 && b1 == 0 && b2 == 2)
                    || (b0 == 198 && b1 == 51 && b2 == 100)
                    || (b0 == 203 && b1 == 0 && b2 == 113)) return false;
            // 198.18.0.0/15
            if (b0 == 198 && (b1 == 18 || b1 == 19)) return false;
            // 0.0.0.0/8
            if (b0 == 0) return false;
            // 224.0.0.0/4 multicast and above
            if (b0 >= 224) return false;
            return true;
        } else { // IPv6
            int first = b[0] & 0xFF;
            int second = b[1] & 0xFF;
            // fc00::/7 Unique local addresses
            if ((first & 0xFE) == 0xFC) return false;
            // fe80::/10 link-local (already caught by isLinkLocalAddress)
            // ::/128 unspecified
            boolean allZero = true;
            for (byte value : b) {
                if (value != 0) {
                    allZero = false;
                    break;
                }
            }
            if (allZero) return false;
            // 2001:db8::/32 documentation
            if (first == 0x20 && second == 0x01 && (b[2] & 0xFF) == 0x0D && (b[3] & 0xFF) == 0xB8) return false;
            return true;
        }
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
                "https://example.com",
                "http://example.com",
                "http://127.0.0.1",
                "http://169.254.169.254/latest/meta-data/",
                "https://example.com:443"
        };
        for (String t : tests) {
            System.out.println("URL: " + t);
            String res = safeHttpFetch(t);
            System.out.println("Result (first 200 chars):");
            if (res == null) res = "";
            System.out.println(res.substring(0, Math.min(200, res.length())));
            System.out.println("----");
        }
    }
}