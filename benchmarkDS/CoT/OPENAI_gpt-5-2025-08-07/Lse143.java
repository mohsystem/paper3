import java.util.*;

public class Lse143 {
    private static final Set<String> database = new HashSet<>();
    private static final Set<String> mailingList = new HashSet<>();

    public static void initializeDB() {
        database.clear();
        mailingList.clear();
        List<String> initial = Arrays.asList(
            "alice@example.com",
            "bob@example.com",
            "dana@example.com"
        );
        database.addAll(initial);
        mailingList.addAll(initial);
    }

    public static String unsubscribe(String email) {
        if (email == null || email.isEmpty()) {
            return "Invalid email";
        }
        if (database.contains(email)) {
            database.remove(email);
            mailingList.remove(email);
            return "Unsubscribed: " + email;
        } else {
            return "User not subscribed: " + email;
        }
    }

    public static void main(String[] args) {
        initializeDB();
        String[] tests = new String[] {
            "alice@example.com",
            "bob@example.com",
            "charlie@example.com",
            "bob@example.com",
            "dana@example.com"
        };
        for (String email : tests) {
            System.out.println(unsubscribe(email));
        }
    }
}