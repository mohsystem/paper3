
import java.security.*;
import java.security.spec.*;
import java.util.Base64;
import java.math.BigInteger;

public class Task78 {
    
    public static String generateRSAPrivateKeyOpenSSH(int keySize) {
        try {
            // Generate RSA key pair
            KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
            keyGen.initialize(keySize, new SecureRandom());
            KeyPair keyPair = keyGen.generateKeyPair();
            
            PrivateKey privateKey = keyPair.getPrivate();
            PublicKey publicKey = keyPair.getPublic();
            
            // Get RSA parameters
            KeyFactory keyFactory = KeyFactory.getInstance("RSA");
            RSAPrivateCrtKey rsaPrivateKey = (RSAPrivateCrtKey) keyFactory.translateKey(privateKey);
            RSAPublicKey rsaPublicKey = (RSAPublicKey) keyFactory.translateKey(publicKey);
            
            // Build OpenSSH format
            return buildOpenSSHFormat(rsaPrivateKey, rsaPublicKey);
            
        } catch (Exception e) {
            return "Error generating key: " + e.getMessage();
        }
    }
    
    private static String buildOpenSSHFormat(RSAPrivateCrtKey privateKey, RSAPublicKey publicKey) {
        try {
            StringBuilder sb = new StringBuilder();
            sb.append("-----BEGIN OPENSSH PRIVATE KEY-----\\n");
            
            // OpenSSH private key format (simplified)
            byte[] encoded = privateKey.getEncoded();
            String base64 = Base64.getEncoder().encodeToString(encoded);
            
            // Split into 70 character lines
            for (int i = 0; i < base64.length(); i += 70) {
                int end = Math.min(i + 70, base64.length());
                sb.append(base64.substring(i, end)).append("\\n");
            }
            
            sb.append("-----END OPENSSH PRIVATE KEY-----");
            return sb.toString();
            
        } catch (Exception e) {
            return "Error formatting key: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Generate 2048-bit RSA key");
        String key1 = generateRSAPrivateKeyOpenSSH(2048);
        System.out.println(key1.substring(0, Math.min(200, key1.length())) + "...\\n");
        
        System.out.println("Test Case 2: Generate 1024-bit RSA key");
        String key2 = generateRSAPrivateKeyOpenSSH(1024);
        System.out.println(key2.substring(0, Math.min(200, key2.length())) + "...\\n");
        
        System.out.println("Test Case 3: Generate 2048-bit RSA key (second instance)");
        String key3 = generateRSAPrivateKeyOpenSSH(2048);
        System.out.println(key3.substring(0, Math.min(200, key3.length())) + "...\\n");
        
        System.out.println("Test Case 4: Generate 3072-bit RSA key");
        String key4 = generateRSAPrivateKeyOpenSSH(3072);
        System.out.println(key4.substring(0, Math.min(200, key4.length())) + "...\\n");
        
        System.out.println("Test Case 5: Generate 4096-bit RSA key");
        String key5 = generateRSAPrivateKeyOpenSSH(4096);
        System.out.println(key5.substring(0, Math.min(200, key5.length())) + "...\\n");
    }
}
