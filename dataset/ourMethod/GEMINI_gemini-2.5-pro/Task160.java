import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.UnknownHostException;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Task160 {

    private static final int CONNECT_TIMEOUT = 5000; // 5 seconds
    private static final int READ_TIMEOUT = 5000; // 5 seconds
    // A regex to validate URL structure. It ensures the protocol is http or https.
    private static final Pattern URL_PATTERN = Pattern.compile(
        "^(https?):\\/\\/((([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9]))(:[0-9]+)?(\\/[^\\s]*)?$"
    );

    /**
     * Checks if a given host resolves to a private or loopback IP address to prevent SSRF.
     * @param host The hostname to check.
     * @return true if the host is private, false otherwise.
     */
    private static boolean isHostPrivate(String host) {
        try {
            InetAddress[] addresses = InetAddress.getAllByName(host);
            for (InetAddress address : addresses) {
                if (address.isSiteLocalAddress() || address.isLoopbackAddress() || address.isLinkLocalAddress()) {
                    System.err.println("Error: Host " + host + " resolves to a private IP address: " + address.getHostAddress());
                    return true;
                }
            }
        } catch (UnknownHostException e) {
            System.err.println("Error: Could not resolve host: " + host);
            return true; // Fail closed
        }
        return false;
    }

    /**
     * Makes an HTTP GET request to a given URL.
     * @param urlString The URL to make the request to.
     * @return The response body as a string, or an error message.
     */
    public static String makeHttpRequest(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            return "Error: URL is null or empty.";
        }

        if (!URL_PATTERN.matcher(urlString).matches()) {
            return "Error: Invalid URL format or protocol.";
        }

        URL url;
        try {
            url = new URL(urlString);
        } catch (MalformedURLException e) {
            return "Error: Malformed URL: " + e.getMessage();
        }
        
        if (isHostPrivate(url.getHost())) {
            return "Error: SSRF attempt detected. Requests to private networks are not allowed.";
        }

        HttpURLConnection connection = null;
        try {
            connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");
            connection.setConnectTimeout(CONNECT_TIMEOUT);
            connection.setReadTimeout(READ_TIMEOUT);
            connection.setInstanceFollowRedirects(true); // Safely follow redirects

            int responseCode = connection.getResponseCode();
            // We only accept 2xx response codes
            if (responseCode >= HttpURLConnection.HTTP_OK && responseCode < HttpURLConnection.HTTP_MULT_CHOICE) {
                StringBuilder response = new StringBuilder();
                try (BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream(), StandardCharsets.UTF_8))) {
                    String inputLine;
                    while ((inputLine = in.readLine()) != null) {
                        response.append(inputLine);
                        response.append("\n");
                    }
                }
                return response.toString();
            } else {
                return "Error: HTTP request failed with response code: " + responseCode;
            }
        } catch (IOException e) {
            return "Error: An I/O error occurred: " + e.getMessage();
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://api.github.com", // Valid HTTPS
            "http://example.com",     // Valid HTTP
            "ftp://example.com",      // Invalid protocol
            "https://127.0.0.1",      // SSRF attempt (loopback)
            "https://localhost",      // SSRF attempt (loopback)
            "http://192.168.1.1",     // SSRF attempt (private)
            "not-a-url"               // Invalid format
        };

        for (int i = 0; i < testUrls.length; i++) {
            System.out.println("---- Test Case " + (i + 1) + ": " + testUrls[i] + " ----");
            String response = makeHttpRequest(testUrls[i]);
            // Print only the first 300 characters of the response to keep output clean
            if (response.length() > 300) {
                System.out.println(response.substring(0, 300) + "...");
            } else {
                System.out.println(response);
            }
            System.out.println("-------------------------------------------\n");
        }
    }
}