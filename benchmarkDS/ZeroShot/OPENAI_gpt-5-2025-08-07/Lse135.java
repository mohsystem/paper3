import java.nio.charset.StandardCharsets;

public class Lse135 {
    private static final String STORED_USER = "admin";
    private static final String STORED_PASS = "S3cureP@ss!";

    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] ab = a.getBytes(StandardCharsets.UTF_8);
        byte[] bb = b.getBytes(StandardCharsets.UTF_8);
        int max = Math.max(ab.length, bb.length);
        int diff = ab.length ^ bb.length;
        for (int i = 0; i < max; i++) {
            byte x = i < ab.length ? ab[i] : 0;
            byte y = i < bb.length ? bb[i] : 0;
            diff |= (x ^ y);
        }
        return diff == 0;
    }

    public static String check_mod(String username, String password) {
        boolean userOk = constantTimeEquals(username == null ? "" : username, STORED_USER);
        boolean passOk = constantTimeEquals(password == null ? "" : password, STORED_PASS);
        return (userOk && passOk) ? "true" : "false";
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(check_mod("admin", "S3cureP@ss!")); // expected true
        System.out.println(check_mod("admin", "wrong"));       // expected false
        System.out.println(check_mod("user", "S3cureP@ss!"));  // expected false
        System.out.println(check_mod("", ""));                 // expected false
        System.out.println(check_mod("admin", "S3cureP@ss!!"));// expected false
    }
}