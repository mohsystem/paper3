import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Task82 {
    public static String hashPasswordWithSalt(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] input = (salt + password).getBytes(StandardCharsets.UTF_8);
            byte[] digest = md.digest(input);
            StringBuilder sb = new StringBuilder(digest.length * 2);
            for (byte b : digest) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        } catch (NoSuchAlgorithmException e) {
            return null;
        }
    }

    public static void main(String[] args) {
        String[][] tests = {
            {"password123", "NaCl"},
            {"hunter2", "S@1t!"},
            {"Tr0ub4dor&3", "pepper"},
            {"correcthorsebatterystaple", ""},
            {"", "onlysalt"}
        };
        for (String[] t : tests) {
            String out = hashPasswordWithSalt(t[0], t[1]);
            System.out.println("password=\"" + t[0] + "\", salt=\"" + t[1] + "\" -> " + out);
        }
    }
}