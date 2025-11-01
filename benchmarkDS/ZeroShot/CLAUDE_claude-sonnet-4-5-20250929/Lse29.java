
import java.io.IOException;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.io.PrintWriter;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class Lse29 extends HttpServlet {
    private static final Pattern ALLOWED_URL_PATTERN = Pattern.compile("^https?://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/.*)?$");
    
    public static boolean isValidTarget(String target) {
        if (target == null || target.isEmpty()) {
            return false;
        }
        Matcher matcher = ALLOWED_URL_PATTERN.matcher(target);
        return matcher.matches();
    }
    
    public static String getRedirectUrl(String target) {
        if (isValidTarget(target)) {
            return target;
        }
        return "/";
    }
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws IOException {
        String target = request.getParameter("target");
        String redirectUrl = getRedirectUrl(target);
        response.sendRedirect(redirectUrl);
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: " + getRedirectUrl("http://example.com"));
        System.out.println("Expected: http://example.com");
        
        System.out.println("\\nTest Case 2: " + getRedirectUrl("https://subdomain.example.com/path"));
        System.out.println("Expected: https://subdomain.example.com/path");
        
        System.out.println("\\nTest Case 3: " + getRedirectUrl("http://malicious.com"));
        System.out.println("Expected: /");
        
        System.out.println("\\nTest Case 4: " + getRedirectUrl("https://example.com.malicious.com"));
        System.out.println("Expected: /");
        
        System.out.println("\\nTest Case 5: " + getRedirectUrl(null));
        System.out.println("Expected: /");
    }
}
