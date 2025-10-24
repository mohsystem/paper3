
import javax.naming.*;
import java.util.*;
import java.util.regex.Pattern;

public class Task107 {
    private static final Pattern SAFE_JNDI_PATTERN = Pattern.compile("^[a-zA-Z0-9/_-]+$");
    private static final Set<String> ALLOWED_CONTEXTS = new HashSet<>(Arrays.asList(
        "java:comp/env/jdbc/",
        "java:comp/env/jms/",
        "java:comp/env/mail/"
    ));
    
    // Secure JNDI lookup with validation
    public static Object performSecureJNDILookup(String jndiName) {
        if (jndiName == null || jndiName.isEmpty()) {
            System.out.println("Error: JNDI name cannot be null or empty");
            return null;
        }
        
        // Validate JNDI name format
        if (!SAFE_JNDI_PATTERN.matcher(jndiName).matches()) {
            System.out.println("Error: Invalid JNDI name format: " + jndiName);
            return null;
        }
        
        // Check if JNDI name starts with allowed context
        boolean isAllowed = false;
        for (String allowedContext : ALLOWED_CONTEXTS) {
            if (jndiName.startsWith(allowedContext)) {
                isAllowed = true;
                break;
            }
        }
        
        if (!isAllowed) {
            System.out.println("Error: JNDI context not allowed: " + jndiName);
            return null;
        }
        
        try {
            // Create secure initial context with restricted properties
            Hashtable<String, String> env = new Hashtable<>();
            env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.fscontext.RefFSContextFactory");
            // Disable remote class loading to prevent remote code execution
            env.put("com.sun.jndi.rmi.object.trustURLCodebase", "false");
            env.put("com.sun.jndi.cosnaming.object.trustURLCodebase", "false");
            
            InitialContext context = new InitialContext(env);
            Object resource = context.lookup(jndiName);
            context.close();
            
            System.out.println("Successfully retrieved resource: " + jndiName);
            return resource;
            
        } catch (NamingException e) {
            System.out.println("JNDI Lookup failed for: " + jndiName + " - " + e.getMessage());
            return null;
        }
    }
    
    // Validate JNDI name
    public static boolean isValidJNDIName(String jndiName) {
        if (jndiName == null || jndiName.isEmpty()) {
            return false;
        }
        
        if (!SAFE_JNDI_PATTERN.matcher(jndiName).matches()) {
            return false;
        }
        
        for (String allowedContext : ALLOWED_CONTEXTS) {
            if (jndiName.startsWith(allowedContext)) {
                return true;
            }
        }
        
        return false;
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure JNDI Lookup Program ===\\n");
        
        // Test Case 1: Valid JDBC lookup
        System.out.println("Test 1: Valid JDBC lookup");
        performSecureJNDILookup("java:comp/env/jdbc/myDataSource");
        
        // Test Case 2: Valid JMS lookup
        System.out.println("\\nTest 2: Valid JMS lookup");
        performSecureJNDILookup("java:comp/env/jms/myQueue");
        
        // Test Case 3: Invalid characters (injection attempt)
        System.out.println("\\nTest 3: Invalid characters (security test)");
        performSecureJNDILookup("java:comp/env/jdbc/../../../malicious");
        
        // Test Case 4: Unauthorized context (RMI - potential remote code execution)
        System.out.println("\\nTest 4: Unauthorized context");
        performSecureJNDILookup("rmi://malicious.server/Exploit");
        
        // Test Case 5: Empty JNDI name
        System.out.println("\\nTest 5: Empty JNDI name");
        performSecureJNDILookup("");
        
        // Test validation function
        System.out.println("\\n=== JNDI Name Validation Tests ===");
        System.out.println("Valid name: " + isValidJNDIName("java:comp/env/jdbc/testDB"));
        System.out.println("Invalid name: " + isValidJNDIName("ldap://evil.com/exploit"));
    }
}
