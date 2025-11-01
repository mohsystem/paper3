import io.javalin.Javalin;
import io.javalin.http.Context;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

// Add the following dependencies to your pom.xml or build.gradle:
// <dependency>
//     <groupId>io.javalin</groupId>
//     <artifactId>javalin</artifactId>
//     <version>5.6.3</version>
// </dependency>
// <dependency>
//     <groupId>org.slf4j</groupId>
//     <artifactId>slf4j-simple</artifactId>
//     <version>2.0.7</version>
// </dependency>
public class Lse52 {

    // In-memory session store (for demonstration purposes)
    private static final Map<String, String> sessions = new ConcurrentHashMap<>();
    // Hardcoded user credentials
    private static final Map<String, String> users = Map.of("admin", "password");

    public static void main(String[] args) {
        Javalin app = Javalin.create().start(7070);

        app.get("/", Lse52::handleIndex);
        app.get("/login", Lse52::handleLoginGet);
        app.post("/login", Lse52::handleLoginPost);
        app.get("/secret", Lse52::handleSecret);

        System.out.println("Server started on http://localhost:7070");
        printTestInstructions(7070);
    }

    private static void handleIndex(Context ctx) {
        ctx.html("<h1>Welcome!</h1><a href='/login'>Login</a>");
    }

    private static void handleLoginGet(Context ctx) {
        String html = "<h1>Login</h1>" +
                      "<form action='/login' method='post'>" +
                      "  <label for='username'>Username:</label><br>" +
                      "  <input type='text' id='username' name='username'><br>" +
                      "  <label for='password'>Password:</label><br>" +
                      "  <input type='password' id='password' name='password'><br><br>" +
                      "  <input type='submit' value='Submit'>" +
                      "</form>";
        ctx.html(html);
    }

    private static void handleLoginPost(Context ctx) {
        String username = ctx.formParam("username");
        String password = ctx.formParam("password");

        if (users.containsKey(username) && users.get(username).equals(password)) {
            String sessionId = UUID.randomUUID().toString();
            sessions.put(sessionId, username);
            ctx.cookie("session-id", sessionId);
            ctx.redirect("/secret");
        } else {
            ctx.redirect("/login"); // Optional: add error message
        }
    }

    private static void handleSecret(Context ctx) {
        String sessionId = ctx.cookie("session-id");
        if (sessionId != null && sessions.containsKey(sessionId)) {
            String username = sessions.get(sessionId);
            ctx.html("<h1>Secret settings</h1><p>Welcome, " + username + "!</p>");
        } else {
            ctx.redirect("/login");
        }
    }

    private static void printTestInstructions(int port) {
        String baseUrl = "http://localhost:" + port;
        System.out.println("\n--- 5 TEST CASES ---");
        System.out.println("To test, please perform the following actions in your web browser:");
        System.out.println("1. Visit Index Page: Go to " + baseUrl + "/");
        System.out.println("   Expected: See 'Welcome!' and a login link.");
        System.out.println("\n2. Access Secret Page (Not Logged In): Go to " + baseUrl + "/secret");
        System.out.println("   Expected: Redirected to the login page.");
        System.out.println("\n3. Login with Incorrect Credentials: Go to " + baseUrl + "/login and enter 'user'/'wrongpass'.");
        System.out.println("   Expected: Redirected back to the login page.");
        System.out.println("\n4. Login with Correct Credentials: On the login page, enter 'admin'/'password'.");
        System.out.println("   Expected: Redirected to the secret page, see 'Secret settings'.");
        System.out.println("\n5. Access Secret Page (Logged In): After logging in, refresh or revisit " + baseUrl + "/secret");
        System.out.println("   Expected: Still see 'Secret settings' page.");
    }
}