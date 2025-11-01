
import java.io.IOException;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class Lse29 extends HttpServlet {
    private static final Pattern ALLOWED_PATTERN = Pattern.compile(".*example\\\\.com.*");
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String target = request.getParameter("target");
        
        if (target != null && isValidTarget(target)) {
            response.sendRedirect(target);
        } else {
            response.sendRedirect("/");
        }
    }
    
    public static boolean isValidTarget(String target) {
        if (target == null) {
            return false;
        }
        Matcher matcher = ALLOWED_PATTERN.matcher(target);
        return matcher.matches();
    }
    
    public static void main(String[] args) {
        Lse29 app = new Lse29();
        
        // Test cases
        System.out.println("Test 1: " + isValidTarget("https://example.com") + " (Expected: true)");
        System.out.println("Test 2: " + isValidTarget("https://www.example.com/page") + " (Expected: true)");
        System.out.println("Test 3: " + isValidTarget("https://subdomain.example.com") + " (Expected: true)");
        System.out.println("Test 4: " + isValidTarget("https://google.com") + " (Expected: false)");
        System.out.println("Test 5: " + isValidTarget("https://malicious.com") + " (Expected: false)");
    }
}
