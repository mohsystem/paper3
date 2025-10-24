
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import java.security.*;

public class Task131 {
    private static final String MAGIC = "ENC1";
    private static final int VERSION = 1;
    private static final int SALT_SIZE = 16;
    private static final int IV_SIZE = 12;
    private static final int TAG_SIZE = 16;
    private static final int MAX_DATA_SIZE = 1024 * 1024; // 1MB limit
    
    public static class SecureData implements Serializable {
        private static final long serialVersionUID = 1L;
        private final String data;
        private final long timestamp;
        
        public SecureData(String data) {
            if (data == null || data.length() > 10000) {
                throw new IllegalArgumentException("Invalid data");
            }
            this.data = data;
            this.timestamp = System.currentTimeMillis();
        }
        
        public String getData() { return data; }
        public long getTimestamp() { return timestamp; }
        
        @Override
        public String toString() {
            return "SecureData{data='" + data + "', timestamp=" + timestamp + "}";
        }
    }
    
    public static byte[] serializeData(SecureData data, String passphrase) throws Exception {
        if (data == null || passphrase == null || passphrase.length() < 8) {
            throw new IllegalArgumentException("Invalid input");
        }
        
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        baos.write(MAGIC.getBytes(StandardCharsets.UTF_8));
        baos.write(VERSION);
        
        byte[] salt = new byte[SALT_SIZE];
        SecureRandom random = new SecureRandom();
        random.nextBytes(salt);
        baos.write(salt);
        
        SecretKeySpec key = deriveKey(passphrase, salt);
        byte[] iv = new byte[IV_SIZE];
        random.nextBytes(iv);
        baos.write(iv);
        
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_SIZE * 8, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, spec);
        
        ByteArrayOutputStream dataStream = new ByteArrayOutputStream();
        try (ObjectOutputStream oos = new ObjectOutputStream(dataStream)) {
            oos.writeObject(data);
        }
        byte[] plaintext = dataStream.toByteArray();
        
        byte[] ciphertext = cipher.doFinal(plaintext);
        baos.write(ciphertext);
        
        return baos.toByteArray();
    }
    
    public static SecureData deserializeData(byte[] encryptedData, String passphrase) throws Exception {
        if (encryptedData == null || passphrase == null || passphrase.length() < 8) {
            throw new IllegalArgumentException("Invalid input");
        }
        
        if (encryptedData.length > MAX_DATA_SIZE) {
            throw new IllegalArgumentException("Data too large");
        }
        
        ByteArrayInputStream bais = new ByteArrayInputStream(encryptedData);
        
        byte[] magic = new byte[4];
        if (bais.read(magic) != 4 || !Arrays.equals(magic, MAGIC.getBytes(StandardCharsets.UTF_8))) {
            throw new SecurityException("Invalid magic");
        }
        
        int version = bais.read();
        if (version != VERSION) {
            throw new SecurityException("Invalid version");
        }
        
        byte[] salt = new byte[SALT_SIZE];
        if (bais.read(salt) != SALT_SIZE) {
            throw new SecurityException("Invalid salt");
        }
        
        byte[] iv = new byte[IV_SIZE];
        if (bais.read(iv) != IV_SIZE) {
            throw new SecurityException("Invalid IV");
        }
        
        byte[] ciphertext = bais.readAllBytes();
        if (ciphertext.length < TAG_SIZE) {
            throw new SecurityException("Invalid ciphertext");
        }
        
        SecretKeySpec key = deriveKey(passphrase, salt);
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_SIZE * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, spec);
        
        byte[] plaintext = cipher.doFinal(ciphertext);
        
        try (ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(plaintext)) {
            @Override
            protected Class<?> resolveClass(ObjectStreamClass desc) throws IOException, ClassNotFoundException {
                if (!desc.getName().equals(SecureData.class.getName())) {
                    throw new SecurityException("Unauthorized deserialization attempt");
                }
                return super.resolveClass(desc);
            }
        }) {
            Object obj = ois.readObject();
            if (!(obj instanceof SecureData)) {
                throw new SecurityException("Invalid object type");
            }
            return (SecureData) obj;
        }
    }
    
    private static SecretKeySpec deriveKey(String passphrase, byte[] salt) throws Exception {
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        KeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, 100000, 256);
        SecretKey tmp = factory.generateSecret(spec);
        return new SecretKeySpec(tmp.getEncoded(), "AES");
    }
    
    public static void main(String[] args) {
        try {
            System.out.println("Test Case 1: Basic serialization/deserialization");
            SecureData data1 = new SecureData("Hello World");
            byte[] encrypted1 = serializeData(data1, "strongpassphrase123");
            SecureData decrypted1 = deserializeData(encrypted1, "strongpassphrase123");
            System.out.println("Original: " + data1);
            System.out.println("Decrypted: " + decrypted1);
            System.out.println("Match: " + data1.getData().equals(decrypted1.getData()));
            System.out.println();
            
            System.out.println("Test Case 2: Different data");
            SecureData data2 = new SecureData("Sensitive information 12345");
            byte[] encrypted2 = serializeData(data2, "anotherpassword456");
            SecureData decrypted2 = deserializeData(encrypted2, "anotherpassword456");
            System.out.println("Original: " + data2);
            System.out.println("Decrypted: " + decrypted2);
            System.out.println("Match: " + data2.getData().equals(decrypted2.getData()));
            System.out.println();
            
            System.out.println("Test Case 3: Wrong passphrase");
            try {
                SecureData data3 = new SecureData("Test data");
                byte[] encrypted3 = serializeData(data3, "correctpass123");
                deserializeData(encrypted3, "wrongpass123");
                System.out.println("ERROR: Should have thrown exception");
            } catch (Exception e) {
                System.out.println("Correctly rejected wrong passphrase: " + e.getClass().getSimpleName());
            }
            System.out.println();
            
            System.out.println("Test Case 4: Corrupted data");
            try {
                SecureData data4 = new SecureData("Test data");
                byte[] encrypted4 = serializeData(data4, "mypassword789");
                encrypted4[encrypted4.length - 1] ^= 0xFF;
                deserializeData(encrypted4, "mypassword789");
                System.out.println("ERROR: Should have thrown exception");
            } catch (Exception e) {
                System.out.println("Correctly rejected corrupted data: " + e.getClass().getSimpleName());
            }
            System.out.println();
            
            System.out.println("Test Case 5: Empty string");
            SecureData data5 = new SecureData("");
            byte[] encrypted5 = serializeData(data5, "emptytest123");
            SecureData decrypted5 = deserializeData(encrypted5, "emptytest123");
            System.out.println("Original: " + data5);
            System.out.println("Decrypted: " + decrypted5);
            System.out.println("Match: " + data5.getData().equals(decrypted5.getData()));
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
