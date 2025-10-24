
import javax.naming.*;
import javax.naming.directory.*;
import java.util.*;

public class Task68 {
    
    public static String searchLDAP(String dcComponent, String username) {
        StringBuilder result = new StringBuilder();
        
        try {
            // Setup LDAP connection properties
            Hashtable<String, String> env = new Hashtable<>();
            env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
            env.put(Context.PROVIDER_URL, "ldap://localhost:389");
            env.put(Context.SECURITY_AUTHENTICATION, "none");
            
            // Create LDAP context
            DirContext ctx = new InitialDirContext(env);
            
            // Construct base DN from DC component
            String baseDN = "dc=" + dcComponent.replace(".", ",dc=");
            
            // Construct LDAP search filter (VULNERABLE TO INJECTION)
            String searchFilter = "(uid=" + username + ")";
            
            // Setup search controls
            SearchControls searchControls = new SearchControls();
            searchControls.setSearchScope(SearchControls.SUBTREE_SCOPE);
            
            // Perform search
            NamingEnumeration<SearchResult> results = ctx.search(baseDN, searchFilter, searchControls);
            
            // Process results
            while (results.hasMore()) {
                SearchResult searchResult = results.next();
                Attributes attributes = searchResult.getAttributes();
                
                result.append("DN: ").append(searchResult.getNameInNamespace()).append("\\n");
                
                NamingEnumeration<? extends Attribute> attrs = attributes.getAll();
                while (attrs.hasMore()) {
                    Attribute attr = attrs.next();
                    result.append(attr.getID()).append(": ").append(attr.get()).append("\\n");
                }
                result.append("\\n");
            }
            
            ctx.close();
            
            if (result.length() == 0) {
                result.append("No user found");
            }
            
        } catch (Exception e) {
            result.append("Error: ").append(e.getMessage());
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(searchLDAP("example.com", "john.doe"));
        System.out.println("---");
        
        System.out.println("Test Case 2:");
        System.out.println(searchLDAP("company.org", "admin"));
        System.out.println("---");
        
        System.out.println("Test Case 3:");
        System.out.println(searchLDAP("test.local", "user123"));
        System.out.println("---");
        
        System.out.println("Test Case 4:");
        System.out.println(searchLDAP("domain.net", "testuser"));
        System.out.println("---");
        
        System.out.println("Test Case 5:");
        System.out.println(searchLDAP("myorg.com", "employee"));
        System.out.println("---");
        
        // Command line argument handling
        if (args.length >= 2) {
            System.out.println("\\nCommand Line Arguments:");
            System.out.println(searchLDAP(args[0], args[1]));
        }
    }
}
