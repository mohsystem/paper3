public class Task79 {

    /**
     * Encrypts a message using the Vigenère cipher.
     *
     * @param message The plaintext message to encrypt.
     * @param key The secret key for encryption.
     * @return The encrypted ciphertext.
     */
    public static String encrypt(String message, String key) {
        if (key == null || key.isEmpty()) {
            return message;
        }

        StringBuilder encryptedMessage = new StringBuilder();
        String upperMessage = message.toUpperCase();
        String upperKey = key.toUpperCase();
        
        int keyIndex = 0;
        for (int i = 0; i < upperMessage.length(); i++) {
            char messageChar = upperMessage.charAt(i);

            if (Character.isLetter(messageChar)) {
                char keyChar = upperKey.charAt(keyIndex % upperKey.length());
                int shift = keyChar - 'A';
                char encryptedChar = (char) (((messageChar - 'A' + shift) % 26) + 'A');
                encryptedMessage.append(encryptedChar);
                keyIndex++;
            } else {
                encryptedMessage.append(message.charAt(i)); // Preserve original non-letter characters
            }
        }
        return encryptedMessage.toString();
    }

    public static void main(String[] args) {
        String[] messages = {
            "ATTACK AT DAWN",
            "HELLO WORLD",
            "Cryptography",
            "This is a test message 123!",
            "ALLCAPS"
        };
        String[] keys = {
            "LEMON",
            "KEY",
            "SECRET",
            "CIPHER",
            "ALLCAPS"
        };

        for (int i = 0; i < messages.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            String message = messages[i];
            String key = keys[i];
            String encrypted = encrypt(message, key);
            System.out.println("  Original:  " + message);
            System.out.println("  Key:       " + key);
            System.out.println("  Encrypted: " + encrypted);
            System.out.println();
        }
    }
}