public class Task51 {

    /**
     * Encrypts a string using a repeating key XOR cipher and returns the hex-encoded result.
     * This is a simple educational cipher and is NOT secure for real-world use.
     *
     * @param plaintext The string to encrypt.
     * @param key The encryption key.
     * @return The hex-encoded encrypted string, or the original plaintext if the key is null or empty.
     */
    public static String encrypt(String plaintext, String key) {
        if (plaintext == null || key == null || key.isEmpty()) {
            return plaintext;
        }

        StringBuilder hexCiphertext = new StringBuilder();
        byte[] plaintextBytes = plaintext.getBytes();
        byte[] keyBytes = key.getBytes();

        for (int i = 0; i < plaintextBytes.length; i++) {
            byte encryptedByte = (byte) (plaintextBytes[i] ^ keyBytes[i % keyBytes.length]);
            hexCiphertext.append(String.format("%02x", encryptedByte));
        }

        return hexCiphertext.toString();
    }

    public static void main(String[] args) {
        String[][] testCases = {
            {"Hello, World!", "KEY"},
            {"This is a longer test message.", "SECRET"},
            {"Short", "LONGERKEY"},
            {"12345!@#$%", "NUMSYM"},
            {"Test with empty key", ""}
        };

        System.out.println("Java Encryption Test Cases:");
        for (String[] testCase : testCases) {
            String plaintext = testCase[0];
            String key = testCase[1];
            String encrypted = encrypt(plaintext, key);
            System.out.println("Plaintext : \"" + plaintext + "\", Key: \"" + key + "\"");
            System.out.println("Encrypted : " + encrypted);
            System.out.println("--------------------");
        }
    }
}