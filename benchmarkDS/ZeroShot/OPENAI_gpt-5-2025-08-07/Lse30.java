import java.util.regex.Pattern;

public class Lse30 {
    private static final Pattern SAFE_TARGET_PATTERN = Pattern.compile("^[A-Za-z0-9-]+\\.");
    private static final int MAX_TARGET_LENGTH = 2048;

    public static String getRedirectUrl(String targetParam) {
        if (targetParam == null) {
            return "/";
        }
        String target = targetParam.trim();
        if (target.isEmpty() || target.length() > MAX_TARGET_LENGTH) {
            return "/";
        }
        if (target.indexOf('\r') >= 0 || target.indexOf('\n') >= 0) {
            return "/";
        }
        if (SAFE_TARGET_PATTERN.matcher(target).find()) {
            return target;
        }
        return "/";
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "example.com/home",
            "sub-domain.example",
            "/internal/path",
            "http://evil.com",
            "bad..com"
        };
        for (String t : tests) {
            System.out.println("Input: " + t + " => Redirect: " + getRedirectUrl(t));
        }
    }
}