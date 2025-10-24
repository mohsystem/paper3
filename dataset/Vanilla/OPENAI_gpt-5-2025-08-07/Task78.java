import java.io.ByteArrayOutputStream;
import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.interfaces.RSAPrivateCrtKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Base64;

public class Task78 {

    public static String generateOpenSSHRSAPrivateKey(int bits, String comment) throws Exception {
        KeyPairGenerator kpg = KeyPairGenerator.getInstance("RSA");
        kpg.initialize(bits, new SecureRandom());
        KeyPair kp = kpg.generateKeyPair();

        RSAPublicKey pub = (RSAPublicKey) kp.getPublic();
        RSAPrivateCrtKey prv = (RSAPrivateCrtKey) kp.getPrivate();

        BigInteger n = pub.getModulus();
        BigInteger e = BigInteger.valueOf(pub.getPublicExponent().longValue());
        BigInteger d = prv.getPrivateExponent();
        BigInteger p = prv.getPrimeP();
        BigInteger q = prv.getPrimeQ();
        BigInteger iqmp = prv.getCrtCoefficient(); // q^{-1} mod p

        byte[] pubKeyBlob = buildPublicKeyBlob(n, e);
        byte[] privKeyBlob = buildPrivateKeyBlob(n, e, d, iqmp, p, q, comment);

        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write("openssh-key-v1\0".getBytes(StandardCharsets.UTF_8));
        out.write(encodeString("none".getBytes(StandardCharsets.UTF_8))); // ciphername
        out.write(encodeString("none".getBytes(StandardCharsets.UTF_8))); // kdfname
        out.write(encodeString(new byte[0])); // kdfoptions
        out.write(uint32(1)); // number of keys
        out.write(encodeString(pubKeyBlob)); // public key
        out.write(encodeString(privKeyBlob)); // private key block

        byte[] finalData = out.toByteArray();
        String b64 = Base64.getEncoder().encodeToString(finalData);
        String wrapped = wrapBase64(b64, 70);

        StringBuilder sb = new StringBuilder();
        sb.append("-----BEGIN OPENSSH PRIVATE KEY-----\n");
        sb.append(wrapped);
        sb.append("\n-----END OPENSSH PRIVATE KEY-----\n");
        return sb.toString();
    }

    private static byte[] buildPublicKeyBlob(BigInteger n, BigInteger e) throws Exception {
        ByteArrayOutputStream pub = new ByteArrayOutputStream();
        pub.write(encodeString("ssh-rsa".getBytes(StandardCharsets.UTF_8)));
        pub.write(encodeMPInt(e));
        pub.write(encodeMPInt(n));
        return pub.toByteArray();
    }

    private static byte[] buildPrivateKeyBlob(BigInteger n, BigInteger e, BigInteger d, BigInteger iqmp, BigInteger p, BigInteger q, String comment) throws Exception {
        ByteArrayOutputStream priv = new ByteArrayOutputStream();
        SecureRandom rnd = new SecureRandom();
        int check = rnd.nextInt();
        priv.write(uint32(check));
        priv.write(uint32(check));
        priv.write(encodeString("ssh-rsa".getBytes(StandardCharsets.UTF_8)));
        priv.write(encodeMPInt(n));
        priv.write(encodeMPInt(e));
        priv.write(encodeMPInt(d));
        priv.write(encodeMPInt(iqmp));
        priv.write(encodeMPInt(p));
        priv.write(encodeMPInt(q));
        priv.write(encodeString(comment.getBytes(StandardCharsets.UTF_8)));

        // Padding to block size (8) with 1..N
        int blockSize = 8;
        int padLen = blockSize - (priv.size() % blockSize);
        if (padLen == blockSize) padLen = 0;
        for (int i = 1; i <= padLen; i++) {
            priv.write(i);
        }
        return priv.toByteArray();
    }

    private static byte[] encodeString(byte[] data) throws Exception {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        out.write(uint32(data.length));
        out.write(data);
        return out.toByteArray();
    }

    private static byte[] encodeMPInt(BigInteger bi) throws Exception {
        if (bi.equals(BigInteger.ZERO)) {
            return encodeString(new byte[0]);
        }
        byte[] raw = bi.toByteArray(); // two's complement big-endian
        // Ensure minimal and non-negative representation per SSH spec
        if (raw[0] == 0 && raw.length > 1) {
            // remove leading zero if unnecessary
            int i = 0;
            while (i < raw.length - 1 && raw[i] == 0 && (raw[i + 1] & 0x80) == 0) i++;
            byte[] trimmed = new byte[raw.length - i];
            System.arraycopy(raw, i, trimmed, 0, trimmed.length);
            raw = trimmed;
        } else if ((raw[0] & 0x80) != 0) {
            // add leading zero to keep positive
            byte[] prefixed = new byte[raw.length + 1];
            System.arraycopy(raw, 0, prefixed, 1, raw.length);
            raw = prefixed;
        }
        return encodeString(raw);
    }

    private static byte[] uint32(int v) {
        return new byte[] {
            (byte)((v >>> 24) & 0xFF),
            (byte)((v >>> 16) & 0xFF),
            (byte)((v >>> 8) & 0xFF),
            (byte)(v & 0xFF)
        };
    }

    private static String wrapBase64(String s, int width) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < s.length(); i += width) {
            int end = Math.min(i + width, s.length());
            sb.append(s, i, end);
            if (end < s.length()) sb.append('\n');
        }
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        System.out.println(generateOpenSSHRSAPrivateKey(2048, "java-test-1"));
        System.out.println(generateOpenSSHRSAPrivateKey(1024, "java-test-2"));
        System.out.println(generateOpenSSHRSAPrivateKey(3072, "java-test-3"));
        System.out.println(generateOpenSSHRSAPrivateKey(4096, "java-test-4"));
        System.out.println(generateOpenSSHRSAPrivateKey(1536, "java-test-5"));
    }
}