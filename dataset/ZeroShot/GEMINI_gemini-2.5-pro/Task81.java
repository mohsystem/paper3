import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLPeerUnverifiedException;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.util.Formatter;

public class Task81 {

    /**
     * Connects to a given URL, retrieves the server's SSL certificate,
     * and checks if its SHA-256 hash matches the expected hash.
     *
     * @param urlString    The URL of the server to check (e.g., "https://example.com").
     * @param expectedHash The expected SHA-256 hash of the certificate as a hex string.
     * @return true if the certificate's hash matches the expected hash, false otherwise.
     */
    public static boolean checkCertificateHash(String urlString, String expectedHash) {
        if (urlString == null || expectedHash == null || !urlString.toLowerCase().startsWith("https://")) {
            return false;
        }

        try {
            URL url = new URL(urlString);
            HttpsURLConnection conn = (HttpsURLConnection) url.openConnection();
            conn.connect();

            Certificate[] certs = conn.getServerCertificates();
            if (certs == null || certs.length == 0) {
                System.err.println("Error: No server certificates found.");
                conn.disconnect();
                return false;
            }

            // The server's certificate is the first in the chain
            Certificate serverCert = certs[0];
            byte[] encodedCert = serverCert.getEncoded();

            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] digest = md.digest(encodedCert);
            
            String actualHash = toHexString(digest);

            conn.disconnect();
            
            return expectedHash.equalsIgnoreCase(actualHash);

        } catch (MalformedURLException e) {
            System.err.println("Error: The URL is malformed: " + e.getMessage());
        } catch (SSLPeerUnverifiedException e) {
            System.err.println("Error: SSL peer could not be verified: " + e.getMessage());
        } catch (IOException e) {
            System.err.println("Error: Could not connect to the host or I/O error: " + e.getMessage());
        } catch (CertificateEncodingException e) {
            System.err.println("Error: Could not encode the certificate: " + e.getMessage());
        } catch (NoSuchAlgorithmException e) {
            System.err.println("Error: SHA-256 algorithm not available: " + e.getMessage());
        }
        
        return false;
    }

    /**
     * Converts a byte array to a hexadecimal string.
     */
    private static String toHexString(byte[] bytes) {
        Formatter formatter = new Formatter();
        for (byte b : bytes) {
            formatter.format("%02x", b);
        }
        return formatter.toString();
    }

    public static void main(String[] args) {
        // NOTE: Certificate hashes change when certificates are renewed. 
        // These test cases use a hash for example.com that was valid at the time of writing.
        // It may need to be updated. You can get the current hash with:
        // openssl s_client -connect example.com:443 -showcerts < /dev/null 2>/dev/null | openssl x509 -outform DER | openssl dgst -sha256 -hex
        
        String exampleComUrl = "https://example.com";
        // This hash is for the example.com certificate and may change over time.
        String correctExampleComHash = "3469e34a6058e5e959725f463385623089d7b901e0a9d8329b35e69e061517a9";
        String incorrectHash = "0000000000000000000000000000000000000000000000000000000000000000";

        // Test Case 1: Correct URL and correct hash
        System.out.println("Test Case 1 (Correct): " + (checkCertificateHash(exampleComUrl, correctExampleComHash) ? "Passed" : "Failed"));

        // Test Case 2: Correct URL and incorrect hash
        System.out.println("Test Case 2 (Incorrect Hash): " + (!checkCertificateHash(exampleComUrl, incorrectHash) ? "Passed" : "Failed"));

        // Test Case 3: Different URL with the first URL's hash
        System.out.println("Test Case 3 (Mismatched Host): " + (!checkCertificateHash("https://google.com", correctExampleComHash) ? "Passed" : "Failed"));
        
        // Test Case 4: Non-existent host
        System.out.println("Test Case 4 (Invalid Host): " + (!checkCertificateHash("https://non-existent-domain-12345.com", correctExampleComHash) ? "Passed" : "Failed"));

        // Test Case 5: Non-HTTPS URL
        System.out.println("Test Case 5 (Non-HTTPS URL): " + (!checkCertificateHash("http://example.com", correctExampleComHash) ? "Passed" : "Failed"));
    }
}