import java.math.BigInteger;

public class Task106 {

    private BigInteger p, q, n, phi, e, d;

    // Constructor to generate keys with fixed p and q for demonstration
    public Task106() {
        // Using small primes for demonstration consistency with C/C++
        // In a real-world scenario, use large, randomly generated primes.
        p = new BigInteger("61");
        q = new BigInteger("53");

        // n = p * q
        n = p.multiply(q);
        
        // phi = (p - 1) * (q - 1)
        phi = p.subtract(BigInteger.ONE).multiply(q.subtract(BigInteger.ONE));

        // Choose e such that 1 < e < phi and gcd(e, phi) = 1
        e = new BigInteger("17"); // A common choice for e

        // Calculate d, the modular multiplicative inverse of e mod phi
        d = e.modInverse(phi);
    }

    public BigInteger getPublicKeyE() {
        return e;
    }

    public BigInteger getPrivateKeyD() {
        return d;
    }

    public BigInteger getModulusN() {
        return n;
    }

    /**
     * Encrypts a message using the public key.
     * @param message The message to encrypt as a BigInteger.
     * @param e The public exponent.
     * @param n The modulus.
     * @return The encrypted message (ciphertext) as a BigInteger.
     */
    public static BigInteger encrypt(BigInteger message, BigInteger e, BigInteger n) {
        return message.modPow(e, n);
    }

    /**
     * Decrypts a message using the private key.
     * @param encryptedMessage The ciphertext to decrypt.
     * @param d The private exponent.
     * @param n The modulus.
     * @return The decrypted message (plaintext) as a BigInteger.
     */
    public static BigInteger decrypt(BigInteger encryptedMessage, BigInteger d, BigInteger n) {
        return encryptedMessage.modPow(d, n);
    }

    public static void main(String[] args) {
        Task106 rsa = new Task106();

        BigInteger publicKeyE = rsa.getPublicKeyE();
        BigInteger privateKeyD = rsa.getPrivateKeyD();
        BigInteger modulusN = rsa.getModulusN();

        System.out.println("Public Key (e, n): (" + publicKeyE + ", " + modulusN + ")");
        System.out.println("Private Key (d, n): (" + privateKeyD + ", " + modulusN + ")");
        System.out.println("----------------------------------------");

        long[] testMessages = {123, 89, 999, 2500, 3000};

        for (int i = 0; i < testMessages.length; i++) {
            BigInteger originalMessage = BigInteger.valueOf(testMessages[i]);
            System.out.println("Test Case " + (i + 1));
            System.out.println("Original Message: " + originalMessage);

            // Encrypt the message
            BigInteger encryptedMessage = encrypt(originalMessage, publicKeyE, modulusN);
            System.out.println("Encrypted Message: " + encryptedMessage);

            // Decrypt the message
            BigInteger decryptedMessage = decrypt(encryptedMessage, privateKeyD, modulusN);
            System.out.println("Decrypted Message: " + decryptedMessage);
            System.out.println("----------------------------------------");
        }
    }
}