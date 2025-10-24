import java.io.IOException;
import java.io.PrintWriter;
import java.security.SecureRandom;
import java.util.Base64;

import jakarta.servlet.ServletException;
import jakarta.servlet.annotation.WebServlet;
import jakarta.servlet.http.HttpServlet;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import jakarta.servlet.http.HttpSession;

/*
 * NOTE: This is a Java Servlet. It cannot be run as a standalone application
 * from a main method. It requires a Jakarta EE compatible servlet container
 * like Apache Tomcat 10+ to be deployed and run.
 *
 * How to run and test this code:
 * 1. You need a Servlet Container like Apache Tomcat.
 * 2. Compile this Java file with the Servlet API in your classpath.
 *    (e.g., javac -cp jakarta.servlet-api-6.0.0.jar Task40.java)
 * 3. Place the compiled .class file in the WEB-INF/classes directory of a web application.
 * 4. Deploy the web application to your servlet container.
 * 5. Access the servlet in your browser, typically at a URL like:
 *    http://localhost:8080/[YourApplicationName]/settings
 *
 * How to perform 5 manual test cases:
 * 1. [Normal Load]: Navigate to the servlet's URL. The form should load correctly.
 * 2. [Valid Submission]: Fill in the form and click "Update". You should see a success message.
 * 3. [Invalid Token Submission]:
 *    a. Load the form.
 *    b. Use browser developer tools to change the value of the hidden "csrf_token" input to "invalidtoken".
 *    c. Submit the form. You should receive a "403 Forbidden" error.
 * 4. [Missing Token Submission]:
 *    a. Load the form.
 *    b. Use browser developer tools to delete the hidden "csrf_token" input field.
 *    c. Submit the form. You should receive a "403 Forbidden" error.
 * 5. [Token Replay Attack]:
 *    a. Submit the form successfully.
 *    b. Use the browser's "back" button to return to the form page.
 *    c. Try to submit the form again without reloading. You should receive a "403 Forbidden" error.
*/

@WebServlet("/settings")
public class Task40 extends HttpServlet {
    private static final long serialVersionUID = 1L;

    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        
        HttpSession session = request.getSession(true);

        // 1. Generate a secure, random CSRF token
        byte[] tokenBytes = new byte[32];
        new SecureRandom().nextBytes(tokenBytes);
        String csrfToken = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);

        // 2. Store the token in the user's session
        session.setAttribute("csrf_token", csrfToken);

        // 3. Render the HTML form with the token in a hidden field
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        out.println("<html><head><title>User Settings</title></head><body>");
        out.println("<h2>Update User Settings</h2>");
        out.println("<form method='POST' action='settings'>");
        out.println("  Email: <input type='email' name='email'><br/><br/>");
        out.println("  <input type='hidden' name='csrf_token' value='" + csrfToken + "'>");
        out.println("  <input type='submit' value='Update'>");
        out.println("</form>");
        out.println("</body></html>");
    }

    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {

        HttpSession session = request.getSession(false);

        String sessionToken = null;
        if (session != null) {
            sessionToken = (String) session.getAttribute("csrf_token");
        }
        
        String requestToken = request.getParameter("csrf_token");

        // 4. Validate the CSRF token
        if (sessionToken == null || requestToken == null || !sessionToken.equals(requestToken)) {
            response.sendError(HttpServletResponse.SC_FORBIDDEN, "CSRF token is invalid or missing.");
            return;
        }

        // 5. Invalidate the token after use to prevent reuse (replay attacks)
        if (session != null) {
             session.removeAttribute("csrf_token");
        }

        // --- CSRF check passed, process the form data ---
        String email = request.getParameter("email");
        
        // In a real application, you would update the database here.
        // For this example, we'll just print a success message.
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        out.println("<html><body>");
        out.println("<h2>Settings updated successfully!</h2>");
        out.println("<p>New email (simulated update): " + (email != null ? email : "N/A") + "</p>");
        out.println("</body></html>");
    }
}