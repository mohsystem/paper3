import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.math.BigInteger;
import java.security.interfaces.RSAPrivateCrtKey;
import java.util.Base64;

public class Task78 {

    public static String generateOpenSSHRSAPrivateKey(int keySize, String comment) throws Exception {
        if (keySize < 2048 || keySize > 8192) {
            throw new IllegalArgumentException("keySize must be between 2048 and 8192 bits");
        }
        if (comment == null) comment = "";
        if (comment.length() > 1024) {
            throw new IllegalArgumentException("comment too long");
        }

        // Generate RSA keypair
        KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA");
        kpg.initialize(keySize, SecureRandom.getInstanceStrong());
        KeyPair kp = kpg.generateKeyPair();
        RSAPrivateCrtKey priv = (RSAPrivateCrtKey) kp.getPrivate();

        BigInteger n = priv.getModulus();
        BigInteger e = priv.getPublicExponent();
        BigInteger d = priv.getPrivateExponent();
        BigInteger p = priv.getPrimeP();
        BigInteger q = priv.getPrimeQ();
        BigInteger iqmp = priv.getCrtCoefficient();

        // Build public key blob: string "ssh-rsa", mpint e, mpint n
        byte[] pubBlob = buildPublicKeyBlob(e, n);

        // Build private key block
        byte[] privBlock = buildPrivateKeyBlock(n, e, d, iqmp, p, q, comment);

        // Build final openssh-key-v1 structure
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write("openssh-key-v1\0".getBytes(StandardCharsets.UTF_8));
        writeString(out, "none".getBytes(StandardCharsets.UTF_8)); // ciphername
        writeString(out, "none".getBytes(StandardCharsets.UTF_8)); // kdfname
        writeString(out, new byte[0]);                              // kdfoptions
        writeUInt32(out, 1);                                        // number of keys
        writeString(out, pubBlob);                                  // public keys
        writeString(out, privBlock);                                // private key block

        byte[] finalBytes = out.toByteArray();
        String b64 = Base64.getMimeEncoder(70, new byte[]{'\n'}).encodeToString(finalBytes);
        String header = "-----BEGIN OPENSSH PRIVATE KEY-----\n";
        String footer = "-----END OPENSSH PRIVATE KEY-----\n";
        return header + b64 + "\n" + footer;
    }

    private static byte[] buildPublicKeyBlob(BigInteger e, BigInteger n) throws Exception {
        ByteArrayOutputStream buf = new ByteArrayOutputStream();
        writeString(buf, "ssh-rsa".getBytes(StandardCharsets.UTF_8));
        writeMpint(buf, e);
        writeMpint(buf, n);
        return buf.toByteArray();
    }

    private static byte[] buildPrivateKeyBlock(BigInteger n, BigInteger e, BigInteger d, BigInteger iqmp, BigInteger p, BigInteger q, String comment) throws Exception {
        ByteArrayOutputStream buf = new ByteArrayOutputStream();
        SecureRandom sr = SecureRandom.getInstanceStrong();
        int check = sr.nextInt();
        writeUInt32(buf, check);
        writeUInt32(buf, check);
        writeString(buf, "ssh-rsa".getBytes(StandardCharsets.UTF_8));
        writeMpint(buf, n);
        writeMpint(buf, e);
        writeMpint(buf, d);
        writeMpint(buf, iqmp);
        writeMpint(buf, p);
        writeMpint(buf, q);
        writeString(buf, comment.getBytes(StandardCharsets.UTF_8));

        // Padding to multiple of 8 bytes
        int padLen = 8 - (buf.size() % 8);
        if (padLen == 0) padLen = 8;
        for (int i = 1; i <= padLen; i++) {
            buf.write(i & 0xFF);
        }
        return buf.toByteArray();
    }

    private static void writeUInt32(ByteArrayOutputStream out, int v) {
        byte[] b = ByteBuffer.allocate(4).putInt(v).array();
        out.write(b, 0, 4);
    }

    private static void writeString(ByteArrayOutputStream out, byte[] data) {
        writeUInt32(out, data.length);
        out.write(data, 0, data.length);
    }

    private static void writeMpint(ByteArrayOutputStream out, BigInteger bi) {
        if (bi.signum() == 0) {
            writeUInt32(out, 0);
            return;
        }
        byte[] arr = bi.toByteArray(); // two's complement big-endian
        int off = 0;
        while (off < arr.length - 1 && arr[off] == 0) {
            off++;
        }
        int len = arr.length - off;
        boolean needZero = (arr[off] & 0x80) != 0;
        if (needZero) {
            writeUInt32(out, len + 1);
            out.write(0);
        } else {
            writeUInt32(out, len);
        }
        out.write(arr, off, len);
    }

    public static void main(String[] args) {
        try {
            String[] comments = {"test1", "user@example", "device-key", "backup", "prod-key"};
            int[] sizes = {2048, 2048, 3072, 4096, 2048};
            for (int i = 0; i < 5; i++) {
                String key = generateOpenSSHRSAPrivateKey(sizes[i], comments[i]);
                boolean headerOk = key.startsWith("-----BEGIN OPENSSH PRIVATE KEY-----");
                System.out.println("Case " + (i + 1) + ": generated=" + headerOk + ", size=" + sizes[i] + ", length=" + key.length());
            }
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}