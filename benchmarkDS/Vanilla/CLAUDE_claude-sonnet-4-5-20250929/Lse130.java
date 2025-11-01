
import java.io.*;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;

public class Lse130 {
    // Template class to simulate template rendering
    static class Template {
        private String templateContent;
        
        public Template(String content) {
            this.templateContent = content;
        }
        
        public String render(Map<String, String> context) {
            String result = templateContent;
            for (Map.Entry<String, String> entry : context.entrySet()) {
                result = result.replace("{{" + entry.getKey() + "}}", entry.getValue());
            }
            return result;
        }
    }
    
    // Environment class to simulate template environment
    static class Environment {
        private Map<String, Template> templates;
        private HttpServletRequest request;
        
        public Environment(HttpServletRequest request) {
            this.request = request;
            this.templates = new HashMap<>();
            this.templates.put("default", new Template("Hello, {{name}}!"));
        }
        
        public String getNameParameter() {
            return request.getParameter("name");
        }
        
        public Template getTemplate(String templateName) {
            return templates.get(templateName);
        }
    }
    
    // Response class to simulate HTTP response
    static class Response {
        private String content;
        
        public Response(String content) {
            this.content = content;
        }
        
        public String getContent() {
            return content;
        }
    }
    
    // Mock request for testing
    static class MockRequest extends HttpServletRequest {
        private Map<String, String> parameters;
        
        public MockRequest(String name) {
            this.parameters = new HashMap<>();
            this.parameters.put("name", name);
        }
        
        @Override
        public String getParameter(String name) {
            return parameters.get(name);
        }
        
        // Other required methods left unimplemented for brevity
        public Object getAttribute(String name) { return null; }
        public Enumeration<String> getAttributeNames() { return null; }
        public String getCharacterEncoding() { return null; }
        public void setCharacterEncoding(String env) {}
        public int getContentLength() { return 0; }
        public long getContentLengthLong() { return 0; }
        public String getContentType() { return null; }
        public ServletInputStream getInputStream() { return null; }
        public String getProtocol() { return null; }
        public String getScheme() { return null; }
        public String getServerName() { return null; }
        public int getServerPort() { return 0; }
        public BufferedReader getReader() { return null; }
        public String getRemoteAddr() { return null; }
        public String getRemoteHost() { return null; }
        public void setAttribute(String name, Object o) {}
        public void removeAttribute(String name) {}
        public Locale getLocale() { return null; }
        public Enumeration<Locale> getLocales() { return null; }
        public boolean isSecure() { return false; }
        public RequestDispatcher getRequestDispatcher(String path) { return null; }
        public String getRealPath(String path) { return null; }
        public int getRemotePort() { return 0; }
        public String getLocalName() { return null; }
        public String getLocalAddr() { return null; }
        public int getLocalPort() { return 0; }
        public ServletContext getServletContext() { return null; }
        public AsyncContext startAsync() { return null; }
        public AsyncContext startAsync(ServletRequest servletRequest, ServletResponse servletResponse) { return null; }
        public boolean isAsyncStarted() { return false; }
        public boolean isAsyncSupported() { return false; }
        public AsyncContext getAsyncContext() { return null; }
        public DispatcherType getDispatcherType() { return null; }
        public String getAuthType() { return null; }
        public Cookie[] getCookies() { return null; }
        public long getDateHeader(String name) { return 0; }
        public String getHeader(String name) { return null; }
        public Enumeration<String> getHeaders(String name) { return null; }
        public Enumeration<String> getHeaderNames() { return null; }
        public int getIntHeader(String name) { return 0; }
        public String getMethod() { return null; }
        public String getPathInfo() { return null; }
        public String getPathTranslated() { return null; }
        public String getContextPath() { return null; }
        public String getQueryString() { return null; }
        public String getRemoteUser() { return null; }
        public boolean isUserInRole(String role) { return false; }
        public java.security.Principal getUserPrincipal() { return null; }
        public String getRequestedSessionId() { return null; }
        public String getRequestURI() { return null; }
        public StringBuffer getRequestURL() { return null; }
        public String getServletPath() { return null; }
        public HttpSession getSession(boolean create) { return null; }
        public HttpSession getSession() { return null; }
        public String changeSessionId() { return null; }
        public boolean isRequestedSessionIdValid() { return false; }
        public boolean isRequestedSessionIdFromCookie() { return false; }
        public boolean isRequestedSessionIdFromURL() { return false; }
        public boolean isRequestedSessionIdFromUrl() { return false; }
        public boolean authenticate(HttpServletResponse response) { return false; }
        public void login(String username, String password) {}
        public void logout() {}
        public Collection<Part> getParts() { return null; }
        public Part getPart(String name) { return null; }
        public <T extends HttpUpgradeHandler> T upgrade(Class<T> handlerClass) { return null; }
        public Map<String, String[]> getParameterMap() { return null; }
        public Enumeration<String> getParameterNames() { return null; }
        public String[] getParameterValues(String name) { return null; }
    }
    
    public static Response renderResponseFromEnv(Environment env) {
        String name = env.getNameParameter();
        Template template = env.getTemplate("default");
        
        Map<String, String> context = new HashMap<>();
        context.put("name", name != null ? name : "World");
        
        String renderedContent = template.render(context);
        return new Response(renderedContent);
    }
    
    public static Response home(Environment env) {
        return renderResponseFromEnv(env);
    }
    
    public static void main(String[] args) {
        // Test case 1
        MockRequest request1 = new MockRequest("Alice");
        Environment env1 = new Environment(request1);
        Response response1 = home(env1);
        System.out.println("Test 1: " + response1.getContent());
        
        // Test case 2
        MockRequest request2 = new MockRequest("Bob");
        Environment env2 = new Environment(request2);
        Response response2 = home(env2);
        System.out.println("Test 2: " + response2.getContent());
        
        // Test case 3
        MockRequest request3 = new MockRequest("Charlie");
        Environment env3 = new Environment(request3);
        Response response3 = home(env3);
        System.out.println("Test 3: " + response3.getContent());
        
        // Test case 4
        MockRequest request4 = new MockRequest("Diana");
        Environment env4 = new Environment(request4);
        Response response4 = home(env4);
        System.out.println("Test 4: " + response4.getContent());
        
        // Test case 5
        MockRequest request5 = new MockRequest(null);
        Environment env5 = new Environment(request5);
        Response response5 = home(env5);
        System.out.println("Test 5: " + response5.getContent());
    }
}
