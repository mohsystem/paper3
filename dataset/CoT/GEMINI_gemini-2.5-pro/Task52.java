import javax.crypto.*;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task52 {

    private static final String ALGORITHM = "AES/GCM/NoPadding";
    private static final int TAG_LENGTH_BIT = 128;
    private static final int IV_LENGTH_BYTE = 12;
    private static final int AES_KEY_BIT = 256;

    /**
     * Encrypts a file using AES-GCM.
     *
     * @param inputFile  Path to the input file.
     * @param outputFile Path to the output encrypted file.
     * @param key        The 256-bit (32-byte) encryption key.
     * @return True if encryption is successful, false otherwise.
     */
    public static boolean encryptFile(String inputFile, String outputFile, byte[] key) {
        if (key.length * 8 != AES_KEY_BIT) {
            System.err.println("Error: Invalid key size. Key must be 256 bits (32 bytes).");
            return false;
        }

        try {
            byte[] fileBytes = Files.readAllBytes(Paths.get(inputFile));

            Cipher cipher = Cipher.getInstance(ALGORITHM);
            SecretKeySpec keySpec = new SecretKeySpec(key, "AES");

            byte[] iv = new byte[IV_LENGTH_BYTE];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv);

            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BIT, iv);
            cipher.init(Cipher.ENCRYPT_MODE, keySpec, gcmParameterSpec);
            byte[] cipherText = cipher.doFinal(fileBytes);

            // Prepend IV to ciphertext
            try (FileOutputStream out = new FileOutputStream(outputFile)) {
                out.write(iv);
                out.write(cipherText);
            }

            return true;

        } catch (NoSuchAlgorithmException | NoSuchPaddingException | InvalidKeyException |
                 InvalidAlgorithmParameterException | IllegalBlockSizeException | BadPaddingException e) {
            System.err.println("Encryption error: " + e.getMessage());
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            System.err.println("File I/O error: " + e.getMessage());
            return false;
        }
    }

    /**
     * Decrypts a file using AES-GCM.
     *
     * @param inputFile  Path to the input encrypted file.
     * @param outputFile Path to the output decrypted file.
     * @param key        The 256-bit (32-byte) decryption key.
     * @return True if decryption is successful, false otherwise.
     */
    public static boolean decryptFile(String inputFile, String outputFile, byte[] key) {
        if (key.length * 8 != AES_KEY_BIT) {
            System.err.println("Error: Invalid key size. Key must be 256 bits (32 bytes).");
            return false;
        }

        try {
            byte[] fileBytes = Files.readAllBytes(Paths.get(inputFile));

            if (fileBytes.length < IV_LENGTH_BYTE) {
                System.err.println("Error: Invalid encrypted file format.");
                return false;
            }

            // Extract IV from the beginning of the file
            byte[] iv = Arrays.copyOfRange(fileBytes, 0, IV_LENGTH_BYTE);
            byte[] cipherText = Arrays.copyOfRange(fileBytes, IV_LENGTH_BYTE, fileBytes.length);

            Cipher cipher = Cipher.getInstance(ALGORITHM);
            SecretKeySpec keySpec = new SecretKeySpec(key, "AES");
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BIT, iv);

            cipher.init(Cipher.DECRYPT_MODE, keySpec, gcmParameterSpec);
            byte[] decryptedText = cipher.doFinal(cipherText);

            try (FileOutputStream out = new FileOutputStream(outputFile)) {
                out.write(decryptedText);
            }

            return true;

        } catch (AEADBadTagException e) {
            // This is the expected exception for a wrong key or tampered data
            System.err.println("Decryption failed: Incorrect key or tampered data.");
            return false;
        } catch (NoSuchAlgorithmException | NoSuchPaddingException | InvalidKeyException |
                 InvalidAlgorithmParameterException | IllegalBlockSizeException | BadPaddingException e) {
            System.err.println("Decryption error: " + e.getMessage());
            e.printStackTrace();
            return false;
        } catch (IOException e) {
            System.err.println("File I/O error: " + e.getMessage());
            return false;
        }
    }

    // Helper to create a dummy file
    private static void createFile(String path, String content) throws IOException {
        Files.write(Paths.get(path), content.getBytes());
    }

    // Helper to read file content as string
    private static String readFile(String path) throws IOException {
        return new String(Files.readAllBytes(Paths.get(path)));
    }
    
    // Helper to tamper a file
    private static void tamperFile(String path) throws IOException {
        byte[] data = Files.readAllBytes(Paths.get(path));
        if (data.length > IV_LENGTH_BYTE + 1) { // Ensure there's ciphertext to tamper
            data[data.length - 1] ^= 1; // Flip the last bit of the ciphertext
        }
        Files.write(Paths.get(path), data);
    }
    
    public static void main(String[] args) throws IOException {
        // --- Test Setup ---
        final String ORIGINAL_FILE = "original.txt";
        final String ENCRYPTED_FILE = "encrypted.bin";
        final String DECRYPTED_FILE = "decrypted.txt";
        final String TAMPERED_FILE = "tampered.bin";
        final String EMPTY_FILE = "empty.txt";
        final String EMPTY_ENC_FILE = "empty.enc";
        final String EMPTY_DEC_FILE = "empty.dec";
        final String NON_EXISTENT_FILE = "non_existent.txt";

        // Use a secure random generator for keys in a real application
        SecureRandom random = new SecureRandom();
        byte[] correctKey = new byte[32]; // 256 bits
        random.nextBytes(correctKey);
        byte[] wrongKey = new byte[32];
        random.nextBytes(wrongKey);
        
        String originalContent = "This is a secret message for testing AES-GCM file encryption.";

        // --- Test Case 1: Successful Encryption and Decryption ---
        System.out.println("--- Test Case 1: Successful Encryption/Decryption ---");
        createFile(ORIGINAL_FILE, originalContent);
        boolean encryptSuccess = encryptFile(ORIGINAL_FILE, ENCRYPTED_FILE, correctKey);
        boolean decryptSuccess = decryptFile(ENCRYPTED_FILE, DECRYPTED_FILE, correctKey);
        if (encryptSuccess && decryptSuccess && originalContent.equals(readFile(DECRYPTED_FILE))) {
            System.out.println("Test 1 Passed: Content matches after decryption.");
        } else {
            System.out.println("Test 1 Failed.");
        }
        System.out.println("--------------------------------------------------\n");

        // --- Test Case 2: Decryption with Wrong Key ---
        System.out.println("--- Test Case 2: Decryption with Wrong Key ---");
        boolean decryptWrongKey = decryptFile(ENCRYPTED_FILE, "decrypted_wrong.txt", wrongKey);
        if (!decryptWrongKey) {
            System.out.println("Test 2 Passed: Decryption failed as expected.");
        } else {
            System.out.println("Test 2 Failed: Decryption succeeded with a wrong key.");
        }
        System.out.println("--------------------------------------------------\n");

        // --- Test Case 3: Decryption of Tampered File ---
        System.out.println("--- Test Case 3: Decryption of Tampered File ---");
        Files.copy(Paths.get(ENCRYPTED_FILE), Paths.get(TAMPERED_FILE));
        tamperFile(TAMPERED_FILE);
        boolean decryptTampered = decryptFile(TAMPERED_FILE, "decrypted_tampered.txt", correctKey);
        if (!decryptTampered) {
            System.out.println("Test 3 Passed: Decryption of tampered file failed as expected.");
        } else {
            System.out.println("Test 3 Failed: Decryption of tampered file succeeded.");
        }
        System.out.println("--------------------------------------------------\n");
        
        // --- Test Case 4: Encrypting and Decrypting an Empty File ---
        System.out.println("--- Test Case 4: Encrypt/Decrypt Empty File ---");
        createFile(EMPTY_FILE, "");
        boolean encryptEmptySuccess = encryptFile(EMPTY_FILE, EMPTY_ENC_FILE, correctKey);
        boolean decryptEmptySuccess = decryptFile(EMPTY_ENC_FILE, EMPTY_DEC_FILE, correctKey);
        if (encryptEmptySuccess && decryptEmptySuccess && readFile(EMPTY_DEC_FILE).isEmpty()) {
            System.out.println("Test 4 Passed: Empty file handled correctly.");
        } else {
            System.out.println("Test 4 Failed.");
        }
        System.out.println("--------------------------------------------------\n");

        // --- Test Case 5: Handling Non-Existent Input File ---
        System.out.println("--- Test Case 5: Handling Non-Existent Input File ---");
        boolean encryptNonExistent = encryptFile(NON_EXISTENT_FILE, "out.enc", correctKey);
        if (!encryptNonExistent) {
            System.out.println("Test 5 Passed: Handled non-existent input file correctly.");
        } else {
            System.out.println("Test 5 Failed.");
        }
        System.out.println("--------------------------------------------------\n");
        
        // --- Cleanup ---
        Files.deleteIfExists(Paths.get(ORIGINAL_FILE));
        Files.deleteIfExists(Paths.get(ENCRYPTED_FILE));
        Files.deleteIfExists(Paths.get(DECRYPTED_FILE));
        Files.deleteIfExists(Paths.get("decrypted_wrong.txt"));
        Files.deleteIfExists(Paths.get(TAMPERED_FILE));
        Files.deleteIfExists(Paths.get("decrypted_tampered.txt"));
        Files.deleteIfExists(Paths.get(EMPTY_FILE));
        Files.deleteIfExists(Paths.get(EMPTY_ENC_FILE));
        Files.deleteIfExists(Paths.get(EMPTY_DEC_FILE));
    }
}