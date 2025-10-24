
import java.math.BigInteger;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task106 {
    static class RSA {
        private BigInteger n, d, e;
        
        public RSA(int bitLength) {
            SecureRandom random = new SecureRandom();
            BigInteger p = BigInteger.probablePrime(bitLength / 2, random);
            BigInteger q = BigInteger.probablePrime(bitLength / 2, random);
            
            n = p.multiply(q);
            BigInteger phi = p.subtract(BigInteger.ONE).multiply(q.subtract(BigInteger.ONE));
            
            e = BigInteger.valueOf(65537);
            while (phi.gcd(e).intValue() > 1) {
                e = e.add(BigInteger.TWO);
            }
            
            d = e.modInverse(phi);
        }
        
        public BigInteger encrypt(BigInteger message) {
            return message.modPow(e, n);
        }
        
        public BigInteger decrypt(BigInteger encrypted) {
            return encrypted.modPow(d, n);
        }
        
        public String encryptString(String message) {
            BigInteger msg = new BigInteger(message.getBytes());
            BigInteger encrypted = encrypt(msg);
            return encrypted.toString();
        }
        
        public String decryptString(String encryptedMessage) {
            BigInteger encrypted = new BigInteger(encryptedMessage);
            BigInteger decrypted = decrypt(encrypted);
            return new String(decrypted.toByteArray());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("RSA Encryption/Decryption Test Cases:");
        System.out.println("=====================================\\n");
        
        // Test Case 1
        System.out.println("Test Case 1: Simple message");
        RSA rsa1 = new RSA(512);
        String message1 = "Hello";
        String encrypted1 = rsa1.encryptString(message1);
        String decrypted1 = rsa1.decryptString(encrypted1);
        System.out.println("Original: " + message1);
        System.out.println("Encrypted: " + encrypted1);
        System.out.println("Decrypted: " + decrypted1);
        System.out.println("Match: " + message1.equals(decrypted1) + "\\n");
        
        // Test Case 2
        System.out.println("Test Case 2: Longer message");
        RSA rsa2 = new RSA(512);
        String message2 = "RSA Algorithm";
        String encrypted2 = rsa2.encryptString(message2);
        String decrypted2 = rsa2.decryptString(encrypted2);
        System.out.println("Original: " + message2);
        System.out.println("Encrypted: " + encrypted2);
        System.out.println("Decrypted: " + decrypted2);
        System.out.println("Match: " + message2.equals(decrypted2) + "\\n");
        
        // Test Case 3
        System.out.println("Test Case 3: Numbers");
        RSA rsa3 = new RSA(512);
        String message3 = "12345";
        String encrypted3 = rsa3.encryptString(message3);
        String decrypted3 = rsa3.decryptString(encrypted3);
        System.out.println("Original: " + message3);
        System.out.println("Encrypted: " + encrypted3);
        System.out.println("Decrypted: " + decrypted3);
        System.out.println("Match: " + message3.equals(decrypted3) + "\\n");
        
        // Test Case 4
        System.out.println("Test Case 4: Special characters");
        RSA rsa4 = new RSA(512);
        String message4 = "Test@123!";
        String encrypted4 = rsa4.encryptString(message4);
        String decrypted4 = rsa4.decryptString(encrypted4);
        System.out.println("Original: " + message4);
        System.out.println("Encrypted: " + encrypted4);
        System.out.println("Decrypted: " + decrypted4);
        System.out.println("Match: " + message4.equals(decrypted4) + "\\n");
        
        // Test Case 5
        System.out.println("Test Case 5: Single character");
        RSA rsa5 = new RSA(512);
        String message5 = "A";
        String encrypted5 = rsa5.encryptString(message5);
        String decrypted5 = rsa5.decryptString(encrypted5);
        System.out.println("Original: " + message5);
        System.out.println("Encrypted: " + encrypted5);
        System.out.println("Decrypted: " + decrypted5);
        System.out.println("Match: " + message5.equals(decrypted5) + "\\n");
    }
}
