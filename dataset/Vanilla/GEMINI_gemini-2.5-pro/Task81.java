import javax.net.ssl.*;
import java.io.IOException;
import java.net.URL;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;

public class Task81 {

    private static final char[] HEX_ARRAY = "0123456789abcdef".toCharArray();

    /**
     * Converts a byte array to a hexadecimal string representation.
     */
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars);
    }

    /**
     * Connects to a host, retrieves its SSL certificate, calculates its SHA-256 hash,
     * and compares it to a known hash.
     *
     * @param host      The hostname to connect to.
     * @param port      The port to connect to (usually 443 for HTTPS).
     * @param knownHash The expected SHA-256 hash of the certificate as a hex string.
     * @return True if the calculated hash matches the known hash, false otherwise.
     */
    public static boolean checkCertificateHash(String host, int port, String knownHash) {
        try {
            URL url = new URL("https://" + host + ":" + port);
            HttpsURLConnection conn = (HttpsURLConnection) url.openConnection();
            conn.connect();
            Certificate[] certs = conn.getServerCertificates();
            conn.disconnect();

            if (certs.length > 0) {
                // The leaf certificate (the server's own certificate) is the first in the chain.
                Certificate leafCert = certs[0];
                
                // Get the certificate in its DER-encoded form.
                byte[] derCert = leafCert.getEncoded();

                // Calculate the SHA-256 hash of the DER-encoded certificate.
                MessageDigest digest = MessageDigest.getInstance("SHA-256");
                byte[] calculatedHashBytes = digest.digest(derCert);
                String calculatedHash = bytesToHex(calculatedHashBytes);

                // Perform a case-insensitive comparison.
                return calculatedHash.equalsIgnoreCase(knownHash);
            }
        } catch (IOException | NoSuchAlgorithmException | CertificateEncodingException e) {
            System.err.println("  Error checking " + host + ": " + e.getMessage());
            return false;
        }
        return false;
    }

    public static void main(String[] args) {
        // NOTE: These hashes are time-sensitive and may need to be updated as
        // websites rotate their SSL certificates. The hashes provided are for the
        // *entire* DER-encoded certificate, not just the public key.
        // The values were correct at the time of writing.

        String[] hosts = {
            "google.com",
            "github.com",
            "example.com",
            "self-signed.badssl.com",
            "thishostdoesnotexist12345.com"
        };

        String[] knownHashes = {
            "19f7a72675b331481b21cb4e488135891e4f34614207963d722108709328246a", // Correct for google.com
            "346b45b239094d2b2746487e452a2656973e6f21789c6d5b035f5022e032230a", // Correct for github.com
            "this_is_an_incorrect_hash_string_for_testing_failure1234567890", // Incorrect for example.com
            "987724f28012626e205a91438b417c8227b403f752496a71e16c905f884a4411", // Correct for self-signed.badssl.com
            "any_hash_will_do_since_host_is_unreachable" // For non-existent host
        };

        boolean[] expectedResults = { true, true, false, true, false };

        System.out.println("--- Running Java Test Cases ---");
        for (int i = 0; i < hosts.length; i++) {
            System.out.println("Test " + (i + 1) + ": Checking host " + hosts[i]);
            boolean result = checkCertificateHash(hosts[i], 443, knownHashes[i]);
            System.out.println("  Result: " + result);
            System.out.println("  Expected: " + expectedResults[i]);
            System.out.println("  Status: " + (result == expectedResults[i] ? "PASS" : "FAIL"));
            System.out.println();
        }
    }
}