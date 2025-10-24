
import java.io.*;
import java.util.*;

public class Task52 {
    private static final int KEY = 123;
    
    // XOR-based encryption/decryption
    public static byte[] encryptDecrypt(byte[] data, int key) {
        byte[] result = new byte[data.length];
        for (int i = 0; i < data.length; i++) {
            result[i] = (byte) (data[i] ^ key);
        }
        return result;
    }
    
    public static void encryptFile(String inputFile, String outputFile, int key) throws IOException {
        FileInputStream fis = new FileInputStream(inputFile);
        byte[] data = fis.readAllBytes();
        fis.close();
        
        byte[] encrypted = encryptDecrypt(data, key);
        
        FileOutputStream fos = new FileOutputStream(outputFile);
        fos.write(encrypted);
        fos.close();
    }
    
    public static void decryptFile(String inputFile, String outputFile, int key) throws IOException {
        FileInputStream fis = new FileInputStream(inputFile);
        byte[] data = fis.readAllBytes();
        fis.close();
        
        byte[] decrypted = encryptDecrypt(data, key);
        
        FileOutputStream fos = new FileOutputStream(outputFile);
        fos.write(decrypted);
        fos.close();
    }
    
    public static String encryptString(String plaintext, int key) {
        byte[] encrypted = encryptDecrypt(plaintext.getBytes(), key);
        return Base64.getEncoder().encodeToString(encrypted);
    }
    
    public static String decryptString(String encrypted, int key) {
        byte[] encryptedBytes = Base64.getDecoder().decode(encrypted);
        byte[] decrypted = encryptDecrypt(encryptedBytes, key);
        return new String(decrypted);
    }
    
    public static void main(String[] args) {
        System.out.println("File Encryption/Decryption Program");
        System.out.println("===================================\\n");
        
        // Test Case 1: Encrypt and decrypt a simple string
        System.out.println("Test Case 1: Simple String Encryption");
        String text1 = "Hello World!";
        String encrypted1 = encryptString(text1, KEY);
        String decrypted1 = decryptString(encrypted1, KEY);
        System.out.println("Original: " + text1);
        System.out.println("Encrypted: " + encrypted1);
        System.out.println("Decrypted: " + decrypted1);
        System.out.println("Match: " + text1.equals(decrypted1) + "\\n");
        
        // Test Case 2: Encrypt and decrypt a longer text
        System.out.println("Test Case 2: Longer Text Encryption");
        String text2 = "This is a test message with numbers 12345 and symbols @#$%";
        String encrypted2 = encryptString(text2, KEY);
        String decrypted2 = decryptString(encrypted2, KEY);
        System.out.println("Original: " + text2);
        System.out.println("Encrypted: " + encrypted2);
        System.out.println("Decrypted: " + decrypted2);
        System.out.println("Match: " + text2.equals(decrypted2) + "\\n");
        
        // Test Case 3: Encrypt and decrypt with different key
        System.out.println("Test Case 3: Different Key");
        String text3 = "Secret Message";
        int customKey = 255;
        String encrypted3 = encryptString(text3, customKey);
        String decrypted3 = decryptString(encrypted3, customKey);
        System.out.println("Original: " + text3);
        System.out.println("Encrypted: " + encrypted3);
        System.out.println("Decrypted: " + decrypted3);
        System.out.println("Match: " + text3.equals(decrypted3) + "\\n");
        
        // Test Case 4: Empty string
        System.out.println("Test Case 4: Empty String");
        String text4 = "";
        String encrypted4 = encryptString(text4, KEY);
        String decrypted4 = decryptString(encrypted4, KEY);
        System.out.println("Original: '" + text4 + "'");
        System.out.println("Encrypted: '" + encrypted4 + "'");
        System.out.println("Decrypted: '" + decrypted4 + "'");
        System.out.println("Match: " + text4.equals(decrypted4) + "\\n");
        
        // Test Case 5: File encryption/decryption
        System.out.println("Test Case 5: File Encryption/Decryption");
        try {
            String originalFile = "test_input.txt";
            String encryptedFile = "test_encrypted.bin";
            String decryptedFile = "test_decrypted.txt";
            
            // Create a test file
            FileWriter fw = new FileWriter(originalFile);
            fw.write("This is a test file content.\\nLine 2 with more data.");
            fw.close();
            
            // Encrypt file
            encryptFile(originalFile, encryptedFile, KEY);
            System.out.println("File encrypted: " + encryptedFile);
            
            // Decrypt file
            decryptFile(encryptedFile, decryptedFile, KEY);
            System.out.println("File decrypted: " + decryptedFile);
            
            // Verify content
            FileReader fr = new FileReader(decryptedFile);
            BufferedReader br = new BufferedReader(fr);
            StringBuilder content = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) {
                content.append(line).append("\\n");
            }
            br.close();
            System.out.println("Decrypted content: " + content.toString().trim());
            
        } catch (IOException e) {
            System.out.println("File operation: " + e.getMessage());
        }
    }
}
