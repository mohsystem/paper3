
import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.util.Arrays;
import java.util.Base64;

public class Task72 {
    private static final byte[] STATIC_IV = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    private static final int BLOCK_SIZE = 16;
    
    public static byte[] encryptData(byte[] data, byte[] key) throws Exception {
        byte[] paddedData = padData(data, BLOCK_SIZE);
        
        SecretKeySpec secretKey = new SecretKeySpec(key, "AES");
        IvParameterSpec ivSpec = new IvParameterSpec(STATIC_IV);
        
        Cipher cipher = Cipher.getInstance("AES/CBC/NoPadding");
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, ivSpec);
        
        return cipher.doFinal(paddedData);
    }
    
    private static byte[] padData(byte[] data, int blockSize) {
        int paddingLength = blockSize - (data.length % blockSize);
        if (paddingLength == 0) {
            paddingLength = blockSize;
        }
        
        byte[] paddedData = new byte[data.length + paddingLength];
        System.arraycopy(data, 0, paddedData, 0, data.length);
        
        for (int i = data.length; i < paddedData.length; i++) {
            paddedData[i] = (byte) paddingLength;
        }
        
        return paddedData;
    }
    
    public static void main(String[] args) {
        try {
            byte[] key = "0123456789ABCDEF".getBytes();
            
            System.out.println("Test Case 1:");
            byte[] data1 = "Hello World!".getBytes();
            byte[] encrypted1 = encryptData(data1, key);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted1));
            
            System.out.println("\\nTest Case 2:");
            byte[] data2 = "Test".getBytes();
            byte[] encrypted2 = encryptData(data2, key);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted2));
            
            System.out.println("\\nTest Case 3:");
            byte[] data3 = "A".getBytes();
            byte[] encrypted3 = encryptData(data3, key);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted3));
            
            System.out.println("\\nTest Case 4:");
            byte[] data4 = "This is a longer message for encryption testing".getBytes();
            byte[] encrypted4 = encryptData(data4, key);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted4));
            
            System.out.println("\\nTest Case 5:");
            byte[] data5 = "1234567890123456".getBytes();
            byte[] encrypted5 = encryptData(data5, key);
            System.out.println("Encrypted: " + Base64.getEncoder().encodeToString(encrypted5));
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
