
import java.io.*;
import java.nio.file.*;
import java.security.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import java.util.Base64;

public class Task52 {
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/CBC/PKCS5Padding";
    private static final int KEY_SIZE = 256;
    private static final int IV_SIZE = 16;

    public static SecretKey generateKey(String password) throws Exception {
        MessageDigest sha = MessageDigest.getInstance("SHA-256");
        byte[] key = sha.digest(password.getBytes("UTF-8"));
        return new SecretKeySpec(key, ALGORITHM);
    }

    public static byte[] encryptData(byte[] data, String password) throws Exception {
        SecretKey key = generateKey(password);
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        
        byte[] iv = new byte[IV_SIZE];
        SecureRandom random = new SecureRandom();
        random.nextBytes(iv);
        IvParameterSpec ivSpec = new IvParameterSpec(iv);
        
        cipher.init(Cipher.ENCRYPT_MODE, key, ivSpec);
        byte[] encrypted = cipher.doFinal(data);
        
        byte[] combined = new byte[iv.length + encrypted.length];
        System.arraycopy(iv, 0, combined, 0, iv.length);
        System.arraycopy(encrypted, 0, combined, iv.length, encrypted.length);
        
        return combined;
    }

    public static byte[] decryptData(byte[] encryptedData, String password) throws Exception {
        SecretKey key = generateKey(password);
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        
        byte[] iv = new byte[IV_SIZE];
        System.arraycopy(encryptedData, 0, iv, 0, IV_SIZE);
        IvParameterSpec ivSpec = new IvParameterSpec(iv);
        
        cipher.init(Cipher.DECRYPT_MODE, key, ivSpec);
        
        byte[] encryptedContent = new byte[encryptedData.length - IV_SIZE];
        System.arraycopy(encryptedData, IV_SIZE, encryptedContent, 0, encryptedContent.length);
        
        return cipher.doFinal(encryptedContent);
    }

    public static void encryptFile(String inputFile, String outputFile, String password) throws Exception {
        byte[] fileContent = Files.readAllBytes(Paths.get(inputFile));
        byte[] encrypted = encryptData(fileContent, password);
        Files.write(Paths.get(outputFile), encrypted);
    }

    public static void decryptFile(String inputFile, String outputFile, String password) throws Exception {
        byte[] encryptedContent = Files.readAllBytes(Paths.get(inputFile));
        byte[] decrypted = decryptData(encryptedContent, password);
        Files.write(Paths.get(outputFile), decrypted);
    }

    public static void main(String[] args) {
        try {
            // Test Case 1: Encrypt and decrypt a simple text
            System.out.println("Test Case 1: Simple text encryption/decryption");
            String text1 = "Hello, World!";
            String password1 = "SecurePassword123";
            byte[] encrypted1 = encryptData(text1.getBytes(), password1);
            byte[] decrypted1 = decryptData(encrypted1, password1);
            System.out.println("Original: " + text1);
            System.out.println("Decrypted: " + new String(decrypted1));
            System.out.println("Match: " + text1.equals(new String(decrypted1)) + "\\n");

            // Test Case 2: Different password
            System.out.println("Test Case 2: Longer text with special characters");
            String text2 = "This is a secure message with special chars: @#$%^&*()";
            String password2 = "AnotherSecurePass456";
            byte[] encrypted2 = encryptData(text2.getBytes(), password2);
            byte[] decrypted2 = decryptData(encrypted2, password2);
            System.out.println("Original: " + text2);
            System.out.println("Decrypted: " + new String(decrypted2));
            System.out.println("Match: " + text2.equals(new String(decrypted2)) + "\\n");

            // Test Case 3: Empty string
            System.out.println("Test Case 3: Empty string");
            String text3 = "";
            String password3 = "password";
            byte[] encrypted3 = encryptData(text3.getBytes(), password3);
            byte[] decrypted3 = decryptData(encrypted3, password3);
            System.out.println("Original length: " + text3.length());
            System.out.println("Decrypted length: " + new String(decrypted3).length());
            System.out.println("Match: " + text3.equals(new String(decrypted3)) + "\\n");

            // Test Case 4: Numeric data
            System.out.println("Test Case 4: Numeric data");
            String text4 = "1234567890";
            String password4 = "NumericPass999";
            byte[] encrypted4 = encryptData(text4.getBytes(), password4);
            byte[] decrypted4 = decryptData(encrypted4, password4);
            System.out.println("Original: " + text4);
            System.out.println("Decrypted: " + new String(decrypted4));
            System.out.println("Match: " + text4.equals(new String(decrypted4)) + "\\n");

            // Test Case 5: File encryption/decryption
            System.out.println("Test Case 5: File encryption/decryption");
            String testContent = "This is a test file content for encryption.";
            Files.write(Paths.get("test_input.txt"), testContent.getBytes());
            
            encryptFile("test_input.txt", "test_encrypted.bin", "FilePassword123");
            decryptFile("test_encrypted.bin", "test_decrypted.txt", "FilePassword123");
            
            String decryptedContent = new String(Files.readAllBytes(Paths.get("test_decrypted.txt")));
            System.out.println("Original file content: " + testContent);
            System.out.println("Decrypted file content: " + decryptedContent);
            System.out.println("Match: " + testContent.equals(decryptedContent));
            
            // Cleanup
            Files.deleteIfExists(Paths.get("test_input.txt"));
            Files.deleteIfExists(Paths.get("test_encrypted.bin"));
            Files.deleteIfExists(Paths.get("test_decrypted.txt"));
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
