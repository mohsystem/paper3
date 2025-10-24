
import java.io.*;
import java.security.*;
import java.security.cert.*;
import java.util.*;
import javax.xml.bind.DatatypeConverter;

public class Task81 {
    
    /**
     * Securely computes SHA-256 hash of a certificate
     * @param certBytes The certificate bytes
     * @return Hexadecimal string representation of the hash
     * @throws NoSuchAlgorithmException if SHA-256 is not available
     */
    public static String computeCertificateHash(byte[] certBytes) throws NoSuchAlgorithmException {
        if (certBytes == null || certBytes.length == 0) {
            throw new IllegalArgumentException("Certificate bytes cannot be null or empty");
        }
        
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        byte[] hash = digest.digest(certBytes);
        
        // Convert to hex string
        StringBuilder hexString = new StringBuilder();
        for (byte b : hash) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) {
                hexString.append('0');
            }
            hexString.append(hex);
        }
        return hexString.toString().toLowerCase();
    }
    
    /**
     * Securely checks if certificate matches known hash using constant-time comparison
     * @param certBytes The certificate bytes to check
     * @param knownHash The known hash to compare against (hexadecimal string)
     * @return true if hashes match, false otherwise
     */
    public static boolean verifyCertificateHash(byte[] certBytes, String knownHash) {
        try {
            if (knownHash == null || knownHash.isEmpty()) {
                throw new IllegalArgumentException("Known hash cannot be null or empty");
            }
            
            // Normalize known hash to lowercase and remove any whitespace/colons
            String normalizedKnownHash = knownHash.toLowerCase().replaceAll("[:\\\\s]", "");
            
            // Validate known hash format (should be 64 hex characters for SHA-256)
            if (!normalizedKnownHash.matches("^[0-9a-f]{64}$")) {
                throw new IllegalArgumentException("Invalid hash format. Expected 64 hexadecimal characters.");
            }
            
            String computedHash = computeCertificateHash(certBytes);
            
            // Use constant-time comparison to prevent timing attacks
            return MessageDigest.isEqual(
                computedHash.getBytes(),
                normalizedKnownHash.getBytes()
            );
            
        } catch (NoSuchAlgorithmException e) {
            System.err.println("SHA-256 algorithm not available: " + e.getMessage());
            return false;
        } catch (IllegalArgumentException e) {
            System.err.println("Invalid input: " + e.getMessage());
            return false;
        }
    }
    
    /**
     * Loads certificate from PEM format string
     * @param pemCert PEM formatted certificate string
     * @return X509Certificate object
     */
    public static X509Certificate loadCertificateFromPEM(String pemCert) throws CertificateException {
        if (pemCert == null || pemCert.isEmpty()) {
            throw new IllegalArgumentException("PEM certificate cannot be null or empty");
        }
        
        // Remove PEM headers and whitespace
        String certData = pemCert
            .replaceAll("-----BEGIN CERTIFICATE-----", "")
            .replaceAll("-----END CERTIFICATE-----", "")
            .replaceAll("\\\\s", "");
        
        byte[] decoded = Base64.getDecoder().decode(certData);
        
        CertificateFactory cf = CertificateFactory.getInstance("X.509");
        return (X509Certificate) cf.generateCertificate(new ByteArrayInputStream(decoded));
    }
    
    public static void main(String[] args) {
        System.out.println("=== SSL Certificate Hash Verification Tests ===\\n");
        
        // Test Case 1: Valid certificate with matching hash
        try {
            String testCert1 = "Test Certificate Data 1";
            byte[] certBytes1 = testCert1.getBytes();
            String hash1 = computeCertificateHash(certBytes1);
            boolean result1 = verifyCertificateHash(certBytes1, hash1);
            System.out.println("Test 1 - Valid certificate with matching hash:");
            System.out.println("  Computed Hash: " + hash1);
            System.out.println("  Result: " + (result1 ? "PASS (Hashes match)" : "FAIL"));
            System.out.println();
        } catch (Exception e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test Case 2: Valid certificate with non-matching hash
        try {
            String testCert2 = "Test Certificate Data 2";
            byte[] certBytes2 = testCert2.getBytes();
            String wrongHash = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
            boolean result2 = verifyCertificateHash(certBytes2, wrongHash);
            System.out.println("Test 2 - Valid certificate with non-matching hash:");
            System.out.println("  Expected: FAIL (Hashes don't match)");
            System.out.println("  Result: " + (!result2 ? "PASS (Correctly identified mismatch)" : "FAIL"));
            System.out.println();
        } catch (Exception e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test Case 3: Hash with colons (common format)
        try {
            String testCert3 = "Test Certificate Data 3";
            byte[] certBytes3 = testCert3.getBytes();
            String hash3 = computeCertificateHash(certBytes3);
            String hashWithColons = hash3.replaceAll("(.{2})", "$1:").substring(0, hash3.length() * 2 - 1);
            boolean result3 = verifyCertificateHash(certBytes3, hashWithColons);
            System.out.println("Test 3 - Hash with colon separators:");
            System.out.println("  Hash format: " + hashWithColons.substring(0, 20) + "...");
            System.out.println("  Result: " + (result3 ? "PASS (Correctly handled format)" : "FAIL"));
            System.out.println();
        } catch (Exception e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test Case 4: Invalid hash format
        try {
            String testCert4 = "Test Certificate Data 4";
            byte[] certBytes4 = testCert4.getBytes();
            String invalidHash = "invalid_hash_format";
            boolean result4 = verifyCertificateHash(certBytes4, invalidHash);
            System.out.println("Test 4 - Invalid hash format:");
            System.out.println("  Expected: FAIL (Invalid format)");
            System.out.println("  Result: " + (!result4 ? "PASS (Correctly rejected invalid format)" : "FAIL"));
            System.out.println();
        } catch (Exception e) {
            System.err.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test Case 5: Empty certificate bytes
        try {
            byte[] emptyCert = new byte[0];
            String someHash = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
            boolean result5 = verifyCertificateHash(emptyCert, someHash);
            System.out.println("Test 5 - Empty certificate:");
            System.out.println("  Expected: FAIL (Empty certificate)");
            System.out.println("  Result: " + (!result5 ? "PASS (Correctly handled empty input)" : "FAIL"));
            System.out.println();
        } catch (Exception e) {
            System.err.println("Test 5 handled gracefully: " + e.getMessage());
        }
    }
}
