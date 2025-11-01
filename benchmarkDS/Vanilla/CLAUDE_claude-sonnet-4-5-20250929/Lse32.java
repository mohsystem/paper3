
import java.io.IOException;
import java.net.URL;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

@WebServlet("/redirect/*")
public class Lse32 extends HttpServlet {
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String pathInfo = request.getPathInfo();
        
        if (pathInfo != null && pathInfo.length() > 1) {
            String url = pathInfo.substring(1);
            String result = redirectInvalidUrl(url);
            
            if (result.startsWith("http")) {
                response.sendRedirect(result);
            } else {
                response.setContentType("text/plain");
                response.getWriter().write(result);
            }
        } else {
            response.setContentType("text/plain");
            response.getWriter().write("Error: No URL provided");
        }
    }
    
    public static String redirectInvalidUrl(String url) {
        try {
            if (!url.startsWith("http://") && !url.startsWith("https://")) {
                url = "http://" + url;
            }
            
            URL urlObj = new URL(url);
            String host = urlObj.getHost();
            
            if (host.equals("example.com") || host.endsWith(".example.com")) {
                return url;
            } else {
                return "Error: Invalid URL. Only example.com URLs are allowed.";
            }
        } catch (Exception e) {
            return "Error: Invalid URL format";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + redirectInvalidUrl("example.com/page"));
        System.out.println("Test 2: " + redirectInvalidUrl("http://example.com/test"));
        System.out.println("Test 3: " + redirectInvalidUrl("https://www.example.com/home"));
        System.out.println("Test 4: " + redirectInvalidUrl("google.com"));
        System.out.println("Test 5: " + redirectInvalidUrl("http://malicious.com"));
    }
}
