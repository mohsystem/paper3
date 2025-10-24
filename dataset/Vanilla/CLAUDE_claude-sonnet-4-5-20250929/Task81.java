
import java.security.MessageDigest;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.io.ByteArrayInputStream;
import java.util.Base64;

public class Task81 {
    public static boolean checkCertificateHash(String certificatePem, String knownHash, String algorithm) {
        try {
            // Remove PEM headers and decode base64
            String certContent = certificatePem
                .replace("-----BEGIN CERTIFICATE-----", "")
                .replace("-----END CERTIFICATE-----", "")
                .replaceAll("\\\\s", "");
            
            byte[] certBytes = Base64.getDecoder().decode(certContent);
            
            // Create certificate from bytes
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            Certificate cert = cf.generateCertificate(new ByteArrayInputStream(certBytes));
            
            // Get certificate encoded bytes
            byte[] encoded = cert.getEncoded();
            
            // Calculate hash
            MessageDigest digest = MessageDigest.getInstance(algorithm);
            byte[] hash = digest.digest(encoded);
            
            // Convert to hex string
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            
            // Compare with known hash (case-insensitive)
            return hexString.toString().equalsIgnoreCase(knownHash.replace(":", "").replace(" ", ""));
            
        } catch (Exception e) {
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test certificate (self-signed example)
        String testCert = "MIICWzCCAcSgAwIBAgIJAL7xG3YqkqWGMA0GCSqGSIb3DQEBCwUAMEUxCzAJBgNV" +
                         "BAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBX" +
                         "aWRnaXRzIFB0eSBMdGQwHhcNMjEwMTAxMDAwMDAwWhcNMjIwMTAxMDAwMDAwWjBF" +
                         "MQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50" +
                         "ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKB" +
                         "gQDHc0BcF8y6tN3jKBvXvL7VdqOYWlYwXMdGpLvXgNF5h6m6R7X9J4qH8o4F3k9Q" +
                         "y6z8K7dF9e3pQ6mL4x2X7fG5j8K9L3m4x5N6p7qQ8R3j4K9L2m5x6N7q8R3j5K9L" +
                         "3m5x7N8q9R4j6K0L4m6x8N0q0R5j7K1L5m7x9N1q1R6j8K2L6m8xQIDAQABo1Aw" +
                         "TjAdBgNVHQ4EFgQU8h3j9K0L4m6x8N0q0R5j7K1L5m8wHwYDVR0jBBgwFoAU8h3j" +
                         "9K0L4m6x8N0q0R5j7K1L5m8wDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOB" +
                         "gQBZ3j9K1L5m7x9N1q1R6j8K2L6m8x0N2q2R7j9K3L7m9xN3q3R8jK4L8mxN4q4R" +
                         "9jK5L9mx0N5q5R0jK6LmxN6q6R1jK7LmxN7q7R2jK8LmxN8q8R3jK9Lmx";
        
        // Test case 1: Valid SHA-256 hash
        System.out.println("Test 1 - Valid SHA-256: " + 
            checkCertificateHash("-----BEGIN CERTIFICATE-----\\n" + testCert + "\\n-----END CERTIFICATE-----",
                                "a1b2c3d4e5f6", "SHA-256"));
        
        // Test case 2: Invalid hash
        System.out.println("Test 2 - Invalid hash: " + 
            checkCertificateHash("-----BEGIN CERTIFICATE-----\\n" + testCert + "\\n-----END CERTIFICATE-----",
                                "0000000000000000", "SHA-256"));
        
        // Test case 3: SHA-1 algorithm
        System.out.println("Test 3 - SHA-1: " + 
            checkCertificateHash("-----BEGIN CERTIFICATE-----\\n" + testCert + "\\n-----END CERTIFICATE-----",
                                "abcdef123456", "SHA-1"));
        
        // Test case 4: MD5 algorithm
        System.out.println("Test 4 - MD5: " + 
            checkCertificateHash("-----BEGIN CERTIFICATE-----\\n" + testCert + "\\n-----END CERTIFICATE-----",
                                "fedcba654321", "MD5"));
        
        // Test case 5: Invalid certificate
        System.out.println("Test 5 - Invalid cert: " + 
            checkCertificateHash("-----BEGIN CERTIFICATE-----\\nINVALID\\n-----END CERTIFICATE-----",
                                "a1b2c3d4e5f6", "SHA-256"));
    }
}
