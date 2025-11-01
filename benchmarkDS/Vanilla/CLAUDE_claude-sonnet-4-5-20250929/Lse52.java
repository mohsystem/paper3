
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet(urlPatterns = {"/", "/login", "/secret", "/do_login"})
public class Lse52 extends HttpServlet {
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String path = request.getServletPath();
        
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        switch (path) {
            case "/":
                showIndexPage(out);
                break;
            case "/login":
                showLoginPage(out);
                break;
            case "/secret":
                showSecretPage(request, out);
                break;
            default:
                response.sendError(HttpServletResponse.SC_NOT_FOUND);
        }
    }
    
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String path = request.getServletPath();
        
        if ("/do_login".equals(path)) {
            doLogin(request, response);
        } else {
            response.sendError(HttpServletResponse.SC_NOT_FOUND);
        }
    }
    
    private void showIndexPage(PrintWriter out) {
        out.println("<html><body>");
        out.println("<h1>Welcome to Index Page</h1>");
        out.println("<a href='/login'>Go to Login</a>");
        out.println("</body></html>");
    }
    
    private void showLoginPage(PrintWriter out) {
        out.println("<html><body>");
        out.println("<h1>Login Page</h1>");
        out.println("<form action='/do_login' method='post'>");
        out.println("Username: <input type='text' name='username'><br>");
        out.println("Password: <input type='password' name='password'><br>");
        out.println("<input type='submit' value='Login'>");
        out.println("</form>");
        out.println("</body></html>");
    }
    
    private void doLogin(HttpServletRequest request, HttpServletResponse response) 
            throws IOException {
        String username = request.getParameter("username");
        String password = request.getParameter("password");
        
        if ("admin".equals(username) && "password123".equals(password)) {
            HttpSession session = request.getSession();
            session.setAttribute("username", username);
            response.sendRedirect("/secret");
        } else {
            response.sendRedirect("/login");
        }
    }
    
    private void showSecretPage(HttpServletRequest request, PrintWriter out) {
        HttpSession session = request.getSession(false);
        
        if (session != null && session.getAttribute("username") != null) {
            out.println("<html><body>");
            out.println("<h1>Secret settings</h1>");
            out.println("</body></html>");
        } else {
            out.println("<html><body>");
            out.println("<h1>Access Denied</h1>");
            out.println("<a href='/login'>Please login</a>");
            out.println("</body></html>");
        }
    }
}
