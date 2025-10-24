import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.LinkedHashMap;
import java.util.Map;

public class Task124 {
    // Securely serialize a map as lines of base64(key)=base64(value)
    public static byte[] serialize(Map<String, String> data) {
        StringBuilder sb = new StringBuilder();
        Base64.Encoder enc = Base64.getEncoder();
        for (Map.Entry<String, String> e : data.entrySet()) {
            String k = enc.encodeToString(e.getKey().getBytes(StandardCharsets.UTF_8));
            String v = enc.encodeToString(e.getValue().getBytes(StandardCharsets.UTF_8));
            sb.append(k).append('=').append(v).append('\n');
        }
        return sb.toString().getBytes(StandardCharsets.UTF_8);
    }

    // Deserialize map from the serialize format above
    public static Map<String, String> deserialize(byte[] bytes) {
        Map<String, String> out = new LinkedHashMap<>();
        String s = new String(bytes, StandardCharsets.UTF_8);
        Base64.Decoder dec = Base64.getDecoder();
        String[] lines = s.split("\n");
        for (String line : lines) {
            if (line.isEmpty()) continue;
            int idx = line.indexOf('=');
            if (idx <= 0) continue;
            String kB64 = line.substring(0, idx);
            String vB64 = line.substring(idx + 1);
            String k = new String(dec.decode(kB64), StandardCharsets.UTF_8);
            String v = new String(dec.decode(vB64), StandardCharsets.UTF_8);
            out.put(k, v);
        }
        return out;
    }

    // Derive key from password using PBKDF2-HMAC-SHA256
    private static byte[] kdf(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return skf.generateSecret(spec).getEncoded();
    }

    // Encrypt data using AES-256-GCM with PBKDF2 derived key.
    // Returns a self-contained binary blob containing header + salt + iv + ciphertext.
    public static byte[] encrypt(Map<String, String> data, char[] password) throws Exception {
        SecureRandom rng = new SecureRandom();
        byte[] salt = new byte[16];
        rng.nextBytes(salt);
        byte[] key = kdf(password, salt, 200_000, 32);

        byte[] iv = new byte[12];
        rng.nextBytes(iv);

        byte[] plaintext = serialize(data);

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec gcm = new GCMParameterSpec(128, iv);
        cipher.init(Cipher.ENCRYPT_MODE, new SecretKeySpec(key, "AES"), gcm);
        cipher.updateAAD("SVLTv1".getBytes(StandardCharsets.US_ASCII));
        byte[] ct = cipher.doFinal(plaintext);

        // Build blob: magic "SVLT" (4) + version(1) + saltLen(1) + ivLen(1) + salt + iv + ct
        ByteBuffer buf = ByteBuffer.allocate(4 + 1 + 1 + 1 + salt.length + iv.length + ct.length);
        buf.put(new byte[]{'S', 'V', 'L', 'T'});
        buf.put((byte) 1);
        buf.put((byte) salt.length);
        buf.put((byte) iv.length);
        buf.put(salt);
        buf.put(iv);
        buf.put(ct);
        // Zero sensitive
        java.util.Arrays.fill(key, (byte) 0);
        java.util.Arrays.fill(plaintext, (byte) 0);
        return buf.array();
    }

    // Decrypt data produced by encrypt()
    public static Map<String, String> decrypt(byte[] blob, char[] password) throws Exception {
        ByteBuffer buf = ByteBuffer.wrap(blob);
        byte[] magic = new byte[4];
        buf.get(magic);
        if (magic[0] != 'S' || magic[1] != 'V' || magic[2] != 'L' || magic[3] != 'T') {
            throw new IllegalArgumentException("Invalid blob");
        }
        byte version = buf.get();
        if (version != 1) throw new IllegalArgumentException("Unsupported version");
        int saltLen = Byte.toUnsignedInt(buf.get());
        int ivLen = Byte.toUnsignedInt(buf.get());
        if (saltLen <= 0 || ivLen <= 0) throw new IllegalArgumentException("Invalid lengths");
        byte[] salt = new byte[saltLen];
        byte[] iv = new byte[ivLen];
        buf.get(salt);
        buf.get(iv);
        byte[] ct = new byte[buf.remaining()];
        buf.get(ct);

        byte[] key = kdf(password, salt, 200_000, 32);
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec gcm = new GCMParameterSpec(128, iv);
        cipher.init(Cipher.DECRYPT_MODE, new SecretKeySpec(key, "AES"), gcm);
        cipher.updateAAD("SVLTv1".getBytes(StandardCharsets.US_ASCII));
        byte[] pt = cipher.doFinal(ct);

        Map<String, String> out = deserialize(pt);

        java.util.Arrays.fill(key, (byte) 0);
        java.util.Arrays.fill(pt, (byte) 0);
        return out;
    }

    // Save bytes to a file (overwrites)
    public static void saveToFile(byte[] data, String filename) throws IOException {
        Files.write(Paths.get(filename), data);
    }

    // Load bytes from a file
    public static byte[] loadFromFile(String filename) throws IOException {
        return Files.readAllBytes(Paths.get(filename));
    }

    // Example 5 test cases
    public static void main(String[] args) throws Exception {
        // Test case 1
        Map<String, String> m1 = new LinkedHashMap<>();
        m1.put("cardNumber", "4111111111111111");
        m1.put("cardHolder", "ALICE A");
        m1.put("expiry", "12/29");
        byte[] b1 = encrypt(m1, "StrongPass#1".toCharArray());
        saveToFile(b1, "vault1.bin");
        Map<String, String> r1 = decrypt(loadFromFile("vault1.bin"), "StrongPass#1".toCharArray());
        System.out.println("Test1: " + r1);

        // Test case 2
        Map<String, String> m2 = new LinkedHashMap<>();
        m2.put("ssn", "123-45-6789");
        m2.put("name", "Bob B");
        byte[] b2 = encrypt(m2, "StrongPass#2".toCharArray());
        saveToFile(b2, "vault2.bin");
        Map<String, String> r2 = decrypt(loadFromFile("vault2.bin"), "StrongPass#2".toCharArray());
        System.out.println("Test2: " + r2);

        // Test case 3
        Map<String, String> m3 = new LinkedHashMap<>();
        m3.put("email", "carol@example.com");
        m3.put("dob", "1990-07-15");
        m3.put("address", "123 Main St, City");
        byte[] b3 = encrypt(m3, "StrongPass#3".toCharArray());
        saveToFile(b3, "vault3.bin");
        Map<String, String> r3 = decrypt(loadFromFile("vault3.bin"), "StrongPass#3".toCharArray());
        System.out.println("Test3: " + r3);

        // Test case 4
        Map<String, String> m4 = new LinkedHashMap<>();
        m4.put("passport", "X12345678");
        m4.put("country", "Wonderland");
        byte[] b4 = encrypt(m4, "StrongPass#4".toCharArray());
        saveToFile(b4, "vault4.bin");
        Map<String, String> r4 = decrypt(loadFromFile("vault4.bin"), "StrongPass#4".toCharArray());
        System.out.println("Test4: " + r4);

        // Test case 5
        Map<String, String> m5 = new LinkedHashMap<>();
        m5.put("apiKey", "sk_test_51Hk...");
        m5.put("note", "Rotate regularly");
        byte[] b5 = encrypt(m5, "StrongPass#5".toCharArray());
        saveToFile(b5, "vault5.bin");
        Map<String, String> r5 = decrypt(loadFromFile("vault5.bin"), "StrongPass#5".toCharArray());
        System.out.println("Test5: " + r5);
    }
}