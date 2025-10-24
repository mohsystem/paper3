
import javax.naming.*;
import java.util.*;

public class Task107 {
    
    public static Object performJNDILookup(String jndiName) {
        try {
            Properties props = new Properties();
            props.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.fscontext.RefFSContextFactory");
            props.put(Context.PROVIDER_URL, "file:///tmp/jndi");
            
            Context context = new InitialContext(props);
            Object resource = context.lookup(jndiName);
            context.close();
            
            return resource;
        } catch (NamingException e) {
            System.err.println("JNDI Lookup failed for: " + jndiName);
            System.err.println("Error: " + e.getMessage());
            return null;
        }
    }
    
    public static void bindResource(String jndiName, Object resource) {
        try {
            Properties props = new Properties();
            props.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.fscontext.RefFSContextFactory");
            props.put(Context.PROVIDER_URL, "file:///tmp/jndi");
            
            Context context = new InitialContext(props);
            context.rebind(jndiName, resource);
            context.close();
            
            System.out.println("Successfully bound: " + jndiName);
        } catch (NamingException e) {
            System.err.println("Failed to bind: " + jndiName);
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    public static void listBindings(String contextName) {
        try {
            Properties props = new Properties();
            props.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.fscontext.RefFSContextFactory");
            props.put(Context.PROVIDER_URL, "file:///tmp/jndi");
            
            Context context = new InitialContext(props);
            NamingEnumeration<Binding> bindings = context.listBindings(contextName);
            
            System.out.println("Bindings in context: " + contextName);
            while (bindings.hasMore()) {
                Binding binding = bindings.next();
                System.out.println("  Name: " + binding.getName() + ", Class: " + binding.getClassName());
            }
            
            context.close();
        } catch (NamingException e) {
            System.err.println("Failed to list bindings for: " + contextName);
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== JNDI Lookup Example ===\\n");
        
        // Test Case 1: Bind and lookup a String
        System.out.println("Test Case 1: String Resource");
        String testString = "Hello JNDI World";
        bindResource("test/string", testString);
        Object result1 = performJNDILookup("test/string");
        System.out.println("Retrieved: " + result1);
        System.out.println();
        
        // Test Case 2: Bind and lookup an Integer
        System.out.println("Test Case 2: Integer Resource");
        Integer testInteger = 12345;
        bindResource("test/integer", testInteger);
        Object result2 = performJNDILookup("test/integer");
        System.out.println("Retrieved: " + result2);
        System.out.println();
        
        // Test Case 3: Bind and lookup a custom object
        System.out.println("Test Case 3: Custom Object");
        Map<String, String> testMap = new HashMap<>();
        testMap.put("key1", "value1");
        testMap.put("key2", "value2");
        bindResource("test/map", testMap);
        Object result3 = performJNDILookup("test/map");
        System.out.println("Retrieved: " + result3);
        System.out.println();
        
        // Test Case 4: List all bindings
        System.out.println("Test Case 4: List Bindings");
        listBindings("test");
        System.out.println();
        
        // Test Case 5: Lookup non-existent resource
        System.out.println("Test Case 5: Non-existent Resource");
        Object result5 = performJNDILookup("test/nonexistent");
        System.out.println("Retrieved: " + result5);
    }
}
