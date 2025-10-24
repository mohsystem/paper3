
import java.math.BigInteger;
import java.security.*;
import javax.crypto.Cipher;
import java.util.Base64;

public class Task106 {
    private PublicKey publicKey;
    private PrivateKey privateKey;

    public Task106() throws NoSuchAlgorithmException {
        generateKeyPair();
    }

    private void generateKeyPair() throws NoSuchAlgorithmException {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        SecureRandom random = new SecureRandom();
        keyGen.initialize(2048, random);
        KeyPair pair = keyGen.generateKeyPair();
        this.publicKey = pair.getPublic();
        this.privateKey = pair.getPrivate();
    }

    public String encrypt(String plainText) throws Exception {
        if (plainText == null || plainText.isEmpty()) {
            throw new IllegalArgumentException("Input cannot be null or empty");
        }
        Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPWITHSHA-256ANDMGF1PADDING");
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        byte[] encryptedBytes = cipher.doFinal(plainText.getBytes("UTF-8"));
        return Base64.getEncoder().encodeToString(encryptedBytes);
    }

    public String decrypt(String encryptedText) throws Exception {
        if (encryptedText == null || encryptedText.isEmpty()) {
            throw new IllegalArgumentException("Input cannot be null or empty");
        }
        Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPWITHSHA-256ANDMGF1PADDING");
        cipher.init(Cipher.DECRYPT_MODE, privateKey);
        byte[] decryptedBytes = cipher.doFinal(Base64.getDecoder().decode(encryptedText));
        return new String(decryptedBytes, "UTF-8");
    }

    public static void main(String[] args) {
        try {
            // Test Case 1: Simple text encryption
            Task106 rsa1 = new Task106();
            String text1 = "Hello World";
            String encrypted1 = rsa1.encrypt(text1);
            String decrypted1 = rsa1.decrypt(encrypted1);
            System.out.println("Test 1:");
            System.out.println("Original: " + text1);
            System.out.println("Decrypted: " + decrypted1);
            System.out.println("Match: " + text1.equals(decrypted1));
            System.out.println();

            // Test Case 2: Numbers and special characters
            Task106 rsa2 = new Task106();
            String text2 = "12345!@#$%";
            String encrypted2 = rsa2.encrypt(text2);
            String decrypted2 = rsa2.decrypt(encrypted2);
            System.out.println("Test 2:");
            System.out.println("Original: " + text2);
            System.out.println("Decrypted: " + decrypted2);
            System.out.println("Match: " + text2.equals(decrypted2));
            System.out.println();

            // Test Case 3: Longer text
            Task106 rsa3 = new Task106();
            String text3 = "RSA Algorithm Test with longer message";
            String encrypted3 = rsa3.encrypt(text3);
            String decrypted3 = rsa3.decrypt(encrypted3);
            System.out.println("Test 3:");
            System.out.println("Original: " + text3);
            System.out.println("Decrypted: " + decrypted3);
            System.out.println("Match: " + text3.equals(decrypted3));
            System.out.println();

            // Test Case 4: Unicode characters
            Task106 rsa4 = new Task106();
            String text4 = "こんにちは世界";
            String encrypted4 = rsa4.encrypt(text4);
            String decrypted4 = rsa4.decrypt(encrypted4);
            System.out.println("Test 4:");
            System.out.println("Original: " + text4);
            System.out.println("Decrypted: " + decrypted4);
            System.out.println("Match: " + text4.equals(decrypted4));
            System.out.println();

            // Test Case 5: Empty-like string (single character)
            Task106 rsa5 = new Task106();
            String text5 = "X";
            String encrypted5 = rsa5.encrypt(text5);
            String decrypted5 = rsa5.decrypt(encrypted5);
            System.out.println("Test 5:");
            System.out.println("Original: " + text5);
            System.out.println("Decrypted: " + decrypted5);
            System.out.println("Match: " + text5.equals(decrypted5));

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
