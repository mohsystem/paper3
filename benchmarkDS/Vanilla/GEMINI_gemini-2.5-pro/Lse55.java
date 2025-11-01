import static spark.Spark.*;

public class Lse55 {
    // Simple in-memory flag to simulate a session
    private static boolean isLoggedIn = false;
    private static boolean isAdmin = false;

    public static void main(String[] args) {
        port(8080);

        // 1. The login function (GET request)
        // Displays the login form
        get("/login", (req, res) -> {
            return login();
        });

        // 2. The do_login function (POST request)
        // Processes login credentials
        post("/do_login", (req, res) -> {
            String user = req.queryParams("username");
            String pass = req.queryParams("password");
            String result = do_login(user, pass);
            // In a real app, you'd redirect, but here we just return the result
            res.body(result);
            if (result.contains("successful")) {
                 res.redirect("/");
            } else {
                 res.redirect("/login");
            }
            return res.body();
        });

        // 3. The index function (GET request)
        // The home page
        get("/", (req, res) -> {
            return index();
        });
        
        // 4. The user_page function (GET request)
        get("/user", (req, res) -> {
            return user_page();
        });

        // 5. The admin_page function (GET request)
        get("/admin", (req, res) -> {
            return admin_page();
        });

        System.out.println("Server started on http://localhost:8080");
        System.out.println("--- Test Cases ---");
        System.out.println("1. View login page: curl http://localhost:8080/login");
        System.out.println("2. Attempt login: curl -X POST -d \"username=admin&password=adminpass\" http://localhost:8080/do_login");
        System.out.println("3. View index page (after login): curl http://localhost:8080/");
        System.out.println("4. View user page (after login): curl http://localhost:8080/user");
        System.out.println("5. View admin page (after admin login): curl http://localhost:8080/admin");
    }

    // Function to render the login page HTML
    public static String login() {
        return "<html><body>"
                + "<h2>Login Form</h2>"
                + "<form action='/do_login' method='post'>"
                + "  Username: <input type='text' name='username'><br>"
                + "  Password: <input type='password' name='password'><br>"
                + "  <input type='submit' value='Login'>"
                + "</form>"
                + "</body></html>";
    }

    // Function to handle the login logic
    public static String do_login(String username, String password) {
        // Simple hardcoded credentials for demonstration
        if ("user".equals(username) && "userpass".equals(password)) {
            isLoggedIn = true;
            isAdmin = false;
            return "User login successful!";
        } else if ("admin".equals(username) && "adminpass".equals(password)) {
            isLoggedIn = true;
            isAdmin = true;
            return "Admin login successful!";
        }
        isLoggedIn = false;
        isAdmin = false;
        return "Login failed!";
    }

    // Function to render the index page
    public static String index() {
        if (isLoggedIn) {
            return "<html><body><h1>Welcome to the Index Page!</h1><p>You are logged in.</p></body></html>";
        }
        return "<html><body><h1>Welcome to the Index Page!</h1><p>Please <a href='/login'>login</a>.</p></body></html>";
    }
    
    // Function to render the user page
    public static String user_page() {
        if (isLoggedIn && !isAdmin) {
             return "<html><body><h1>Welcome, User!</h1><p>This is the standard user page.</p></body></html>";
        } else if (isLoggedIn && isAdmin) {
            return "<html><body><h1>Admins can also see user pages.</h1></body></html>";
        }
        return "<html><body><h1>Access Denied</h1><p>You must be logged in as a user to see this page.</p></body></html>";
    }

    // Function to render the admin page
    public static String admin_page() {
        if (isLoggedIn && isAdmin) {
            return "<html><body><h1>Welcome, Admin!</h1><p>This is the admin-only page.</p></body></html>";
        }
        return "<html><body><h1>Access Denied</h1><p>You must be an admin to see this page.</p></body></html>";
    }
}