
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.*;
import java.security.SecureRandom;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Base64;

public class Task52 {
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/GCM/NoPadding";
    private static final int KEY_SIZE = 256;
    private static final int GCM_IV_LENGTH = 12;
    private static final int GCM_TAG_LENGTH = 128;

    public static byte[] generateKey() throws Exception {
        KeyGenerator keyGenerator = KeyGenerator.getInstance(ALGORITHM);
        keyGenerator.init(KEY_SIZE, new SecureRandom());
        SecretKey secretKey = keyGenerator.generateKey();
        return secretKey.getEncoded();
    }

    public static byte[] encryptFile(byte[] fileContent, byte[] key) throws Exception {
        if (fileContent == null || key == null) {
            throw new IllegalArgumentException("File content and key cannot be null");
        }
        
        SecretKeySpec secretKeySpec = new SecretKeySpec(key, ALGORITHM);
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        
        byte[] iv = new byte[GCM_IV_LENGTH];
        SecureRandom secureRandom = new SecureRandom();
        secureRandom.nextBytes(iv);
        
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, gcmParameterSpec);
        
        byte[] encryptedData = cipher.doFinal(fileContent);
        
        byte[] encryptedWithIv = new byte[iv.length + encryptedData.length];
        System.arraycopy(iv, 0, encryptedWithIv, 0, iv.length);
        System.arraycopy(encryptedData, 0, encryptedWithIv, iv.length, encryptedData.length);
        
        return encryptedWithIv;
    }

    public static byte[] decryptFile(byte[] encryptedContent, byte[] key) throws Exception {
        if (encryptedContent == null || key == null) {
            throw new IllegalArgumentException("Encrypted content and key cannot be null");
        }
        
        if (encryptedContent.length < GCM_IV_LENGTH) {
            throw new IllegalArgumentException("Invalid encrypted content");
        }
        
        byte[] iv = new byte[GCM_IV_LENGTH];
        System.arraycopy(encryptedContent, 0, iv, 0, GCM_IV_LENGTH);
        
        byte[] encryptedData = new byte[encryptedContent.length - GCM_IV_LENGTH];
        System.arraycopy(encryptedContent, GCM_IV_LENGTH, encryptedData, 0, encryptedData.length);
        
        SecretKeySpec secretKeySpec = new SecretKeySpec(key, ALGORITHM);
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.DECRYPT_MODE, secretKeySpec, gcmParameterSpec);
        
        return cipher.doFinal(encryptedData);
    }

    public static void main(String[] args) {
        try {
            System.out.println("=== Secure File Encryption/Decryption Test Cases ===\\n");

            // Test Case 1: Simple text encryption/decryption
            System.out.println("Test Case 1: Simple text encryption");
            byte[] key1 = generateKey();
            String text1 = "Hello, World!";
            byte[] encrypted1 = encryptFile(text1.getBytes("UTF-8"), key1);
            byte[] decrypted1 = decryptFile(encrypted1, key1);
            System.out.println("Original: " + text1);
            System.out.println("Decrypted: " + new String(decrypted1, "UTF-8"));
            System.out.println("Match: " + text1.equals(new String(decrypted1, "UTF-8")) + "\\n");

            // Test Case 2: Empty content
            System.out.println("Test Case 2: Empty content");
            byte[] key2 = generateKey();
            String text2 = "";
            byte[] encrypted2 = encryptFile(text2.getBytes("UTF-8"), key2);
            byte[] decrypted2 = decryptFile(encrypted2, key2);
            System.out.println("Original length: " + text2.length());
            System.out.println("Decrypted length: " + decrypted2.length);
            System.out.println("Match: " + (text2.length() == decrypted2.length) + "\\n");

            // Test Case 3: Large text
            System.out.println("Test Case 3: Large text encryption");
            byte[] key3 = generateKey();
            StringBuilder largeText = new StringBuilder();
            for (int i = 0; i < 1000; i++) {
                largeText.append("Secure encryption test ").append(i).append(" ");
            }
            String text3 = largeText.toString();
            byte[] encrypted3 = encryptFile(text3.getBytes("UTF-8"), key3);
            byte[] decrypted3 = decryptFile(encrypted3, key3);
            System.out.println("Original length: " + text3.length());
            System.out.println("Decrypted length: " + new String(decrypted3, "UTF-8").length());
            System.out.println("Match: " + text3.equals(new String(decrypted3, "UTF-8")) + "\\n");

            // Test Case 4: Special characters
            System.out.println("Test Case 4: Special characters");
            byte[] key4 = generateKey();
            String text4 = "Special chars: !@#$%^&*()_+-=[]{}|;':\\",./<>?";
            byte[] encrypted4 = encryptFile(text4.getBytes("UTF-8"), key4);
            byte[] decrypted4 = decryptFile(encrypted4, key4);
            System.out.println("Original: " + text4);
            System.out.println("Decrypted: " + new String(decrypted4, "UTF-8"));
            System.out.println("Match: " + text4.equals(new String(decrypted4, "UTF-8")) + "\\n");

            // Test Case 5: Unicode characters
            System.out.println("Test Case 5: Unicode characters");
            byte[] key5 = generateKey();
            String text5 = "Unicode: こんにちは 世界 🌍🔒";
            byte[] encrypted5 = encryptFile(text5.getBytes("UTF-8"), key5);
            byte[] decrypted5 = decryptFile(encrypted5, key5);
            System.out.println("Original: " + text5);
            System.out.println("Decrypted: " + new String(decrypted5, "UTF-8"));
            System.out.println("Match: " + text5.equals(new String(decrypted5, "UTF-8")) + "\\n");

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
