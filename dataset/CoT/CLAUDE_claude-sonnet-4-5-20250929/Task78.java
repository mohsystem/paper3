
import java.security.*;
import java.security.interfaces.*;
import java.security.spec.*;
import java.util.Base64;
import java.io.*;

public class Task78 {
    
    public static String generateRSAPrivateKeyOpenSSH(int keySize) {
        try {
            // Validate minimum key size for security
            if (keySize < 2048) {
                throw new IllegalArgumentException("Key size must be at least 2048 bits for security");
            }
            
            // Generate RSA key pair using secure random
            KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
            SecureRandom secureRandom = new SecureRandom();
            keyGen.initialize(keySize, secureRandom);
            KeyPair keyPair = keyGen.generateKeyPair();
            
            // Get private key
            RSAPrivateKey privateKey = (RSAPrivateKey) keyPair.getPrivate();
            RSAPublicKey publicKey = (RSAPublicKey) keyPair.getPublic();
            
            // Convert to OpenSSH format (PKCS#8 wrapped in OpenSSH format)
            return convertToOpenSSHFormat(privateKey, publicKey);
            
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("RSA algorithm not available: " + e.getMessage());
        } catch (Exception e) {
            throw new RuntimeException("Error generating RSA key: " + e.getMessage());
        }
    }
    
    private static String convertToOpenSSHFormat(RSAPrivateKey privateKey, RSAPublicKey publicKey) {
        try {
            // Get key parameters
            byte[] encoded = privateKey.getEncoded();
            
            // Create OpenSSH format
            StringBuilder sb = new StringBuilder();
            sb.append("-----BEGIN OPENSSH PRIVATE KEY-----\\n");
            
            // Base64 encode the key
            String base64Key = Base64.getEncoder().encodeToString(encoded);
            
            // Split into 70-character lines (OpenSSH standard)
            int index = 0;
            while (index < base64Key.length()) {
                int end = Math.min(index + 70, base64Key.length());
                sb.append(base64Key.substring(index, end)).append("\\n");
                index = end;
            }
            
            sb.append("-----END OPENSSH PRIVATE KEY-----\\n");
            
            return sb.toString();
            
        } catch (Exception e) {
            throw new RuntimeException("Error converting to OpenSSH format: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== RSA Private Key Generator (OpenSSH Format) ===\\n");
        
        // Test case 1: 2048-bit key (minimum secure)
        System.out.println("Test Case 1: Generate 2048-bit RSA key");
        try {
            String key1 = generateRSAPrivateKeyOpenSSH(2048);
            System.out.println("Success - Key length: " + key1.length() + " characters");
            System.out.println(key1.substring(0, Math.min(200, key1.length())) + "...\\n");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage() + "\\n");
        }
        
        // Test case 2: 4096-bit key (highly secure)
        System.out.println("Test Case 2: Generate 4096-bit RSA key");
        try {
            String key2 = generateRSAPrivateKeyOpenSSH(4096);
            System.out.println("Success - Key length: " + key2.length() + " characters");
            System.out.println(key2.substring(0, Math.min(200, key2.length())) + "...\\n");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage() + "\\n");
        }
        
        // Test case 3: Invalid key size (should fail)
        System.out.println("Test Case 3: Attempt 1024-bit key (insecure, should fail)");
        try {
            String key3 = generateRSAPrivateKeyOpenSSH(1024);
            System.out.println("Generated: " + key3.substring(0, 100) + "...\\n");
        } catch (Exception e) {
            System.out.println("Expected Error: " + e.getMessage() + "\\n");
        }
        
        // Test case 4: 3072-bit key
        System.out.println("Test Case 4: Generate 3072-bit RSA key");
        try {
            String key4 = generateRSAPrivateKeyOpenSSH(3072);
            System.out.println("Success - Key length: " + key4.length() + " characters");
            System.out.println(key4.substring(0, Math.min(200, key4.length())) + "...\\n");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage() + "\\n");
        }
        
        // Test case 5: Standard 2048-bit with full output
        System.out.println("Test Case 5: Generate 2048-bit key with full display");
        try {
            String key5 = generateRSAPrivateKeyOpenSSH(2048);
            System.out.println("Full key generated:\\n" + key5);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage() + "\\n");
        }
    }
}
