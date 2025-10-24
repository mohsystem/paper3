import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.SSLSession;
import java.security.MessageDigest;
import java.security.cert.X509Certificate;

public class Task81 {

    /**
     * Establishes a TLS connection to a given host, retrieves its certificate,
     * and compares its SHA-256 hash with a known hash.
     *
     * @param hostname The hostname to connect to.
     * @param port The port to connect to (usually 443 for HTTPS).
     * @param knownHash The expected SHA-256 hash of the certificate in hexadecimal format.
     * @return true if the certificate's hash matches the known hash, false otherwise.
     */
    public static boolean checkCertificateHash(String hostname, int port, String knownHash) {
        try {
            SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
            // The try-with-resources statement ensures the socket is closed automatically.
            try (SSLSocket socket = (SSLSocket) factory.createSocket(hostname, port)) {
                // Set a reasonable timeout to prevent indefinite blocking
                socket.setSoTimeout(5000); // 5 seconds
                socket.startHandshake();
                SSLSession session = socket.getSession();
                java.security.cert.Certificate[] certs = session.getPeerCertificates();

                if (certs.length > 0 && certs[0] instanceof X509Certificate) {
                    X509Certificate cert = (X509Certificate) certs[0];
                    byte[] encodedCert = cert.getEncoded();

                    MessageDigest md = MessageDigest.getInstance("SHA-256");
                    byte[] digest = md.digest(encodedCert);

                    // Convert byte array to a hex string
                    StringBuilder hexString = new StringBuilder(2 * digest.length);
                    for (byte b : digest) {
                        String hex = Integer.toHexString(0xff & b);
                        if (hex.length() == 1) {
                            hexString.append('0');
                        }
                        hexString.append(hex);
                    }

                    return knownHash.equalsIgnoreCase(hexString.toString());
                }
            }
        } catch (Exception e) {
            // Any exception during connection, handshake, or processing means failure.
            // System.err.println("Error checking " + hostname + ": " + e.getMessage());
            return false;
        }
        return false;
    }

    public static void main(String[] args) {
        // The SHA-256 hash of the certificate for sha256.badssl.com
        // This hash is stable for this specific test site.
        String correctHash = "42a3219451add031a017243a05c45b882d9d1502425576a8049e7b39920194eb";
        String incorrectHash = "0000000000000000000000000000000000000000000000000000000000000000";

        // Test cases
        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Success - Correct host and correct hash
        boolean test1 = checkCertificateHash("sha256.badssl.com", 443, correctHash);
        System.out.println("Test 1 (Success): " + (test1 ? "Passed" : "Failed"));

        // Test Case 2: Failure - Correct host but incorrect hash
        boolean test2 = checkCertificateHash("sha256.badssl.com", 443, incorrectHash);
        System.out.println("Test 2 (Failure - Wrong Hash): " + (!test2 ? "Passed" : "Failed"));

        // Test Case 3: Failure - Wrong host
        boolean test3 = checkCertificateHash("google.com", 443, correctHash);
        System.out.println("Test 3 (Failure - Wrong Host): " + (!test3 ? "Passed" : "Failed"));

        // Test Case 4: Failure - Non-existent host
        boolean test4 = checkCertificateHash("nonexistent.domain.local", 443, correctHash);
        System.out.println("Test 4 (Failure - Non-existent Host): " + (!test4 ? "Passed" : "Failed"));
        
        // Test Case 5: Failure - Host does not support SSL on the port
        boolean test5 = checkCertificateHash("neverssl.com", 443, correctHash);
        System.out.println("Test 5 (Failure - No SSL): " + (!test5 ? "Passed" : "Failed"));
    }
}