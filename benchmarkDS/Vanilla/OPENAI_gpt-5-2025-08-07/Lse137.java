import java.util.*;

public class Lse137 {
    public static boolean auth_mod(String username, String password, String session) {
        if (username == null || password == null || session == null) return false;
        String expectedPass = null;
        if ("alice".equals(username)) expectedPass = "alice123";
        else if ("bob".equals(username)) expectedPass = "bob123";
        else if ("carol".equals(username)) expectedPass = "carol123";
        if (expectedPass == null) return false;
        if (!password.equals(expectedPass)) return false;
        if (!"sessOK".equals(session)) return false;
        return "alice".equals(username) || "bob".equals(username);
    }

    public static boolean check_mod(String url) {
        String username = getParam(url, "username");
        String password = getParam(url, "password");
        String session = getParam(url, "session");
        return auth_mod(username, password, session);
    }

    private static String getParam(String url, String key) {
        if (url == null) return null;
        int q = url.indexOf('?');
        if (q < 0 || q + 1 >= url.length()) return null;
        String qs = url.substring(q + 1);
        String[] pairs = qs.split("&");
        for (String pair : pairs) {
            int eq = pair.indexOf('=');
            if (eq <= 0) continue;
            String k = pair.substring(0, eq);
            String v = pair.substring(eq + 1);
            if (k.equals(key)) return v;
        }
        return null;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "/check_mod?username=alice&password=alice123&session=sessOK",
            "/check_mod?username=bob&password=bob123&session=sessOK",
            "/check_mod?username=carol&password=carol123&session=sessOK",
            "/check_mod?username=alice&password=wrong&session=sessOK",
            "/check_mod?username=alice&password=alice123&session=bad"
        };
        for (String t : tests) {
            System.out.println(check_mod(t));
        }
    }
}