import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;

public class Task82 {
    public static String hashPasswordWithSalt(String password, String salt) throws GeneralSecurityException {
        if (password == null || salt == null) {
            throw new IllegalArgumentException("password and salt must not be null");
        }
        Mac mac = Mac.getInstance("HmacSHA256");
        SecretKeySpec keySpec = new SecretKeySpec(salt.getBytes(StandardCharsets.UTF_8), "HmacSHA256");
        mac.init(keySpec);
        byte[] digest = mac.doFinal(password.getBytes(StandardCharsets.UTF_8));
        return toHex(digest);
    }

    private static String toHex(byte[] data) {
        char[] hexArray = "0123456789abcdef".toCharArray();
        char[] hexChars = new char[data.length * 2];
        for (int j = 0; j < data.length; j++) {
            int v = data[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    public static void main(String[] args) throws Exception {
        String[][] tests = new String[][] {
            {"password123", "NaCl"},
            {"correcthorsebatterystaple", "pepper"},
            {"", "salt"},
            {"p@ssw0rd!#%", "S@1t"},
            {"unicode-パスワード", "ソルト"}
        };
        for (String[] t : tests) {
            String result = hashPasswordWithSalt(t[0], t[1]);
            System.out.println(result);
        }
    }
}