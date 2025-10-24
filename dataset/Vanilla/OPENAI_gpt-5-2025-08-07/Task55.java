import java.util.*;

public class Task55 {

    public static boolean isValidEmail(String email) {
        if (email == null) return false;
        int len = email.length();
        if (len == 0 || len > 254) return false;

        int atFirst = email.indexOf('@');
        int atLast = email.lastIndexOf('@');
        if (atFirst <= 0 || atFirst != atLast || atFirst == len - 1) return false;

        String local = email.substring(0, atFirst);
        String domain = email.substring(atFirst + 1);

        if (local.length() == 0 || local.length() > 64) return false;
        // Local part rules
        if (local.charAt(0) == '.' || local.charAt(local.length() - 1) == '.') return false;
        char prev = '\0';
        for (int i = 0; i < local.length(); i++) {
            char c = local.charAt(i);
            boolean allowed = Character.isLetterOrDigit(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
            if (!allowed) return false;
            if (c == '.' && prev == '.') return false;
            prev = c;
        }

        // Domain rules
        if (domain.length() == 0) return false;
        String[] labels = domain.split("\\.", -1);
        if (labels.length < 2) return false;
        for (String label : labels) {
            if (label.length() == 0 || label.length() > 63) return false;
            if (!Character.isLetterOrDigit(label.charAt(0)) || !Character.isLetterOrDigit(label.charAt(label.length() - 1)))
                return false;
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                if (!(Character.isLetterOrDigit(c) || c == '-')) return false;
            }
        }
        String tld = labels[labels.length - 1];
        if (tld.length() < 2) return false;
        for (int i = 0; i < tld.length(); i++) {
            if (!Character.isLetter(tld.charAt(i))) return false;
        }
        return true;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "user@example.com",
            "first.last+tag@sub.domain.co",
            ".user@domain.com",
            "user@-domain.com",
            "user@domain"
        };
        for (String e : tests) {
            System.out.println(e + " => " + isValidEmail(e));
        }
    }
}