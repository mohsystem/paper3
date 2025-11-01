
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.io.IOException;

public class Lse53 {
    
    public static String redirectUser(boolean isLoggedIn, boolean isAdmin) {
        if (!isLoggedIn) {
            return "login.jsp";
        } else if (isAdmin) {
            return "admin.jsp";
        } else {
            return "user.jsp";
        }
    }
    
    public static void handleRedirect(HttpServletRequest request, HttpServletResponse response) 
            throws IOException {
        HttpSession session = request.getSession(false);
        
        boolean isLoggedIn = false;
        boolean isAdmin = false;
        
        if (session != null) {
            Boolean loggedIn = (Boolean) session.getAttribute("isLoggedIn");
            Boolean admin = (Boolean) session.getAttribute("isAdmin");
            
            isLoggedIn = loggedIn != null && loggedIn;
            isAdmin = admin != null && admin;
        }
        
        String redirectPage = redirectUser(isLoggedIn, isAdmin);
        response.sendRedirect(redirectPage);
    }
    
    public static void main(String[] args) {
        // Test case 1: User not logged in
        System.out.println("Test 1 - Not logged in: " + redirectUser(false, false));
        
        // Test case 2: User logged in but not admin
        System.out.println("Test 2 - Logged in, not admin: " + redirectUser(true, false));
        
        // Test case 3: User logged in and is admin
        System.out.println("Test 3 - Logged in, is admin: " + redirectUser(true, true));
        
        // Test case 4: Not logged in but somehow admin flag is true (shouldn't happen)
        System.out.println("Test 4 - Not logged in, admin flag true: " + redirectUser(false, true));
        
        // Test case 5: Regular user scenario
        System.out.println("Test 5 - Regular user: " + redirectUser(true, false));
    }
}
