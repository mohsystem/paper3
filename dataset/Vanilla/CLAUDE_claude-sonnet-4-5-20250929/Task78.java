
import java.math.BigInteger;
import java.security.*;
import java.security.spec.*;
import java.util.Base64;
import java.io.ByteArrayOutputStream;

public class Task78 {
    
    public static String generateRSAPrivateKeyOpenSSH(int keySize) {
        try {
            KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
            keyGen.initialize(keySize);
            KeyPair keyPair = keyGen.generateKeyPair();
            
            PrivateKey privateKey = keyPair.getPrivate();
            RSAPrivateCrtKey rsaPrivateKey = (RSAPrivateCrtKey) privateKey;
            
            return encodeOpenSSHPrivateKey(rsaPrivateKey);
        } catch (Exception e) {
            return "Error generating key: " + e.getMessage();
        }
    }
    
    private static String encodeOpenSSHPrivateKey(RSAPrivateCrtKey key) {
        try {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            
            writeString(out, "ssh-rsa");
            writeBigInteger(out, key.getModulus());
            writeBigInteger(out, key.getPublicExponent());
            writeBigInteger(out, key.getPrivateExponent());
            writeBigInteger(out, key.getCrtCoefficient());
            writeBigInteger(out, key.getPrimeP());
            writeBigInteger(out, key.getPrimeQ());
            
            byte[] encoded = out.toByteArray();
            String base64 = Base64.getEncoder().encodeToString(encoded);
            
            StringBuilder result = new StringBuilder();
            result.append("-----BEGIN OPENSSH PRIVATE KEY-----\\n");
            
            int lineLength = 70;
            for (int i = 0; i < base64.length(); i += lineLength) {
                int end = Math.min(i + lineLength, base64.length());
                result.append(base64.substring(i, end)).append("\\n");
            }
            
            result.append("-----END OPENSSH PRIVATE KEY-----");
            
            return result.toString();
        } catch (Exception e) {
            return "Error encoding key: " + e.getMessage();
        }
    }
    
    private static void writeString(ByteArrayOutputStream out, String str) throws Exception {
        byte[] bytes = str.getBytes("UTF-8");
        writeInt(out, bytes.length);
        out.write(bytes);
    }
    
    private static void writeBigInteger(ByteArrayOutputStream out, BigInteger bi) throws Exception {
        byte[] bytes = bi.toByteArray();
        writeInt(out, bytes.length);
        out.write(bytes);
    }
    
    private static void writeInt(ByteArrayOutputStream out, int value) {
        out.write((value >>> 24) & 0xFF);
        out.write((value >>> 16) & 0xFF);
        out.write((value >>> 8) & 0xFF);
        out.write(value & 0xFF);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: 512-bit key");
        String key1 = generateRSAPrivateKeyOpenSSH(512);
        System.out.println(key1.substring(0, Math.min(200, key1.length())) + "...\\n");
        
        System.out.println("Test Case 2: 1024-bit key");
        String key2 = generateRSAPrivateKeyOpenSSH(1024);
        System.out.println(key2.substring(0, Math.min(200, key2.length())) + "...\\n");
        
        System.out.println("Test Case 3: 2048-bit key");
        String key3 = generateRSAPrivateKeyOpenSSH(2048);
        System.out.println(key3.substring(0, Math.min(200, key3.length())) + "...\\n");
        
        System.out.println("Test Case 4: Another 1024-bit key");
        String key4 = generateRSAPrivateKeyOpenSSH(1024);
        System.out.println(key4.substring(0, Math.min(200, key4.length())) + "...\\n");
        
        System.out.println("Test Case 5: 512-bit key");
        String key5 = generateRSAPrivateKeyOpenSSH(512);
        System.out.println(key5.substring(0, Math.min(200, key5.length())) + "...\\n");
    }
}
