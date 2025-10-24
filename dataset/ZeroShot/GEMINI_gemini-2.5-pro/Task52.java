import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;

public class Task52 {

    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/GCM/NoPadding";
    private static final int KEY_SIZE = 256;
    private static final int SALT_SIZE = 16; // bytes
    private static final int IV_SIZE = 12; // bytes for GCM
    private static final int TAG_BIT_LENGTH = 128;
    private static final int PBKDF2_ITERATIONS = 65536;
    private static final String PBKDF2_ALGORITHM = "PBKDF2WithHmacSHA256";

    private static SecretKey getKeyFromPassword(String password, byte[] salt) throws Exception {
        SecretKeyFactory factory = SecretKeyFactory.getInstance(PBKDF2_ALGORITHM);
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, KEY_SIZE);
        SecretKey secret = new SecretKeySpec(factory.generateSecret(spec).getEncoded(), ALGORITHM);
        return secret;
    }

    public static boolean encryptFile(String inputFile, String outputFile, String password) {
        try {
            byte[] salt = new byte[SALT_SIZE];
            SecureRandom secureRandom = new SecureRandom();
            secureRandom.nextBytes(salt);

            byte[] iv = new byte[IV_SIZE];
            secureRandom.nextBytes(iv);

            SecretKey secretKey = getKeyFromPassword(password, salt);

            Cipher cipher = Cipher.getInstance(TRANSFORMATION);
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_BIT_LENGTH, iv);
            cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);

            try (FileInputStream fis = new FileInputStream(inputFile);
                 FileOutputStream fos = new FileOutputStream(outputFile)) {
                
                fos.write(salt);
                fos.write(iv);

                byte[] buffer = new byte[4096];
                int bytesRead;
                while ((bytesRead = fis.read(buffer)) != -1) {
                    byte[] output = cipher.update(buffer, 0, bytesRead);
                    if (output != null) {
                        fos.write(output);
                    }
                }
                byte[] outputBytes = cipher.doFinal();
                if (outputBytes != null) {
                    fos.write(outputBytes);
                }
            }
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean decryptFile(String inputFile, String outputFile, String password) {
        try (FileInputStream fis = new FileInputStream(inputFile)) {
            byte[] salt = new byte[SALT_SIZE];
            if (fis.read(salt) != SALT_SIZE) throw new IllegalArgumentException("Invalid file format: missing salt.");
            
            byte[] iv = new byte[IV_SIZE];
            if (fis.read(iv) != IV_SIZE) throw new IllegalArgumentException("Invalid file format: missing IV.");

            SecretKey secretKey = getKeyFromPassword(password, salt);

            Cipher cipher = Cipher.getInstance(TRANSFORMATION);
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_BIT_LENGTH, iv);
            cipher.init(Cipher.DECRYPT_MODE, secretKey, gcmParameterSpec);
            
            try (FileOutputStream fos = new FileOutputStream(outputFile)) {
                byte[] buffer = new byte[4096 + TAG_BIT_LENGTH / 8];
                int bytesRead;
                while ((bytesRead = fis.read(buffer)) != -1) {
                     byte[] output = cipher.update(buffer, 0, bytesRead);
                    if (output != null) {
                        fos.write(output);
                    }
                }
                 byte[] outputBytes = cipher.doFinal();
                if (outputBytes != null) {
                    fos.write(outputBytes);
                }
            }
            return true;
        } catch (Exception e) {
            // e.g., AEADBadTagException for wrong password or tampered file
            // e.printStackTrace(); 
            return false;
        }
    }

    // Main method for testing
    public static void main(String[] args) {
        System.out.println("Running Java Tests...");
        String password = "my-very-secret-password-123";

        // Test Case 1: Encrypt and decrypt a simple text file
        try {
            String testContent1 = "This is a test file for encryption.";
            Files.write(Paths.get("plain1.txt"), testContent1.getBytes());
            encryptFile("plain1.txt", "encrypted1.enc", password);
            decryptFile("encrypted1.enc", "decrypted1.txt", password);
            byte[] decryptedContent1 = Files.readAllBytes(Paths.get("decrypted1.txt"));
            if (testContent1.equals(new String(decryptedContent1))) {
                System.out.println("Test Case 1 (Simple Text): PASSED");
            } else {
                System.out.println("Test Case 1 (Simple Text): FAILED");
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println("Test Case 1 (Simple Text): FAILED with exception");
        } finally {
            cleanup("plain1.txt", "encrypted1.enc", "decrypted1.txt");
        }

        // Test Case 2: Encrypt and decrypt a larger file
        try {
            byte[] largeContent = new byte[1024 * 100]; // 100 KB
            new SecureRandom().nextBytes(largeContent);
            Files.write(Paths.get("plain2.bin"), largeContent);
            encryptFile("plain2.bin", "encrypted2.enc", password);
            decryptFile("encrypted2.enc", "decrypted2.bin", password);
            byte[] decryptedContent2 = Files.readAllBytes(Paths.get("decrypted2.bin"));
            if (Arrays.equals(largeContent, decryptedContent2)) {
                System.out.println("Test Case 2 (Large File): PASSED");
            } else {
                System.out.println("Test Case 2 (Large File): FAILED");
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println("Test Case 2 (Large File): FAILED with exception");
        } finally {
            cleanup("plain2.bin", "encrypted2.enc", "decrypted2.bin");
        }

        // Test Case 3: Empty file
         try {
            Files.write(Paths.get("plain3.txt"), "".getBytes());
            encryptFile("plain3.txt", "encrypted3.enc", password);
            decryptFile("encrypted3.enc", "decrypted3.txt", password);
            byte[] decryptedContent3 = Files.readAllBytes(Paths.get("decrypted3.txt"));
            if (decryptedContent3.length == 0) {
                System.out.println("Test Case 3 (Empty File): PASSED");
            } else {
                System.out.println("Test Case 3 (Empty File): FAILED");
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println("Test Case 3 (Empty File): FAILED with exception");
        } finally {
            cleanup("plain3.txt", "encrypted3.enc", "decrypted3.txt");
        }
        
        // Test Case 4: Decryption with wrong password (should fail)
        try {
            String testContent4 = "Test wrong password.";
            Files.write(Paths.get("plain4.txt"), testContent4.getBytes());
            encryptFile("plain4.txt", "encrypted4.enc", password);
            boolean decryptionResult = decryptFile("encrypted4.enc", "decrypted4.txt", "wrong-password");
            if (!decryptionResult) {
                System.out.println("Test Case 4 (Wrong Password): PASSED");
            } else {
                System.out.println("Test Case 4 (Wrong Password): FAILED");
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println("Test Case 4 (Wrong Password): FAILED with exception");
        } finally {
            cleanup("plain4.txt", "encrypted4.enc", "decrypted4.txt");
        }
        
        // Test Case 5: Decryption of tampered file (should fail)
        try {
            String testContent5 = "Test tampered file.";
            Files.write(Paths.get("plain5.txt"), testContent5.getBytes());
            encryptFile("plain5.txt", "encrypted5.enc", password);

            // Tamper the file
            byte[] encryptedData = Files.readAllBytes(Paths.get("encrypted5.enc"));
            encryptedData[encryptedData.length - 5] ^= (byte) 1; // Flip a bit
            Files.write(Paths.get("encrypted5.enc"), encryptedData);

            boolean decryptionResult = decryptFile("encrypted5.enc", "decrypted5.txt", password);
            if (!decryptionResult) {
                System.out.println("Test Case 5 (Tampered File): PASSED");
            } else {
                System.out.println("Test Case 5 (Tampered File): FAILED");
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.out.println("Test Case 5 (Tampered File): FAILED with exception");
        } finally {
            cleanup("plain5.txt", "encrypted5.enc", "decrypted5.txt");
        }
    }
    
    private static void cleanup(String... files) {
        for (String filename : files) {
            new File(filename).delete();
        }
    }
}