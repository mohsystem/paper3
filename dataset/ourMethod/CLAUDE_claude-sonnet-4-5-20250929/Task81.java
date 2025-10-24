
import javax.net.ssl.*;
import java.io.*;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Task81 {
    
    private static final int DEFAULT_PORT = 443;
    private static final int DEFAULT_TIMEOUT = 10000; // 10 seconds in milliseconds
    private static final Set<String> ALLOWED_ALGORITHMS = new HashSet<>(Arrays.asList("SHA-256", "SHA-512"));
    
    /**
     * Retrieves the SSL certificate from a remote host.
     * Enforces TLS 1.2+ and proper certificate validation to prevent MITM attacks.
     * 
     * @param hostname The hostname to connect to
     * @param port The port to connect on
     * @param timeout Connection timeout in milliseconds
     * @return The DER-encoded certificate bytes, or null on error
     */
    public static byte[] getCertificateFromHost(String hostname, int port, int timeout) {
        // Input validation: hostname must be non-empty (CWE-20)
        if (hostname == null || hostname.trim().isEmpty()) {
            System.err.println("Error: Invalid hostname");
            return null;
        }
        
        // Input validation: port must be in valid range (CWE-20)
        if (port < 1 || port > 65535) {
            System.err.println("Error: Invalid port number");
            return null;
        }
        
        // Input validation: timeout must be positive (CWE-20)
        if (timeout < 1) {
            System.err.println("Error: Invalid timeout value");
            return null;
        }
        
        SSLSocket sslSocket = null;
        
        try {
            // Create SSL context with secure defaults (CWE-295, CWE-297)
            SSLContext sslContext = SSLContext.getInstance("TLS");
            sslContext.init(null, null, null);
            
            SSLSocketFactory factory = sslContext.getSocketFactory();
            
            // Create SSL socket with timeout
            sslSocket = (SSLSocket) factory.createSocket();
            sslSocket.setSoTimeout(timeout);
            
            // Force TLS 1.2 and TLS 1.3 only to prevent downgrade attacks (CWE-327)
            String[] protocols = {"TLSv1.2", "TLSv1.3"};
            sslSocket.setEnabledProtocols(protocols);
            
            // Connect to the host
            sslSocket.connect(new java.net.InetSocketAddress(hostname, port), timeout);
            
            // Start handshake to retrieve certificate
            sslSocket.startHandshake();
            
            // Get the certificate chain
            Certificate[] certs = sslSocket.getSession().getPeerCertificates();
            
            if (certs == null || certs.length == 0) {
                System.err.println("Error: No certificates received");
                return null;
            }
            
            // Return the first certificate (server certificate) in DER format
            return certs[0].getEncoded();
            
        } catch (SSLException e) {
            System.err.println("SSL Error: " + e.getMessage());
            return null;
        } catch (java.net.SocketTimeoutException e) {
            System.err.println("Error: Connection timeout");
            return null;
        } catch (java.net.UnknownHostException e) {
            System.err.println("Error: Unable to resolve hostname");
            return null;
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            return null;
        } finally {
            // Clean up resources (CWE-404)
            if (sslSocket != null) {
                try {
                    sslSocket.close();
                } catch (IOException e) {
                    // Ignore close exception
                }
            }
        }
    }
    
    /**
     * Computes the hash of a certificate using a strong cryptographic algorithm.
     * Only allows SHA-256 and SHA-512 to prevent use of weak algorithms.
     * 
     * @param certDer The DER-encoded certificate bytes
     * @param algorithm Hash algorithm to use (SHA-256 or SHA-512)
     * @return Hex-encoded hash string, or null on error
     */
    public static String computeCertificateHash(byte[] certDer, String algorithm) {
        // Input validation: certificate data must be non-empty (CWE-20)
        if (certDer == null || certDer.length == 0) {
            System.err.println("Error: Invalid certificate data");
            return null;
        }
        
        // Input validation: only allow secure hash algorithms (CWE-327)
        // SHA-256 and SHA-512 are secure; MD5 and SHA-1 are forbidden
        if (algorithm == null || !ALLOWED_ALGORITHMS.contains(algorithm)) {
            System.err.println("Error: Unsupported hash algorithm. Use: " + ALLOWED_ALGORITHMS);
            return null;
        }
        
        try {
            MessageDigest digest = MessageDigest.getInstance(algorithm);
            byte[] hashBytes = digest.digest(certDer);
            
            // Convert to hex string
            StringBuilder hexString = new StringBuilder();
            for (byte b : hashBytes) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            
            return hexString.toString();
            
        } catch (Exception e) {
            System.err.println("Error computing hash: " + e.getMessage());
            return null;
        }
    }
    
    /**
     * Verifies that a remote host's SSL certificate matches a known hash.
     * Uses constant-time comparison to prevent timing attacks.
     * 
     * @param hostname The hostname to verify
     * @param knownHash The expected certificate hash (hex string)
     * @param port The port to connect on
     * @param algorithm Hash algorithm to use
     * @return true if certificate matches, false otherwise
     */
    public static boolean verifyCertificateHash(String hostname, String knownHash, 
                                                int port, String algorithm) {
        // Input validation: hostname must be non-empty (CWE-20)
        if (hostname == null || hostname.trim().isEmpty()) {
            System.err.println("Error: Invalid hostname");
            return false;
        }
        
        // Input validation: known hash must be non-empty hex string (CWE-20)
        if (knownHash == null || knownHash.trim().isEmpty()) {
            System.err.println("Error: Invalid known hash");
            return false;
        }
        
        // Validate known hash is valid hex
        if (!knownHash.matches("^[0-9a-fA-F]+$")) {
            System.err.println("Error: Known hash must be a valid hex string");
            return false;
        }
        
        // Validate expected hash length based on algorithm
        int expectedLength = 0;
        if ("SHA-256".equals(algorithm)) {
            expectedLength = 64;
        } else if ("SHA-512".equals(algorithm)) {
            expectedLength = 128;
        }
        
        if (knownHash.length() != expectedLength) {
            System.err.println("Error: Hash length mismatch for " + algorithm);
            return false;
        }
        
        // Retrieve certificate from remote host
        byte[] certDer = getCertificateFromHost(hostname, port, DEFAULT_TIMEOUT);
        if (certDer == null) {
            return false;
        }
        
        // Compute hash of retrieved certificate
        String computedHash = computeCertificateHash(certDer, algorithm);
        if (computedHash == null) {
            return false;
        }
        
        // Use constant-time comparison to prevent timing attacks
        // This prevents attackers from determining the hash through timing analysis
        try {
            String knownLower = knownHash.toLowerCase();
            String computedLower = computedHash.toLowerCase();
            
            if (MessageDigest.isEqual(knownLower.getBytes(StandardCharsets.UTF_8), 
                                     computedLower.getBytes(StandardCharsets.UTF_8))) {
                return true;
            } else {
                // Do not reveal which part of the hash mismatched (fail closed)
                System.err.println("Error: Certificate hash does not match known hash");
                return false;
            }
            
        } catch (Exception e) {
            System.err.println("Error during hash comparison: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== SSL Certificate Hash Verification Tests ===\\n");
        
        // Test Case 1: Retrieve and hash example.com certificate
        System.out.println("Test 1: Retrieve and hash example.com certificate");
        try {
            byte[] cert = getCertificateFromHost("example.com", DEFAULT_PORT, DEFAULT_TIMEOUT);
            if (cert != null) {
                String hashVal = computeCertificateHash(cert, "SHA-256");
                if (hashVal != null) {
                    System.out.println("Success: SHA-256 hash = " + hashVal + "\\n");
                } else {
                    System.out.println("Failed: Could not compute hash\\n");
                }
            } else {
                System.out.println("Failed: Could not retrieve certificate\\n");
            }
        } catch (Exception e) {
            System.out.println("Test 1 exception: " + e.getMessage() + "\\n");
        }
        
        // Test Case 2: Verify with incorrect hash (should fail)
        System.out.println("Test 2: Verify example.com with incorrect hash (should fail)");
        String incorrectHash = "0".repeat(64);
        boolean result = verifyCertificateHash("example.com", incorrectHash, DEFAULT_PORT, "SHA-256");
        System.out.println("Result: " + (!result ? "PASS - Correctly rejected" : "FAIL - Incorrectly accepted") + "\\n");
        
        // Test Case 3: Invalid hostname
        System.out.println("Test 3: Invalid hostname (should fail gracefully)");
        result = verifyCertificateHash("", "0".repeat(64), DEFAULT_PORT, "SHA-256");
        System.out.println("Result: " + (!result ? "PASS - Handled error" : "FAIL") + "\\n");
        
        // Test Case 4: Invalid port
        System.out.println("Test 4: Invalid port number (should fail)");
        byte[] cert = getCertificateFromHost("example.com", 99999, DEFAULT_TIMEOUT);
        System.out.println("Result: " + (cert == null ? "PASS - Handled error" : "FAIL") + "\\n");
        
        // Test Case 5: Invalid hash algorithm
        System.out.println("Test 5: Unsupported hash algorithm (should fail)");
        byte[] testData = "test certificate data".getBytes(StandardCharsets.UTF_8);
        String hashResult = computeCertificateHash(testData, "MD5");
        System.out.println("Result: " + (hashResult == null ? "PASS - Rejected insecure algorithm" : "FAIL") + "\\n");
        
        System.out.println("=== All tests completed ===");
    }
}
