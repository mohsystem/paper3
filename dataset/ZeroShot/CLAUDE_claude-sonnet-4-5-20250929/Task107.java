
import javax.naming.*;
import java.util.Hashtable;

public class Task107 {
    
    // Secure JNDI lookup with validation
    public static Object performJNDILookup(String jndiName, String contextFactory, String providerUrl) {
        // Input validation - prevent JNDI injection attacks
        if (jndiName == null || jndiName.trim().isEmpty()) {
            throw new IllegalArgumentException("JNDI name cannot be null or empty");
        }
        
        // Whitelist validation - only allow safe protocols
        if (jndiName.startsWith("ldap://") || jndiName.startsWith("rmi://") || 
            jndiName.startsWith("dns://") || jndiName.startsWith("iiop://")) {
            throw new SecurityException("Remote JNDI lookups are not allowed for security reasons");
        }
        
        // Only allow java: namespace which is local
        if (!jndiName.startsWith("java:")) {
            throw new SecurityException("Only java: namespace is allowed");
        }
        
        Context context = null;
        try {
            Hashtable<String, String> env = new Hashtable<>();
            
            if (contextFactory != null && !contextFactory.isEmpty()) {
                env.put(Context.INITIAL_CONTEXT_FACTORY, contextFactory);
            }
            
            if (providerUrl != null && !providerUrl.isEmpty()) {
                env.put(Context.PROVIDER_URL, providerUrl);
            }
            
            context = new InitialContext(env);
            Object result = context.lookup(jndiName);
            
            System.out.println("Successfully retrieved resource: " + jndiName);
            return result;
            
        } catch (NamingException e) {
            System.err.println("JNDI lookup failed for: " + jndiName);
            System.err.println("Error: " + e.getMessage());
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
    
    // Secure lookup for local resources only
    public static Object performSecureLocalLookup(String jndiName) {
        return performJNDILookup(jndiName, null, null);
    }
    
    // Bind a resource to JNDI (for testing purposes)
    public static void bindResource(String jndiName, Object resource) {
        if (jndiName == null || !jndiName.startsWith("java:")) {
            throw new SecurityException("Can only bind to java: namespace");
        }
        
        Context context = null;
        try {
            context = new InitialContext();
            context.bind(jndiName, resource);
            System.out.println("Successfully bound resource: " + jndiName);
        } catch (NamingException e) {
            System.err.println("Failed to bind resource: " + e.getMessage());
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
        System.out.println("=== JNDI Lookup Security Demo ===\\n");
        
        // Test Case 1: Valid local lookup (java: namespace)
        System.out.println("Test Case 1: Valid local java: namespace lookup");
        try {
            Object result1 = performSecureLocalLookup("java:comp/env/example");
            System.out.println("Result: " + (result1 != null ? "Found" : "Not found"));
        } catch (Exception e) {
            System.out.println("Expected behavior: " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 2: Attempting LDAP lookup (should be blocked)
        System.out.println("Test Case 2: Blocked LDAP lookup attempt");
        try {
            Object result2 = performJNDILookup("ldap://malicious.com/exploit", null, null);
            System.out.println("Result: " + result2);
        } catch (SecurityException e) {
            System.out.println("Security blocked: " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 3: Attempting RMI lookup (should be blocked)
        System.out.println("Test Case 3: Blocked RMI lookup attempt");
        try {
            Object result3 = performJNDILookup("rmi://malicious.com/exploit", null, null);
            System.out.println("Result: " + result3);
        } catch (SecurityException e) {
            System.out.println("Security blocked: " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 4: Null JNDI name
        System.out.println("Test Case 4: Null JNDI name validation");
        try {
            Object result4 = performJNDILookup(null, null, null);
            System.out.println("Result: " + result4);
        } catch (IllegalArgumentException e) {
            System.out.println("Validation error: " + e.getMessage());
        }
        System.out.println();
        
        // Test Case 5: Empty JNDI name
        System.out.println("Test Case 5: Empty JNDI name validation");
        try {
            Object result5 = performJNDILookup("", null, null);
            System.out.println("Result: " + result5);
        } catch (IllegalArgumentException e) {
            System.out.println("Validation error: " + e.getMessage());
        }
        System.out.println();
        
        System.out.println("=== Security Summary ===");
        System.out.println("✓ Remote lookups (LDAP, RMI, DNS, IIOP) are blocked");
        System.out.println("✓ Only local java: namespace is allowed");
        System.out.println("✓ Input validation prevents injection attacks");
        System.out.println("✓ Proper error handling and resource cleanup");
    }
}
