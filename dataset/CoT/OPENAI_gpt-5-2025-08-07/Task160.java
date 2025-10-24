// Chain-of-Through Process Implementation in Java
// 1) Problem understanding: Provide a function that accepts a URL and makes an HTTP GET request from the server, returning a safe, limited response summary.
// 2) Security requirements: Only allow http/https, validate host resolves to public IPs (no private/loopback/link-local/multicast), enforce timeouts, limit response size, restrict redirects and revalidate each, set a safe User-Agent, avoid automatic redirects.
// 3) Secure coding generation: Implement validations, safe network operations, robust error handling, and size limits.
// 4) Code review: The code avoids dangerous protocols, blocks SSRF to internal addresses, enforces strict timeouts, limits data, avoids automatic redirects, and validates each redirect target.
// 5) Secure code output: Finalized with mitigations applied.

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

public class Task160 {

    private static final int MAX_REDIRECTS = 3;
    private static final int CONNECT_TIMEOUT_MS = 5000;
    private static final int READ_TIMEOUT_MS = 8000;
    private static final int MAX_BYTES = 1024 * 1024; // 1MB
    private static final String USER_AGENT = "Task160-HTTPClient/1.0 (+https://example.com/security)";

    // Validate scheme
    private static void validateScheme(URI uri) throws MalformedURLException {
        String scheme = uri.getScheme();
        if (scheme == null) throw new MalformedURLException("Missing scheme");
        String s = scheme.toLowerCase(Locale.ROOT);
        if (!s.equals("http") && !s.equals("https")) {
            throw new MalformedURLException("Only http/https allowed");
        }
    }

    // Validate host and port basics
    private static void validateHostAndPort(URI uri) throws MalformedURLException {
        String host = uri.getHost();
        if (host == null || host.isEmpty()) {
            throw new MalformedURLException("Missing host");
        }
        int port = uri.getPort();
        if (port < -1 || port == 0 || port > 65535) {
            throw new MalformedURLException("Invalid port");
        }
    }

    // Check InetAddress is public
    private static boolean isPublicInetAddress(InetAddress addr) {
        if (addr.isAnyLocalAddress()) return false;       // 0.0.0.0/:: 
        if (addr.isLoopbackAddress()) return false;       // 127.0.0.1/::1
        if (addr.isLinkLocalAddress()) return false;      // 169.254.0.0/16, fe80::/10
        if (addr.isSiteLocalAddress()) return false;      // RFC1918
        if (addr.isMulticastAddress()) return false;      // 224.0.0.0/4, ff00::/8
        // Additional IPv6 unique local fc00::/7
        if (addr instanceof java.net.Inet6Address) {
            byte[] b = addr.getAddress();
            int first = b[0] & 0xFF;
            if ((first & 0xFE) == 0xFC) return false; // fc00::/7
        }
        return true;
    }

    // Validate all resolved addresses are public
    private static void validatePublicResolution(URI uri) throws Exception {
        String host = uri.getHost();
        String asciiHost = IDN.toASCII(host, IDN.ALLOW_UNASSIGNED);
        InetAddress[] addrs = InetAddress.getAllByName(asciiHost);
        if (addrs == null || addrs.length == 0) {
            throw new UnknownHostException("Unable to resolve host");
        }
        for (InetAddress a : addrs) {
            if (!isPublicInetAddress(a)) {
                throw new SecurityException("Blocked non-public address resolution for host");
            }
        }
    }

    // Fetch with manual redirect handling and limits
    public static String fetchUrl(String urlStr) {
        try {
            URI current = new URI(urlStr.trim());
            int redirects = 0;
            StringBuilder trace = new StringBuilder();

            while (true) {
                validateScheme(current);
                validateHostAndPort(current);
                validatePublicResolution(current);

                URL url = current.toURL();
                HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                conn.setInstanceFollowRedirects(false); // manual handling
                conn.setRequestMethod("GET");
                conn.setConnectTimeout(CONNECT_TIMEOUT_MS);
                conn.setReadTimeout(READ_TIMEOUT_MS);
                conn.setRequestProperty("User-Agent", USER_AGENT);
                conn.setRequestProperty("Accept", "*/*");
                conn.setRequestProperty("Connection", "close");

                int code;
                String locationHeader = null;
                byte[] bodyBytes = null;
                try (InputStream is = new BufferedInputStream(conn.getInputStream())) {
                    // If response code is not 2xx, getInputStream throws; handle below with getErrorStream
                    code = conn.getResponseCode(); // still valid
                    if (code / 100 == 2) {
                        bodyBytes = readLimited(is, MAX_BYTES);
                    }
                } catch (Exception e) {
                    code = conn.getResponseCode();
                    InputStream es = conn.getErrorStream();
                    if (es != null) {
                        try (InputStream eis = new BufferedInputStream(es)) {
                            bodyBytes = readLimited(eis, MAX_BYTES);
                        }
                    } else {
                        bodyBytes = new byte[0];
                    }
                } finally {
                    locationHeader = conn.getHeaderField("Location");
                    conn.disconnect();
                }

                trace.append("URL: ").append(current).append(" -> Status: ").append(code).append("\n");

                if (code / 100 == 3 && locationHeader != null && redirects < MAX_REDIRECTS) {
                    URI next = current.resolve(locationHeader);
                    // Validate next before following
                    validateScheme(next);
                    validateHostAndPort(next);
                    validatePublicResolution(next);
                    current = next;
                    redirects++;
                    continue;
                }

                String contentType = conn.getHeaderField("Content-Type");
                int length = bodyBytes != null ? bodyBytes.length : 0;
                String preview = safePreview(bodyBytes, 200);
                return "Final URL: " + current +
                        "\nHTTP Status: " + code +
                        "\nContent-Type: " + (contentType != null ? contentType : "unknown") +
                        "\nBytes Read (capped): " + length +
                        "\nRedirects Followed: " + redirects +
                        "\nTrace:\n" + trace +
                        "\nPreview (first 200 bytes as UTF-8, may be truncated):\n" + preview;
            }
        } catch (SecurityException se) {
            return "ERROR: " + se.getMessage();
        } catch (Exception ex) {
            return "ERROR: " + ex.getClass().getSimpleName() + ": " + ex.getMessage();
        }
    }

    private static byte[] readLimited(InputStream is, int max) throws Exception {
        ByteArrayOutputStream bos = new ByteArrayOutputStream(Math.min(8192, max));
        byte[] buf = new byte[8192];
        int total = 0;
        int r;
        while ((r = is.read(buf)) != -1) {
            if (total + r > max) {
                int toWrite = max - total;
                if (toWrite > 0) bos.write(buf, 0, toWrite);
                break;
            } else {
                bos.write(buf, 0, r);
                total += r;
            }
        }
        return bos.toByteArray();
    }

    private static String safePreview(byte[] data, int maxChars) {
        if (data == null) return "";
        String s = new String(data, StandardCharsets.UTF_8);
        if (s.length() > maxChars) return s.substring(0, maxChars) + "...";
        return s;
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        String[] tests = new String[] {
                "https://example.com",
                "http://httpbin.org/get",
                "https://httpbin.org/redirect/1",
                "ftp://example.com",           // should be blocked (scheme)
                "http://127.0.0.1"             // should be blocked (private/loopback)
        };
        for (String t : tests) {
            System.out.println("=== Test: " + t + " ===");
            String result = fetchUrl(t);
            System.out.println(result);
            System.out.println();
        }
    }
}