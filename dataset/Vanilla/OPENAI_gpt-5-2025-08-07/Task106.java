import java.math.BigInteger;

public class Task106 {

    public static BigInteger modExp(BigInteger base, BigInteger exp, BigInteger mod) {
        return base.modPow(exp, mod);
    }

    public static BigInteger encrypt(BigInteger message, BigInteger e, BigInteger n) {
        return modExp(message, e, n);
    }

    public static BigInteger decrypt(BigInteger ciphertext, BigInteger d, BigInteger n) {
        return modExp(ciphertext, d, n);
    }

    public static void main(String[] args) {
        BigInteger n = BigInteger.valueOf(3233);
        BigInteger e = BigInteger.valueOf(17);
        BigInteger d = BigInteger.valueOf(2753);

        int[] messages = {65, 42, 123, 999, 2021};

        for (int m : messages) {
            BigInteger message = BigInteger.valueOf(m);
            BigInteger cipher = encrypt(message, e, n);
            BigInteger plain = decrypt(cipher, d, n);
            System.out.println("Message: " + message + " -> Cipher: " + cipher + " -> Decrypted: " + plain);
        }
    }
}