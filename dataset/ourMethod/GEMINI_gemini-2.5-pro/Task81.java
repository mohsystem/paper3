import javax.net.ssl.*;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import java.util.Collections;

public class Task81 {

    /**
     * Checks if the SSL certificate of a remote host matches a known hash.
     *
     * @param host       The hostname to connect to.
     * @param port       The port to connect to.
     * @param knownHash  The known certificate hash as a hex string.
     * @param algorithm  The hashing algorithm (e.g., "SHA-256").
     * @return True if the certificate hash matches, false otherwise.
     */
    public static boolean checkCertificateHash(String host, int port, String knownHash, String algorithm) {
        if (host == null || host.trim().isEmpty() || knownHash == null || algorithm == null) {
            return false;
        }

        try {
            SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
            // The socket will be closed automatically by try-with-resources
            try (SSLSocket socket = (SSLSocket) factory.createSocket(host, port)) {
                // Enable SNI for virtual hosting
                SSLParameters sslParams = new SSLParameters();
                sslParams.setServerNames(Collections.singletonList(new SNIHostName(host)));
                socket.setSSLParameters(sslParams);

                socket.startHandshake();
                SSLSession session = socket.getSession();
                Certificate[] certs = session.getPeerCertificates();

                if (certs.length > 0 && certs[0] instanceof X509Certificate) {
                    X509Certificate cert = (X509Certificate) certs[0];
                    byte[] encodedCert = cert.getEncoded();

                    MessageDigest md = MessageDigest.getInstance(algorithm);
                    byte[] digest = md.digest(encodedCert);

                    String calculatedHash = bytesToHex(digest);
                    
                    // For this example, case-insensitive string comparison is sufficient.
                    // For higher security, convert the known hex hash to bytes and use MessageDigest.isEqual.
                    return calculatedHash.equalsIgnoreCase(knownHash);
                }
            }
        } catch (NoSuchAlgorithmException e) {
            System.err.println("Error: Hashing algorithm not supported: " + algorithm);
        } catch (CertificateEncodingException e) {
            System.err.println("Error: Could not encode certificate for host: " + host);
        } catch (UnknownHostException e) {
            System.err.println("Error: Unknown host: " + host);
        } catch (IOException e) {
            System.err.println("Error connecting to " + host + ":" + port + " - " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
        }
        return false;
    }

    private static String bytesToHex(byte[] hash) {
        StringBuilder hexString = new StringBuilder(2 * hash.length);
        for (byte b : hash) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) {
                hexString.append('0');
            }
            hexString.append(hex);
        }
        return hexString.toString();
    }

    public static void main(String[] args) {
        // NOTE: These hashes are time-sensitive and will change when certificates are renewed.
        // You may need to update them for the tests to pass.
        final String GOOGLE_SHA256 = "1a3b865582e022f42a6c8e317c223c3b0693a61f2382855140b28e57973c150c";
        final String GITHUB_SHA256 = "1980318288b3986a420f4c0842271676f45237c95a25e6215b22b069d2c55b9e";
        final String FAKE_HASH = "0000000000000000000000000000000000000000000000000000000000000000";

        System.out.println("Running test cases...");

        // Test Case 1: Positive match for google.com
        boolean test1 = checkCertificateHash("www.google.com", 443, GOOGLE_SHA256, "SHA-256");
        System.out.println("1. www.google.com with correct hash: " + (test1 ? "PASS" : "FAIL"));

        // Test Case 2: Negative match (wrong hash) for google.com
        boolean test2 = checkCertificateHash("www.google.com", 443, FAKE_HASH, "SHA-256");
        System.out.println("2. www.google.com with incorrect hash: " + (!test2 ? "PASS" : "FAIL"));

        // Test Case 3: Negative match (wrong host for the hash)
        boolean test3 = checkCertificateHash("www.github.com", 443, GOOGLE_SHA256, "SHA-256");
        System.out.println("3. www.github.com with google.com hash: " + (!test3 ? "PASS" : "FAIL"));

        // Test Case 4: Connection error (invalid host)
        boolean test4 = checkCertificateHash("invalid.hostname.that.does.not.exist", 443, FAKE_HASH, "SHA-256");
        System.out.println("4. Invalid hostname: " + (!test4 ? "PASS" : "FAIL"));

        // Test Case 5: Positive match for github.com
        boolean test5 = checkCertificateHash("www.github.com", 443, GITHUB_SHA256, "SHA-256");
        System.out.println("5. www.github.com with correct hash: " + (test5 ? "PASS" : "FAIL"));
    }
}