public class Task55 {
    public static boolean isValidEmail(String email) {
        if (email == null) return false;
        int len = email.length();
        if (len == 0 || len > 254) return false;

        int atIndex = email.indexOf('@');
        if (atIndex <= 0) return false;
        if (email.indexOf('@', atIndex + 1) != -1) return false;

        String local = email.substring(0, atIndex);
        String domain = email.substring(atIndex + 1);

        if (local.length() == 0 || domain.length() == 0) return false;
        if (local.length() > 64) return false;

        if (local.startsWith(".") || local.endsWith(".") || local.contains("..")) return false;
        for (int i = 0; i < local.length(); i++) {
            char c = local.charAt(i);
            if (!isLocalAllowed(c)) return false;
        }

        if (domain.startsWith(".") || domain.endsWith(".") || domain.contains("..")) return false;
        for (int i = 0; i < domain.length(); i++) {
            char c = domain.charAt(i);
            if (!(isAlpha(c) || isDigit(c) || c == '-' || c == '.')) return false;
        }

        if (domain.indexOf('.') == -1) return false;

        String[] labels = domain.split("\\.");
        if (labels.length < 2) return false;

        for (String label : labels) {
            if (label.length() == 0 || label.length() > 63) return false;
            if (label.charAt(0) == '-' || label.charAt(label.length() - 1) == '-') return false;
            for (int i = 0; i < label.length(); i++) {
                char c = label.charAt(i);
                if (!(isAlpha(c) || isDigit(c) || c == '-')) return false;
            }
        }

        String tld = labels[labels.length - 1];
        if (tld.length() < 2) return false;
        for (int i = 0; i < tld.length(); i++) {
            if (!isAlpha(tld.charAt(i))) return false;
        }

        return true;
    }

    private static boolean isAlpha(char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

    private static boolean isDigit(char c) {
        return (c >= '0' && c <= '9');
    }

    private static boolean isLocalAllowed(char c) {
        return isAlpha(c) || isDigit(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "user.name+tag@example.com",
            "user@sub.example.co",
            "user..name@example.com",
            "user@-example.com",
            "user@example"
        };

        for (String email : tests) {
            System.out.println(email + " -> " + isValidEmail(email));
        }
    }
}