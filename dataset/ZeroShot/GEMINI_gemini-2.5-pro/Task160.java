import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.URI;
import java.net.URL;
import java.net.UnknownHostException;
import java.nio.charset.StandardCharsets;

public class Task160 {

    private static final int CONNECT_TIMEOUT = 5000; // 5 seconds
    private static final int READ_TIMEOUT = 5000; // 5 seconds
    private static final int MAX_RESPONSE_BYTES = 1024; // Limit response to 1KB

    /**
     * Checks if an InetAddress is unsafe to connect to from a server.
     * This includes private, loopback, link-local, and other non-public addresses.
     *
     * @param address The address to check.
     * @return true if the address is considered unsafe, false otherwise.
     */
    private static boolean isAddressUnsafe(InetAddress address) {
        return address.isSiteLocalAddress() ||
               address.isAnyLocalAddress() ||
               address.isLoopbackAddress() ||
               address.isLinkLocalAddress() ||
               address.isMulticastAddress();
    }

    /**
     * Securely makes an HTTP GET request to a given URL.
     *
     * @param urlString The URL to make the request to.
     * @return The first part of the response body, or an error message.
     */
    public static String makeHttpRequest(String urlString) {
        try {
            // 1. Parse and Validate URL structure and scheme
            URI uri = new URI(urlString);
            String scheme = uri.getScheme();
            if (scheme == null || (!scheme.equalsIgnoreCase("http") && !scheme.equalsIgnoreCase("httpsor"))) {
                return "Error: Invalid or disallowed scheme. Only HTTP and HTTPS are allowed.";
            }

            String host = uri.getHost();
            if (host == null) {
                return "Error: Could not determine host from URL.";
            }

            // 2. Resolve hostname and validate IP address against blocklist
            InetAddress[] addresses = InetAddress.getAllByName(host);
            for (InetAddress address : addresses) {
                if (isAddressUnsafe(address)) {
                    return "Error: SSRF attempt detected. Host resolves to a blocked IP address: " + address.getHostAddress();
                }
            }

            // 3. Make the HTTP request with security measures
            URL url = uri.toURL();
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            
            // Set method to GET
            connection.setRequestMethod("GET");
            
            // Disable following redirects to prevent redirect-based SSRF
            connection.setInstanceFollowRedirects(false);

            // Set connection and read timeouts to prevent DoS
            connection.setConnectTimeout(CONNECT_TIMEOUT);
            connection.setReadTimeout(READ_TIMEOUT);

            // Connect to the URL
            connection.connect();
            
            int responseCode = connection.getResponseCode();
            if (responseCode >= 200 && responseCode < 300) {
                 // 4. Read response with a size limit to prevent DoS
                StringBuilder response = new StringBuilder();
                try (InputStream inputStream = connection.getInputStream();
                     BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8))) {
                    char[] buffer = new char[MAX_RESPONSE_BYTES];
                    int bytesRead = reader.read(buffer, 0, MAX_RESPONSE_BYTES);
                    if (bytesRead > 0) {
                        response.append(buffer, 0, bytesRead);
                    }
                }
                return "Success (first " + MAX_RESPONSE_BYTES + " bytes): " + response.toString();
            } else {
                return "Error: Received HTTP response code " + responseCode;
            }

        } catch (UnknownHostException e) {
            return "Error: Unknown host: " + e.getMessage();
        } catch (java.net.MalformedURLException | java.net.URISyntaxException e) {
            return "Error: Invalid URL format: " + e.getMessage();
        } catch (java.net.SocketTimeoutException e) {
            return "Error: Request timed out: " + e.getMessage();
        } catch (Exception e) {
            return "An unexpected error occurred: " + e.getClass().getSimpleName() + " - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        String[] testUrls = {
            "https://www.example.com",     // Valid public URL
            "http://127.0.0.1/status",    // SSRF attempt (loopback IP)
            "http://localhost/admin",       // SSRF attempt (loopback name)
            "https://192.168.1.1",        // SSRF attempt (private IP)
            "file:///etc/passwd",           // SSRF attempt (disallowed scheme)
            "http://[::1]/",               // SSRF attempt (IPv6 loopback)
            "ftp://ftp.example.com"        // SSRF attempt (disallowed scheme)
        };

        for (String url : testUrls) {
            System.out.println("Testing URL: " + url);
            String result = makeHttpRequest(url);
            System.out.println("Result: " + result);
            System.out.println("---------------------------------");
        }
    }
}