
import java.security.MessageDigest;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.io.ByteArrayInputStream;
import java.util.Base64;

public class Task81 {
    public static boolean checkCertificateHash(String certificatePEM, String knownHash, String algorithm) {
        try {
            // Remove PEM headers and decode Base64
            String certContent = certificatePEM
                .replace("-----BEGIN CERTIFICATE-----", "")
                .replace("-----END CERTIFICATE-----", "")
                .replaceAll("\\\\s", "");
            
            byte[] certBytes = Base64.getDecoder().decode(certContent);
            
            // Create certificate from bytes
            CertificateFactory certFactory = CertificateFactory.getInstance("X.509");
            Certificate cert = certFactory.generateCertificate(new ByteArrayInputStream(certBytes));
            
            // Calculate hash of certificate
            MessageDigest digest = MessageDigest.getInstance(algorithm);
            byte[] certHash = digest.digest(cert.getEncoded());
            
            // Convert to hex string
            StringBuilder hexString = new StringBuilder();
            for (byte b : certHash) {
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
        String testCert = "-----BEGIN CERTIFICATE-----\\n" +
            "MIICLDCCAdKgAwIBAgIBADAKBggqhkjOPQQDAjB9MQswCQYDVQQGEwJ1czELMAkG\\n" +
            "A1UECAwCQ0ExCzAJBgNVBAcMAlNGMQ8wDQYDVQQKDAZKb3lFbnQxEDAOBgNVBAsM\\n" +
            "B05vZGUtY2ExFDASBgNVBAMMC2V4YW1wbGUuY29tMRswGQYJKoZIhvcNAQkBFgxh\\n" +
            "QGV4YW1wbGUuY29tMB4XDTIxMDEwMTAwMDAwMFoXDTMxMDEwMTAwMDAwMFowfTEL\\n" +
            "MAkGA1UEBhMCdXMxCzAJBgNVBAgMAkNBMQswCQYDVQQHDAJTRjEPMA0GA1UECgwG\\n" +
            "Sm95RW50MRAwDgYDVQQLDAdOb2RlLWNhMRQwEgYDVQQDDAtleGFtcGxlLmNvbTEb\\n" +
            "MBkGCSqGSIb3DQEJARYMYUBleGFtcGxlLmNvbTBZMBMGByqGSM49AgEGCCqGSM49\\n" +
            "AwEHA0IABEg7eqR4yMGb2N8rTOi3OKrPtGLLzqVPnzqLjLwrPmLqLnVChdPMbZHL\\n" +
            "LyZuMN0wPgwGNxQqPVLfvAp9fkqFfqajUDBOMB0GA1UdDgQWBBQ7WxJ8rYxNQvJm\\n" +
            "lzWVhFWrfVWnlDAfBgNVHSMEGDAWgBQ7WxJ8rYxNQvJmlzWVhFWrfVWnlDAMBgNV\\n" +
            "HRMEBTADAQH/MAoGCCqGSM49BAMCA0gAMEUCIQDLWlPGzNCPKqBvbLDPuLOdRVjJ\\n" +
            "kIrGqTI6+xB3L2x8hAIgCHMzH8KwHgfTLxP9gFmqFYhpUkGJvVYMqfnCMZJmLrk=\\n" +
            "-----END CERTIFICATE-----";
        
        // Test Case 1: Valid SHA-256 hash
        String hash1 = "8f43288ad272f3103b6fb1428485ea3014c0bcf547a2f1d4e8e1e2c6e5f7d8c9";
        System.out.println("Test 1 - Valid SHA-256: " + checkCertificateHash(testCert, hash1, "SHA-256"));
        
        // Test Case 2: Valid SHA-1 hash
        String hash2 = "a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0";
        System.out.println("Test 2 - Valid SHA-1: " + checkCertificateHash(testCert, hash2, "SHA-1"));
        
        // Test Case 3: Invalid hash
        String hash3 = "0000000000000000000000000000000000000000000000000000000000000000";
        System.out.println("Test 3 - Invalid hash: " + checkCertificateHash(testCert, hash3, "SHA-256"));
        
        // Test Case 4: Hash with colons
        String hash4 = "8f:43:28:8a:d2:72:f3:10:3b:6f:b1:42:84:85:ea:30:14:c0:bc:f5:47:a2:f1:d4:e8:e1:e2:c6:e5:f7:d8:c9";
        System.out.println("Test 4 - Hash with colons: " + checkCertificateHash(testCert, hash4, "SHA-256"));
        
        // Test Case 5: Empty/Invalid certificate
        System.out.println("Test 5 - Invalid cert: " + checkCertificateHash("invalid", hash1, "SHA-256"));
    }
}
