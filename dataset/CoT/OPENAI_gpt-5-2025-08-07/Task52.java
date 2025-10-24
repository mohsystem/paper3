import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.SecureRandom;
import java.util.Arrays;

/*
Chain-of-Through process:
1) Problem understanding: Provide file encryption/decryption using authenticated encryption (AES-GCM), with functions that take parameters and return outputs; include tests.
2) Security requirements: Use AES-256-GCM with random 96-bit IV, SecureRandom, avoid ECB, include authentication tag, validate decryption.
3) Secure coding generation: Stream I/O, minimal data exposure, proper exception handling, verify outputs.
4) Code review: Avoid hard-coded keys, use constant-time comparisons where relevant, ensure IV uniqueness, write IV+ciphertext+tag format safely.
5) Secure code output: Final hardened implementation below.
*/
public class Task52 {

    // Secure key generation (256-bit)
    public static byte[] generateKey(int bits) throws Exception {
        if (bits != 128 && bits != 192 && bits != 256) {
            throw new IllegalArgumentException("AES key size must be 128, 192, or 256 bits");
        }
        KeyGenerator kg = KeyGenerator.getInstance("AES");
        kg.init(bits, SecureRandom.getInstanceStrong());
        SecretKey key = kg.generateKey();
        return key.getEncoded();
    }

    // Encrypts file: writes [IV (12 bytes)] + [ciphertext||tag] to output
    public static void encryptFile(byte[] key, String inputPath, String outputPath) throws Exception {
        if (key == null || (key.length != 16 && key.length != 24 && key.length != 32))
            throw new IllegalArgumentException("Invalid AES key length");
        byte[] iv = new byte[12];
        SecureRandom.getInstanceStrong().nextBytes(iv);

        SecretKeySpec keySpec = new SecretKeySpec(key, "AES");
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(128, iv);
        cipher.init(Cipher.ENCRYPT_MODE, keySpec, spec);

        try (FileOutputStream fos = new FileOutputStream(outputPath)) {
            fos.write(iv);
            try (CipherOutputStream cos = new CipherOutputStream(fos, cipher);
                 FileInputStream fis = new FileInputStream(inputPath)) {
                byte[] buffer = new byte[8192];
                int read;
                while ((read = fis.read(buffer)) != -1) {
                    cos.write(buffer, 0, read);
                }
                cos.flush();
            }
        }
    }

    // Decrypts file from [IV||ciphertext||tag] format
    public static void decryptFile(byte[] key, String inputPath, String outputPath) throws Exception {
        if (key == null || (key.length != 16 && key.length != 24 && key.length != 32))
            throw new IllegalArgumentException("Invalid AES key length");

        byte[] iv = new byte[12];
        try (FileInputStream fis = new FileInputStream(inputPath)) {
            if (fis.read(iv) != iv.length) throw new IOException("Invalid input file: missing IV");
            SecretKeySpec keySpec = new SecretKeySpec(key, "AES");
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec spec = new GCMParameterSpec(128, iv);
            cipher.init(Cipher.DECRYPT_MODE, keySpec, spec);
            try (CipherInputStream cis = new CipherInputStream(fis, cipher);
                 FileOutputStream fos = new FileOutputStream(outputPath)) {
                byte[] buffer = new byte[8192];
                int read;
                while ((read = cis.read(buffer)) != -1) {
                    fos.write(buffer, 0, read);
                }
                fos.flush();
            } catch (IOException e) {
                // Thrown on authentication failure or IO error
                throw e;
            }
        }
    }

    // Constant-time file equality check (loads fully; for demo/testing)
    public static boolean filesEqual(String path1, String path2) throws IOException {
        byte[] a = Files.readAllBytes(Path.of(path1));
        byte[] b = Files.readAllBytes(Path.of(path2));
        if (a.length != b.length) return false;
        int diff = 0;
        for (int i = 0; i < a.length; i++) diff |= (a[i] ^ b[i]);
        return diff == 0;
    }

    // Helper to write test file content
    private static void writeBytes(String path, byte[] data) throws IOException {
        Files.write(Path.of(path), data);
    }

    // 5 test cases in main
    public static void main(String[] args) throws Exception {
        byte[] key = generateKey(256);
        // Prepare test inputs
        byte[][] testData = new byte[5][];
        testData[0] = new byte[0]; // empty file
        testData[1] = "Hello, AES-GCM!".getBytes("UTF-8");
        testData[2] = "Unicode: こんにちは世界🌍".getBytes("UTF-8");
        testData[3] = new byte[1024]; // 1KB zeros
        testData[4] = new byte[4096]; // 4KB random
        SecureRandom.getInstanceStrong().nextBytes(testData[4]);

        String base = System.getProperty("java.io.tmpdir");
        boolean allOk = true;

        for (int i = 0; i < 5; i++) {
            String in = Path.of(base, "java_in_" + i + ".bin").toString();
            String enc = Path.of(base, "java_enc_" + i + ".bin").toString();
            String out = Path.of(base, "java_out_" + i + ".bin").toString();
            writeBytes(in, testData[i]);
            try {
                encryptFile(key, in, enc);
                decryptFile(key, enc, out);
                boolean eq = filesEqual(in, out);
                allOk &= eq;
            } finally {
                // best-effort cleanup
                try { Files.deleteIfExists(Path.of(in)); } catch (Exception ignore) {}
                try { Files.deleteIfExists(Path.of(enc)); } catch (Exception ignore) {}
                try { Files.deleteIfExists(Path.of(out)); } catch (Exception ignore) {}
            }
        }
        System.out.println(allOk ? "Java tests passed" : "Java tests failed");
    }
}