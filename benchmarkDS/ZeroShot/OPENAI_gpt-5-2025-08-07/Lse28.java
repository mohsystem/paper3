import java.util.regex.Pattern;

public class Lse28 {
    private static final Pattern ALLOWED = Pattern.compile("^https://(www\\.)?example\\.com(/[A-Za-z0-9._~\\-/%?#&=]*)?$");
    private static final int MAX_LEN = 2048;
    private static final String SAFE_FALLBACK = "https://www.example.com/";

    private static boolean hasCtl(String s) {
        for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            if (ch < 0x20 || ch == 0x7F) {
                return true;
            }
        }
        return false;
    }

    private static String sanitizeDefault(String def) {
        if (def == null) return SAFE_FALLBACK;
        if (def.length() == 0 || def.length() > MAX_LEN) return SAFE_FALLBACK;
        if (hasCtl(def)) return SAFE_FALLBACK;
        if (!def.startsWith("https://")) return SAFE_FALLBACK;
        return def;
    }

    public static String resolveRedirect(String targetUrl, String defaultUrl) {
        String safeDefault = sanitizeDefault(defaultUrl);
        if (targetUrl == null) return safeDefault;
        if (targetUrl.length() == 0 || targetUrl.length() > MAX_LEN) return safeDefault;
        if (hasCtl(targetUrl)) return safeDefault;
        if (ALLOWED.matcher(targetUrl).matches()) {
            return targetUrl;
        }
        return safeDefault;
    }

    public static void main(String[] args) {
        String defaultUrl = "https://www.example.com/safe";
        String[] tests = new String[] {
            "https://www.example.com/path?x=1",      // allowed
            "https://evil.com/",                     // wrong domain
            "http://www.example.com/",               // wrong scheme
            null,                                    // null
            "https://www.example.com/\r\nattack"     // CRLF injection attempt
        };
        for (int i = 0; i < tests.length; i++) {
            String result = resolveRedirect(tests[i], defaultUrl);
            System.out.println("Test " + (i+1) + ": " + result);
        }
    }
}