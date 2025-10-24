import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task106 {

    // Step 1: Problem understanding - RSA key structure
    static final class RSAKey {
        final BigInteger n;
        final BigInteger e;
        final BigInteger d;
        RSAKey(BigInteger n, BigInteger e, BigInteger d) {
            this.n = n;
            this.e = e;
            this.d = d;
        }
    }

    // Step 2+3: Security requirements + Secure coding generation
    public static RSAKey generateKeys(int bits) {
        if (bits < 512) {
            throw new IllegalArgumentException("Key size too small; use at least 512 bits");
        }
        SecureRandom sr = new SecureRandom();
        BigInteger e = BigInteger.valueOf(65537);
        while (true) {
            BigInteger p = BigInteger.probablePrime(bits / 2, sr);
            BigInteger q;
            do {
                q = BigInteger.probablePrime(bits / 2, sr);
            } while (p.equals(q));
            BigInteger n = p.multiply(q);
            BigInteger phi = p.subtract(BigInteger.ONE).multiply(q.subtract(BigInteger.ONE));
            if (!e.gcd(phi).equals(BigInteger.ONE)) {
                continue;
            }
            BigInteger d = e.modInverse(phi);
            return new RSAKey(n, e, d);
        }
    }

    public static BigInteger encrypt(BigInteger message, BigInteger e, BigInteger n) {
        if (message.signum() < 0 || message.compareTo(n) >= 0) {
            throw new IllegalArgumentException("Message out of range");
        }
        return message.modPow(e, n);
    }

    public static BigInteger decrypt(BigInteger ciphertext, BigInteger d, BigInteger n) {
        if (ciphertext.signum() < 0 || ciphertext.compareTo(n) >= 0) {
            throw new IllegalArgumentException("Ciphertext out of range");
        }
        return ciphertext.modPow(d, n);
    }

    public static BigInteger stringToBigInteger(String s) {
        byte[] bytes = s.getBytes(StandardCharsets.UTF_8);
        return new BigInteger(1, bytes);
    }

    public static String bigIntegerToString(BigInteger x) {
        byte[] bytes = x.toByteArray();
        // Normalize to remove possible leading zero sign byte
        if (bytes.length > 1 && bytes[0] == 0) {
            bytes = Arrays.copyOfRange(bytes, 1, bytes.length);
        }
        return new String(bytes, StandardCharsets.UTF_8);
    }

    // Step 4: Code review - included via careful checks and bounds
    // Step 5: Secure code output - final tested main with 5 test cases
    public static void main(String[] args) {
        RSAKey key = generateKeys(1024);

        String[] tests = new String[] {
                "Hello RSA",
                "Test 123",
                "Secure message",
                "Data 4",
                "Fifth test!"
        };

        for (int i = 0; i < tests.length; i++) {
            String msg = tests[i];
            BigInteger m = stringToBigInteger(msg);
            if (m.compareTo(key.n) >= 0) {
                System.out.println("Test " + (i + 1) + " message too large for modulus.");
                continue;
            }
            BigInteger c = encrypt(m, key.e, key.n);
            BigInteger p = decrypt(c, key.d, key.n);
            String recovered = bigIntegerToString(p);
            System.out.println("Test " + (i + 1) + ":");
            System.out.println("  Message: " + msg);
            System.out.println("  Cipher (hex): " + c.toString(16));
            System.out.println("  Decrypted: " + recovered);
            System.out.println("  OK: " + msg.equals(recovered));
        }
    }
}