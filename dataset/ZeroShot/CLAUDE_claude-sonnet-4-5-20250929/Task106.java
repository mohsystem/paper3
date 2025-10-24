
import java.math.BigInteger;
import java.security.SecureRandom;

public class Task106 {
    private BigInteger n, d, e;
    private int bitlen = 1024;

    public Task106() {
        SecureRandom r = new SecureRandom();
        BigInteger p = BigInteger.probablePrime(bitlen / 2, r);
        BigInteger q = BigInteger.probablePrime(bitlen / 2, r);
        n = p.multiply(q);
        BigInteger phi = p.subtract(BigInteger.ONE).multiply(q.subtract(BigInteger.ONE));
        e = BigInteger.valueOf(65537);
        d = e.modInverse(phi);
    }

    public Task106(BigInteger e, BigInteger d, BigInteger n) {
        this.e = e;
        this.d = d;
        this.n = n;
    }

    public BigInteger encrypt(BigInteger message) {
        return message.modPow(e, n);
    }

    public BigInteger decrypt(BigInteger encrypted) {
        return encrypted.modPow(d, n);
    }

    public String encryptString(String message) {
        return encrypt(new BigInteger(message.getBytes())).toString();
    }

    public String decryptString(String encrypted) {
        return new String(decrypt(new BigInteger(encrypted)).toByteArray());
    }

    public BigInteger getPublicKey() {
        return e;
    }

    public BigInteger getPrivateKey() {
        return d;
    }

    public BigInteger getModulus() {
        return n;
    }

    public static void main(String[] args) {
        System.out.println("RSA Encryption/Decryption Test Cases:\\n");

        // Test Case 1
        System.out.println("Test Case 1:");
        Task106 rsa1 = new Task106();
        String msg1 = "Hello";
        String encrypted1 = rsa1.encryptString(msg1);
        String decrypted1 = rsa1.decryptString(encrypted1);
        System.out.println("Original: " + msg1);
        System.out.println("Decrypted: " + decrypted1);
        System.out.println("Match: " + msg1.equals(decrypted1) + "\\n");

        // Test Case 2
        System.out.println("Test Case 2:");
        Task106 rsa2 = new Task106();
        String msg2 = "RSA Algorithm";
        String encrypted2 = rsa2.encryptString(msg2);
        String decrypted2 = rsa2.decryptString(encrypted2);
        System.out.println("Original: " + msg2);
        System.out.println("Decrypted: " + decrypted2);
        System.out.println("Match: " + msg2.equals(decrypted2) + "\\n");

        // Test Case 3
        System.out.println("Test Case 3:");
        Task106 rsa3 = new Task106();
        BigInteger num3 = new BigInteger("12345");
        BigInteger encrypted3 = rsa3.encrypt(num3);
        BigInteger decrypted3 = rsa3.decrypt(encrypted3);
        System.out.println("Original: " + num3);
        System.out.println("Decrypted: " + decrypted3);
        System.out.println("Match: " + num3.equals(decrypted3) + "\\n");

        // Test Case 4
        System.out.println("Test Case 4:");
        Task106 rsa4 = new Task106();
        String msg4 = "Secure Communication";
        String encrypted4 = rsa4.encryptString(msg4);
        String decrypted4 = rsa4.decryptString(encrypted4);
        System.out.println("Original: " + msg4);
        System.out.println("Decrypted: " + decrypted4);
        System.out.println("Match: " + msg4.equals(decrypted4) + "\\n");

        // Test Case 5
        System.out.println("Test Case 5:");
        Task106 rsa5 = new Task106();
        String msg5 = "123";
        String encrypted5 = rsa5.encryptString(msg5);
        String decrypted5 = rsa5.decryptString(encrypted5);
        System.out.println("Original: " + msg5);
        System.out.println("Decrypted: " + decrypted5);
        System.out.println("Match: " + msg5.equals(decrypted5) + "\\n");
    }
}
