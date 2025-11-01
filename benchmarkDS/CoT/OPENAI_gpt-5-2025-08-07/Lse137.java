import java.util.*;

public class Lse137 {

    public static boolean auth_mod(String username, String password, String session) {
        String u = username == null ? "" : username;
        String p = password == null ? "" : password;
        String s = session == null ? "" : session;
        return u.startsWith("mod_") && "secret".equals(p) && "valid".equals(s);
    }

    public static boolean check_mod(String username, String password, String session) {
        return auth_mod(username, password, session);
    }

    public static void main(String[] args) {
        String[][] tests = new String[][]{
            {"mod_alice", "secret", "valid"},
            {"user_bob", "secret", "valid"},
            {"mod_carol", "wrong", "valid"},
            {"mod_dan", "secret", "expired"},
            {"", "", ""}
        };
        for (int i = 0; i < tests.length; i++) {
            boolean res = check_mod(tests[i][0], tests[i][1], tests[i][2]);
            System.out.println("Test " + (i + 1) + ": " + res);
        }
    }
}