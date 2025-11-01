import java.util.Objects;

public final class Lse53 {
    private Lse53() {}

    private static final String LOGIN_PATH = "/login";
    private static final String USER_PATH = "/user";
    private static final String ADMIN_PATH = "/admin";

    public static String secureRedirect(Boolean isLoggedIn, Boolean isAdmin) {
        if (isLoggedIn == null || isAdmin == null) {
            return LOGIN_PATH;
        }
        if (!Boolean.TRUE.equals(isLoggedIn)) {
            return LOGIN_PATH;
        }
        if (Boolean.TRUE.equals(isAdmin)) {
            return ADMIN_PATH;
        }
        return USER_PATH;
    }

    public static void main(String[] args) {
        System.out.println(secureRedirect(Boolean.FALSE, Boolean.FALSE)); // /login
        System.out.println(secureRedirect(Boolean.TRUE, Boolean.FALSE));  // /user
        System.out.println(secureRedirect(Boolean.TRUE, Boolean.TRUE));   // /admin
        System.out.println(secureRedirect(Boolean.FALSE, Boolean.TRUE));  // /login
        System.out.println(secureRedirect(null, Boolean.TRUE));           // /login
    }
}