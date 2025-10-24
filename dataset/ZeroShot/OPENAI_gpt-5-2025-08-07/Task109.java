import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Task109 {

    private static final int MAX_USERNAME_LEN = 32;
    private static final int MIN_USERNAME_LEN = 3;

    private static final int MAX_EMAIL_LEN = 320; // generous upper bound
    private static final int MAX_PHONE_LEN = 16;  // E.164 max is 15 digits plus '+'
    private static final int MIN_PASSWORD_LEN = 8;
    private static final int MAX_PASSWORD_LEN = 128;
    private static final int MAX_ZIP_LEN = 10;

    private static final Pattern USERNAME = Pattern.compile("^[A-Za-z0-9_]{3,16}$");
    private static final Pattern EMAIL = Pattern.compile("^[A-Za-z0-9_%+.-]+(\\.[A-Za-z0-9_%+.-]+)*@[A-Za-z0-9-]+(\\.[A-Za-z0-9-]+)*\\.[A-Za-z]{2,24}$");
    private static final Pattern PHONE_E164 = Pattern.compile("^\\+[1-9][0-9]{1,14}$");
    private static final Pattern ZIP_US = Pattern.compile("^[0-9]{5}(-[0-9]{4})?$");

    private static final Pattern P_UPPER = Pattern.compile("[A-Z]");
    private static final Pattern P_LOWER = Pattern.compile("[a-z]");
    private static final Pattern P_DIGIT = Pattern.compile("[0-9]");
    private static final Pattern P_SPECIAL = Pattern.compile("[^A-Za-z0-9]");

    private static boolean safeFullMatch(Pattern p, String s, int maxLen) {
        if (s == null) return false;
        if (s.length() > maxLen) return false;
        Matcher m = p.matcher(s);
        return m.matches();
    }

    public static boolean validateUsername(String username) {
        if (username == null) return false;
        if (username.length() < MIN_USERNAME_LEN || username.length() > MAX_USERNAME_LEN) return false;
        return safeFullMatch(USERNAME, username, MAX_USERNAME_LEN);
    }

    public static boolean validateEmail(String email) {
        if (email == null) return false;
        if (email.length() > MAX_EMAIL_LEN) return false;
        return safeFullMatch(EMAIL, email, MAX_EMAIL_LEN);
    }

    public static boolean validatePhoneE164(String phone) {
        if (phone == null) return false;
        return safeFullMatch(PHONE_E164, phone, MAX_PHONE_LEN);
    }

    public static boolean validatePassword(String password) {
        if (password == null) return false;
        int len = password.length();
        if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) return false;
        if (!P_UPPER.matcher(password).find()) return false;
        if (!P_LOWER.matcher(password).find()) return false;
        if (!P_DIGIT.matcher(password).find()) return false;
        if (!P_SPECIAL.matcher(password).find()) return false;
        return true;
    }

    public static boolean validateUSZip(String zip) {
        if (zip == null) return false;
        return safeFullMatch(ZIP_US, zip, MAX_ZIP_LEN);
    }

    public static void main(String[] args) {
        // 5 test cases
        System.out.println(validateUsername("user_name12"));            // true
        System.out.println(validateEmail("john.doe@example.co.uk"));    // true
        System.out.println(validatePhoneE164("+14155552671"));          // true
        System.out.println(validatePassword("Str0ng!Pass"));            // true
        System.out.println(validateUSZip("12345-6789"));                // true
    }
}