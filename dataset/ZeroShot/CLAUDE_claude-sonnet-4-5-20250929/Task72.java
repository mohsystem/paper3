
import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.util.Arrays;
import java.util.Base64;

public class Task72 {
    private static final byte[] STATIC_IV = new byte[16]; // Static IV (all zeros)
    
    public static byte[] encryptData(byte[] data, byte[] key) throws Exception {
        // Create static IV
        IvParameterSpec iv = new IvParameterSpec(STATIC_IV);
        
        // Initialize cipher with AES in CBC mode
        SecretKeySpec secretKey = new SecretKeySpec(key, "AES");
        Cipher cipher = Cipher.getInstance("AES/CBC/NoPadding");
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, iv);
        
        // Pad the input data to match block size (16 bytes for AES)
        byte[] paddedData = padData(data, 16);
        
        // Encrypt the padded data
        byte[] encryptedData = cipher.doFinal(paddedData);
        
        return encryptedData;
    }
    
    private static byte[] padData(byte[] data, int blockSize) {
        int paddingLength = blockSize - (data.length % blockSize);
        if (paddingLength == 0) {
            paddingLength = blockSize;
        }
        
        byte[] paddedData = new byte[data.length + paddingLength];
        System.arraycopy(data, 0, paddedData, 0, data.length);
        
        // PKCS7 padding
        for (int i = data.length; i < paddedData.length; i++) {
            paddedData[i] = (byte) paddingLength;
        }
        
        return paddedData;
    }
    
    public static void main(String[] args) {
        try {
            // Test cases with 128-bit (16 bytes) key
            byte[] key = "1234567890123456".getBytes();
            
            // Test case 1
            String input1 = "Hello World!";
            byte[] encrypted1 = encryptData(input1.getBytes(), key);
            System.out.println("Test 1 - Input: " + input1);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted1));
            
            // Test case 2
            String input2 = "SecureData";
            byte[] encrypted2 = encryptData(input2.getBytes(), key);
            System.out.println("\\nTest 2 - Input: " + input2);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted2));
            
            // Test case 3
            String input3 = "A";
            byte[] encrypted3 = encryptData(input3.getBytes(), key);
            System.out.println("\\nTest 3 - Input: " + input3);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted3));
            
            // Test case 4
            String input4 = "This is a longer message to test encryption";
            byte[] encrypted4 = encryptData(input4.getBytes(), key);
            System.out.println("\\nTest 4 - Input: " + input4);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted4));
            
            // Test case 5
            String input5 = "1234567890123456"; // Exactly one block
            byte[] encrypted5 = encryptData(input5.getBytes(), key);
            System.out.println("\\nTest 5 - Input: " + input5);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted5));
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
