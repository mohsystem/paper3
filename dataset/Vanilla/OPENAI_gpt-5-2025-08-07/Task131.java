import java.util.*;

public class Task131 {
    public static class User {
        public String name;
        public int age;
        public String email;

        public User() {
            this.name = "";
            this.age = -1;
            this.email = "";
        }

        public User(String name, int age, String email) {
            this.name = name;
            this.age = age;
            this.email = email;
        }

        @Override
        public String toString() {
            return "User{name='" + name + "', age=" + age + ", email='" + email + "'}";
        }
    }

    public static User deserializeUser(String input) {
        User user = new User();
        if (input == null) return user;

        String[] tokens = input.split(";");
        for (String token : tokens) {
            if (token == null) continue;
            int idx = token.indexOf('=');
            if (idx < 0) continue;
            String key = token.substring(0, idx).trim().toLowerCase(Locale.ROOT);
            String value = token.substring(idx + 1).trim();
            switch (key) {
                case "name":
                    user.name = value;
                    break;
                case "age":
                    try {
                        user.age = Integer.parseInt(value);
                    } catch (NumberFormatException e) {
                        user.age = -1;
                    }
                    break;
                case "email":
                    user.email = value;
                    break;
                default:
                    // ignore unknown fields
                    break;
            }
        }
        return user;
    }

    public static void main(String[] args) {
        String[] tests = new String[] {
            "name=Alice; age=30; email=alice@example.com",
            "age=25;name=Bob;email=bob@domain.org",
            "name=Charlie;email=charlie@mail.net",
            "name = Dana ; age = notanumber ; email = dana@x.y",
            "email=eve@ex.com; name=Eve; extra=ignored; age= 42 ;"
        };

        for (String t : tests) {
            User u = deserializeUser(t);
            System.out.println("Input: " + t);
            System.out.println("Deserialized: " + u);
            System.out.println("---");
        }
    }
}