import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.interfaces.RSAPrivateCrtKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Arrays;
import java.util.Base64;

public class Task78 {

    public static String generateOpenSSHRSAPrivateKey(int keySize, String comment) throws GeneralSecurityException, IOException {
        if (keySize < 2048 || keySize > 16384) {
            throw new IllegalArgumentException("keySize must be between 2048 and 16384 bits");
        }
        if (comment == null) {
            comment = "";
        }

        KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA");
        SecureRandom sr = SecureRandom.getInstanceStrong();
        kpg.initialize(keySize, sr);
        KeyPair kp = kpg.generateKeyPair();

        RSAPublicKey pub = (RSAPublicKey) kp.getPublic();
        RSAPrivateCrtKey priv = (RSAPrivateCrtKey) kp.getPrivate();

        byte[] openssh = buildOpenSSHPrivateKey(priv, pub, comment, sr);

        String base64 = Base64.getEncoder().encodeToString(openssh);
        StringBuilder sb = new StringBuilder();
        sb.append("-----BEGIN OPENSSH PRIVATE KEY-----\n");
        for (int i = 0; i < base64.length(); i += 70) {
            sb.append(base64, i, Math.min(i + 70, base64.length())).append('\n');
        }
        sb.append("-----END OPENSSH PRIVATE KEY-----\n");
        return sb.toString();
    }

    private static byte[] buildOpenSSHPrivateKey(RSAPrivateCrtKey priv, RSAPublicKey pub, String comment, SecureRandom sr) throws IOException {
        // Public key blob: string "ssh-rsa" + mpint e + mpint n
        byte[] pubBlob = buildRSAPublicKeyBlob(pub);

        // Private key block (plaintext for cipher "none")
        byte[] privateBlock = buildRSAPrivateBlock(priv, pub, comment, sr);

        // Overall format:
        // "openssh-key-v1" 0x00
        // string ciphername ("none")
        // string kdfname ("none")
        // string kdfoptions (empty)
        // uint32 number of keys (1)
        // string public key
        // string private key (includes checkints, key data, comment, padding)
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        // Magic
        baos.write("openssh-key-v1".getBytes(StandardCharsets.US_ASCII));
        baos.write(0x00);

        try (DataOutputStream dos = new DataOutputStream(baos)) {
            writeString(dos, "none");
            writeString(dos, "none");
            writeString(dos, new byte[0]); // empty kdfopts
            dos.writeInt(1); // number of keys
            writeString(dos, pubBlob);
            writeString(dos, privateBlock);
        }
        return baos.toByteArray();
    }

    private static byte[] buildRSAPublicKeyBlob(RSAPublicKey pub) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try (DataOutputStream dos = new DataOutputStream(baos)) {
            writeString(dos, "ssh-rsa");
            writeMPInt(dos, BigInteger.valueOf(pub.getPublicExponent().longValue()));
            writeMPInt(dos, pub.getModulus());
        }
        return baos.toByteArray();
    }

    private static byte[] buildRSAPrivateBlock(RSAPrivateCrtKey priv, RSAPublicKey pub, String comment, SecureRandom sr) throws IOException {
        ByteArrayOutputStream privBaos = new ByteArrayOutputStream();
        try (DataOutputStream dos = new DataOutputStream(privBaos)) {
            int check = sr.nextInt();
            dos.writeInt(check);
            dos.writeInt(check);
            writeString(dos, "ssh-rsa");
            // Public parameters
            writeMPInt(dos, priv.getModulus()); // n
            writeMPInt(dos, BigInteger.valueOf(pub.getPublicExponent().longValue())); // e
            // Private parameters
            writeMPInt(dos, priv.getPrivateExponent()); // d
            writeMPInt(dos, priv.getCrtCoefficient()); // iqmp = q^{-1} mod p
            writeMPInt(dos, priv.getPrimeP()); // p
            writeMPInt(dos, priv.getPrimeQ()); // q
            writeString(dos, comment);

            // Padding to block size (8 bytes for cipher "none")
            int blockSize = 8;
            int len = privBaos.size();
            int padNeeded = blockSize - (len % blockSize);
            if (padNeeded == blockSize) {
                padNeeded = 0;
            }
            for (int i = 1; i <= padNeeded; i++) {
                dos.writeByte(i & 0xFF);
            }
        }
        return privBaos.toByteArray();
    }

    private static void writeString(DataOutputStream dos, String s) throws IOException {
        byte[] bytes = s.getBytes(StandardCharsets.UTF_8);
        writeString(dos, bytes);
    }

    private static void writeString(DataOutputStream dos, byte[] bytes) throws IOException {
        dos.writeInt(bytes.length);
        dos.write(bytes);
    }

    private static void writeMPInt(DataOutputStream dos, BigInteger bi) throws IOException {
        if (bi.equals(BigInteger.ZERO)) {
            dos.writeInt(0);
            return;
        }
        byte[] bytes = bi.toByteArray();
        // Remove unnecessary leading zero if present
        if (bytes.length > 1 && bytes[0] == 0) {
            bytes = Arrays.copyOfRange(bytes, 1, bytes.length);
        }
        // If MSB is set, prepend 0x00 to indicate positive number
        if ((bytes[0] & 0x80) != 0) {
            byte[] prefixed = new byte[bytes.length + 1];
            System.arraycopy(bytes, 0, prefixed, 1, bytes.length);
            bytes = prefixed; // first byte is 0 by default
        }
        dos.writeInt(bytes.length);
        dos.write(bytes);
    }

    public static void main(String[] args) {
        try {
            // 5 test cases
            String k1 = generateOpenSSHRSAPrivateKey(3072, "user1@example.com");
            String k2 = generateOpenSSHRSAPrivateKey(4096, "Task78-key-1");
            String k3 = generateOpenSSHRSAPrivateKey(3072, "no comment");
            String k4 = generateOpenSSHRSAPrivateKey(2048, "legacy-compat");
            String k5 = generateOpenSSHRSAPrivateKey(3072, "another-key");

            System.out.println(k1);
            System.out.println(k2);
            System.out.println(k3);
            System.out.println(k4);
            System.out.println(k5);
        } catch (Exception e) {
            // Minimal error output to avoid leaking sensitive details
            System.err.println("Key generation failed: " + e.getMessage());
        }
    }
}