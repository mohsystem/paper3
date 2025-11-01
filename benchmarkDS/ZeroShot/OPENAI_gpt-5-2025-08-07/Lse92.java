import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.SecureRandom;
import java.util.Arrays;

public class Lse92 {
    private static final String ALG = "PBKDF2WithHmacSHA256";
    private static final int ITERATIONS = 200000;
    private static final int SALT_LEN = 16;
    private static final int DK_LEN = 32;

    public static String registerUser(String username, String password) throws Exception {
        if (username == null) throw new IllegalArgumentException("username cannot be null");
        if (password == null) throw new IllegalArgumentException("password cannot be null");
        byte[] salt = new byte[SALT_LEN];
        SecureRandom sr = SecureRandom.getInstanceStrong();
        sr.nextBytes(salt);

        char[] pwdChars = password.toCharArray();
        byte[] hash = pbkdf2(pwdChars, salt, ITERATIONS, DK_LEN);
        // wipe password chars
        Arrays.fill(pwdChars, '\0');

        String result = username + ":PBKDF2$SHA256$" + ITERATIONS + "$" + toHex(salt) + "$" + toHex(hash);
        Arrays.fill(hash, (byte) 0);
        Arrays.fill(salt, (byte) 0);
        return result;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance(ALG);
        return skf.generateSecret(spec).getEncoded();
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(String.format("%02x", b & 0xff));
        }
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        String[] users = {"alice", "bob", "charlie", "dana", "eve"};
        String[] pwds  = {"Str0ng!Pass#1", "Another$ecret2", "Pa55_word^3", "Secur3Key&4", "MyP@ssw0rd_5"};
        for (int i = 0; i < users.length; i++) {
            String out = registerUser(users[i], pwds[i]);
            System.out.println(out);
        }
    }
}