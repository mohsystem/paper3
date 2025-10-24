
import javax.naming.*;
import java.util.*;
import java.util.regex.Pattern;

public class Task107 {
    private static final Pattern SAFE_JNDI_PATTERN = Pattern.compile("^java:comp/env/[a-zA-Z0-9/_-]+$");
    private static final Set<String> ALLOWED_SCHEMES = Collections.unmodifiableSet(
        new HashSet<>(Arrays.asList("java:comp/env"))
    );
    
    public static Object safeLookup(String jndiName) {
        if (jndiName == null || jndiName.trim().isEmpty()) {
            throw new IllegalArgumentException("JNDI name cannot be null or empty");
        }
        
        String trimmedName = jndiName.trim();
        
        // Validate JNDI name format - only allow safe local lookups
        if (!SAFE_JNDI_PATTERN.matcher(trimmedName).matches()) {
            throw new IllegalArgumentException("Invalid JNDI name format. Only java:comp/env/ namespace allowed");
        }
        
        // Check for path traversal attempts
        if (trimmedName.contains("..") || trimmedName.contains("//")) {
            throw new IllegalArgumentException("Path traversal detected in JNDI name");
        }
        
        Context context = null;
        try {
            // Use InitialContext with restricted environment
            Hashtable<String, String> env = new Hashtable<>();
            env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.fscontext.RefFSContextFactory");
            env.put(Context.OBJECT_FACTORIES, "");
            
            context = new InitialContext(env);
            
            Object result = context.lookup(trimmedName);
            
            // Validate result is not a remote reference
            if (result instanceof Reference || result instanceof Referenceable) {
                throw new NamingException("Remote references not allowed");
            }
            
            return result;
        } catch (NamingException e) {
            System.err.println("JNDI lookup failed: " + e.getMessage());
            return null;
        } finally {
            if (context != null) {
                try {
                    context.close();
                } catch (NamingException e) {
                    System.err.println("Error closing context: " + e.getMessage());
                }
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("JNDI Secure Lookup Examples:");
        System.out.println("=" .repeat(50));
        
        // Test case 1: Valid JNDI name
        String testName1 = "java:comp/env/jdbc/myDataSource";
        System.out.println("\\nTest 1 - Valid name: " + testName1);
        try {
            Object result = safeLookup(testName1);
            System.out.println("Result: " + (result != null ? "Success (simulated)" : "Not found"));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 2: Invalid scheme
        String testName2 = "ldap://malicious.com/exploit";
        System.out.println("\\nTest 2 - Invalid scheme: " + testName2);
        try {
            Object result = safeLookup(testName2);
            System.out.println("Result: " + result);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 3: Path traversal attempt
        String testName3 = "java:comp/env/../../../etc/passwd";
        System.out.println("\\nTest 3 - Path traversal: " + testName3);
        try {
            Object result = safeLookup(testName3);
            System.out.println("Result: " + result);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 4: Null input
        String testName4 = null;
        System.out.println("\\nTest 4 - Null input: " + testName4);
        try {
            Object result = safeLookup(testName4);
            System.out.println("Result: " + result);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 5: Empty string
        String testName5 = "";
        System.out.println("\\nTest 5 - Empty string: '" + testName5 + "'");
        try {
            Object result = safeLookup(testName5);
            System.out.println("Result: " + result);
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}
