import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.UnknownHostException;

public class Task160 {

    private static final int CONNECT_TIMEOUT = 5000; // 5 seconds
    private static final int READ_TIMEOUT = 5000; // 5 seconds
    private static final int MAX_RESPONSE_SIZE = 4096; // Limit response to 4KB to prevent DoS

    /**
     * Makes an HTTP GET request to a given URL after performing security checks.
     *
     * @param urlString The URL to make a request to.
     * @return The first part of the HTTP response body, or an error message.
     */
    public static String makeHttpRequest(String urlString) {
        try {
            // 1. URL Parsing and Validation
            URL url = new URL(urlString);

            // 2. Security Check: Protocol validation (Allowlist)
            String protocol = url.getProtocol();
            if (!"http".equalsIgnoreCase(protocol) && !"https".equalsIgnoreCase(protocol)) {
                return "Error: Invalid protocol. Only HTTP and HTTPS are allowed.";
            }

            // 3. Security Check: SSRF Prevention
            String host = url.getHost();
            InetAddress address = InetAddress.getByName(host);

            if (address.isLoopbackAddress() || address.isSiteLocalAddress()) {
                return "Error: SSRF attempt detected. Requests to local/private networks are not allowed.";
            }

            // 4. Make the Connection
            HttpURLConnection con = (HttpURLConnection) url.openConnection();
            con.setRequestMethod("GET");
            con.setConnectTimeout(CONNECT_TIMEOUT);
            con.setReadTimeout(READ_TIMEOUT);
            
            // Security: Disable redirects to prevent bypassing SSRF checks on the initial URL
            con.setInstanceFollowRedirects(false);

            int status = con.getResponseCode();
            
            StringBuilder responseContent = new StringBuilder();
            responseContent.append("Status Code: ").append(status).append("\n");

            // Read response body, but limit the size to prevent DoS
            try (BufferedReader in = new BufferedReader(new InputStreamReader(con.getInputStream()))) {
                String inputLine;
                int totalRead = 0;
                responseContent.append("Response Body (first ").append(MAX_RESPONSE_SIZE).append(" bytes):\n");
                while ((inputLine = in.readLine()) != null && totalRead < MAX_RESPONSE_SIZE) {
                    responseContent.append(inputLine).append("\n");
                    totalRead += inputLine.length() + 1;
                }
            } catch (IOException e) {
                 // This can happen for non-2xx status codes, which is expected.
                 // We still want to return the status code.
                 responseContent.append("Could not read response body (or no body present).");
            } finally {
                con.disconnect();
            }

            return responseContent.toString();

        } catch (MalformedURLException e) {
            return "Error: Invalid URL format. " + e.getMessage();
        } catch (UnknownHostException e) {
            return "Error: Host could not be resolved. " + e.getMessage();
        } catch (IOException e) {
            return "Error: Could not connect to the URL. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String[] testUrls = {
            "https://api.ipify.org", // Valid public URL
            "http://google.com",       // Valid public URL that redirects
            "http://127.0.0.1",        // SSRF test (loopback)
            "http://192.168.1.1",      // SSRF test (private network)
            "https://thissitedoesnotexist.invalidtld", // Invalid host
            "file:///etc/passwd"       // Invalid protocol test
        };

        for (String url : testUrls) {
            System.out.println("=========================================");
            System.out.println("Requesting URL: " + url);
            System.out.println("-----------------------------------------");
            String response = makeHttpRequest(url);
            System.out.println(response);
            System.out.println("=========================================\n");
        }
    }
}