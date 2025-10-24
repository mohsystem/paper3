
import javax.crypto.*;
import javax.crypto.spec.*;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.*;
import java.security.spec.*;
import java.util.*;
import java.util.concurrent.locks.*;

public class Task134 {
    private static final String MAGIC = "KEY1";
    private static final int VERSION = 1;
    private static final int SALT_SIZE = 16;
    private static final int IV_SIZE = 12;
    private static final int TAG_SIZE = 16;
    private static final int ITERATIONS = 210000;
    private static final String KEY_ALGORITHM = "AES";
    private static final String CIPHER_ALGORITHM = "AES/GCM/NoPadding";
    private static final String KDF_ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int KEY_SIZE = 256;
    
    private final Path baseDirectory;
    private final ReentrantLock lock = new ReentrantLock();
    
    public Task134(String baseDir) throws IOException {
        if (baseDir == null || baseDir.isEmpty()) {
            throw new IllegalArgumentException("Base directory cannot be null or empty");
        }
        this.baseDirectory = Paths.get(baseDir).toAbsolutePath().normalize();
        Files.createDirectories(this.baseDirectory);
    }
    
    private Path validatePath(String filename) throws IOException {
        if (filename == null || filename.isEmpty()) {
            throw new IllegalArgumentException("Filename cannot be null or empty");
        }
        if (filename.contains("..") || filename.contains("/") || filename.contains("\\\\")) {\n            throw new IllegalArgumentException("Invalid filename");\n        }\n        Path resolved = this.baseDirectory.resolve(filename).normalize();\n        if (!resolved.startsWith(this.baseDirectory)) {\n            throw new SecurityException("Path traversal detected");\n        }\n        return resolved;\n    }\n    \n    private byte[] deriveKey(char[] passphrase, byte[] salt) throws Exception {\n        if (passphrase == null || passphrase.length == 0) {\n            throw new IllegalArgumentException("Passphrase cannot be null or empty");\n        }\n        if (salt == null || salt.length != SALT_SIZE) {\n            throw new IllegalArgumentException("Invalid salt size");\n        }\n        \n        PBEKeySpec spec = new PBEKeySpec(passphrase, salt, ITERATIONS, KEY_SIZE);\n        try {\n            SecretKeyFactory factory = SecretKeyFactory.getInstance(KDF_ALGORITHM);\n            byte[] derived = factory.generateSecret(spec).getEncoded();\n            return derived;\n        } finally {\n            spec.clearPassword();\n        }\n    }\n    \n    public void generateAndStoreKey(String keyName, char[] passphrase) throws Exception {\n        if (keyName == null || keyName.isEmpty()) {\n            throw new IllegalArgumentException("Key name cannot be null or empty");\n        }\n        if (passphrase == null || passphrase.length < 12) {\n            throw new IllegalArgumentException("Passphrase must be at least 12 characters");\n        }\n        \n        lock.lock();\n        try {\n            Path keyPath = validatePath(keyName + ".key");\n            \n            SecureRandom random = new SecureRandom();\n            byte[] keyMaterial = new byte[32];\n            random.nextBytes(keyMaterial);\n            \n            byte[] salt = new byte[SALT_SIZE];\n            random.nextBytes(salt);\n            \n            byte[] iv = new byte[IV_SIZE];\n            random.nextBytes(iv);\n            \n            byte[] derivedKey = deriveKey(passphrase, salt);\n            SecretKeySpec keySpec = new SecretKeySpec(derivedKey, KEY_ALGORITHM);\n            \n            Cipher cipher = Cipher.getInstance(CIPHER_ALGORITHM);\n            GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_SIZE * 8, iv);\n            cipher.init(Cipher.ENCRYPT_MODE, keySpec, gcmSpec);\n            \n            byte[] ciphertext = cipher.doFinal(keyMaterial);\n            \n            Path tempPath = Files.createTempFile(this.baseDirectory, "tmp", ".key");\n            try (FileOutputStream fos = new FileOutputStream(tempPath.toFile())) {\n                fos.write(MAGIC.getBytes(StandardCharsets.UTF_8));\n                fos.write(VERSION);\n                fos.write(salt);\n                fos.write(iv);\n                fos.write(ciphertext);\n                fos.flush();\n                fos.getFD().sync();\n            }\n            \n            Files.move(tempPath, keyPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);\n            \n            Arrays.fill(keyMaterial, (byte) 0);\n            Arrays.fill(derivedKey, (byte) 0);\n        } finally {\n            lock.unlock();\n        }\n    }\n    \n    public byte[] retrieveKey(String keyName, char[] passphrase) throws Exception {\n        if (keyName == null || keyName.isEmpty()) {\n            throw new IllegalArgumentException("Key name cannot be null or empty");\n        }\n        if (passphrase == null || passphrase.length == 0) {\n            throw new IllegalArgumentException("Passphrase cannot be null or empty");\n        }\n        \n        lock.lock();\n        try {\n            Path keyPath = validatePath(keyName + ".key");\n            \n            if (!Files.isRegularFile(keyPath)) {\n                throw new IOException("Key file not found or not a regular file");\n            }\n            \n            byte[] fileData = Files.readAllBytes(keyPath);\n            \n            int minSize = MAGIC.length() + 1 + SALT_SIZE + IV_SIZE + TAG_SIZE;\n            if (fileData.length < minSize) {\n                throw new SecurityException("Invalid key file format");\n            }\n            \n            int offset = 0;\n            byte[] magic = Arrays.copyOfRange(fileData, offset, offset + MAGIC.length());\n            offset += MAGIC.length();\n            \n            if (!Arrays.equals(magic, MAGIC.getBytes(StandardCharsets.UTF_8))) {\n                throw new SecurityException("Invalid magic number");\n            }\n            \n            int version = fileData[offset++] & 0xFF;\n            if (version != VERSION) {\n                throw new SecurityException("Unsupported version");\n            }\n            \n            byte[] salt = Arrays.copyOfRange(fileData, offset, offset + SALT_SIZE);\n            offset += SALT_SIZE;\n            \n            byte[] iv = Arrays.copyOfRange(fileData, offset, offset + IV_SIZE);\n            offset += IV_SIZE;\n            \n            byte[] ciphertext = Arrays.copyOfRange(fileData, offset, fileData.length);\n            \n            byte[] derivedKey = deriveKey(passphrase, salt);\n            SecretKeySpec keySpec = new SecretKeySpec(derivedKey, KEY_ALGORITHM);\n            \n            Cipher cipher = Cipher.getInstance(CIPHER_ALGORITHM);\n            GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_SIZE * 8, iv);\n            cipher.init(Cipher.DECRYPT_MODE, keySpec, gcmSpec);\n            \n            byte[] keyMaterial = cipher.doFinal(ciphertext);\n            \n            Arrays.fill(derivedKey, (byte) 0);\n            \n            return keyMaterial;\n        } finally {\n            lock.unlock();\n        }\n    }\n    \n    public boolean deleteKey(String keyName) throws Exception {\n        if (keyName == null || keyName.isEmpty()) {\n            throw new IllegalArgumentException("Key name cannot be null or empty");\n        }\n        \n        lock.lock();\n        try {\n            Path keyPath = validatePath(keyName + ".key");\n            return Files.deleteIfExists(keyPath);\n        } finally {\n            lock.unlock();\n        }\n    }\n    \n    public List<String> listKeys() throws Exception {\n        lock.lock();\n        try {\n            List<String> keys = new ArrayList<>();\n            try (DirectoryStream<Path> stream = Files.newDirectoryStream(this.baseDirectory, "*.key")) {\n                for (Path entry : stream) {\n                    if (Files.isRegularFile(entry)) {\n                        String filename = entry.getFileName().toString();\n                        keys.add(filename.substring(0, filename.length() - 4));\n                    }\n                }\n            }\n            return keys;\n        } finally {\n            lock.unlock();\n        }\n    }\n    \n    public static void main(String[] args) {\n        try {\n            Path tempDir = Files.createTempDirectory("keymanager");\n            Task134 keyManager = new Task134(tempDir.toString());\n            \n            System.out.println("Test 1: Generate and retrieve key");\n            char[] passphrase1 = "SecurePassphrase123!".toCharArray();\n            keyManager.generateAndStoreKey("testkey1", passphrase1);\n            byte[] key1 = keyManager.retrieveKey("testkey1", passphrase1);\n            System.out.println("Key retrieved successfully: " + (key1.length == 32));\n            Arrays.fill(key1, (byte) 0);\n            Arrays.fill(passphrase1, ' ');\n            \n            System.out.println("\
Test 2: List keys");\n            char[] passphrase2 = "AnotherSecure456!".toCharArray();\n            keyManager.generateAndStoreKey("testkey2", passphrase2);\n            List<String> keys = keyManager.listKeys();\n            System.out.println("Keys found: " + keys.size());\n            Arrays.fill(passphrase2, ' ');\n            \n            System.out.println("\
Test 3: Invalid passphrase");\n            char[] wrongPass = "WrongPassphrase789!".toCharArray();\n            try {\n                keyManager.retrieveKey("testkey1", wrongPass);\n                System.out.println("ERROR: Should have failed");\n            } catch (Exception e) {\n                System.out.println("Correctly rejected wrong passphrase");\n            }\n            Arrays.fill(wrongPass, ' ');\n            \n            System.out.println("\
Test 4: Delete key");\n            boolean deleted = keyManager.deleteKey("testkey1");\n            System.out.println("Key deleted: " + deleted);\n            \n            System.out.println("\
Test 5: Path traversal prevention");\n            try {\n                keyManager.generateAndStoreKey("../etc/passwd", "test123456789!".toCharArray());\n                System.out.println("ERROR: Should have prevented path traversal");\n            } catch (IllegalArgumentException e) {\n                System.out.println("Path traversal correctly prevented");
            }
            
            for (String key : keyManager.listKeys()) {
                keyManager.deleteKey(key);
            }
            Files.delete(tempDir);
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
