
import javax.naming.*;
import javax.naming.directory.*;
import java.util.*;

public class Task68 {
    
    public static String searchLdapUser(String domainComponent, String username) {
        StringBuilder result = new StringBuilder();
        
        try {
            // Set up environment for creating initial context
            Hashtable<String, String> env = new Hashtable<>();
            env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
            env.put(Context.PROVIDER_URL, "ldap://localhost:389");
            env.put(Context.SECURITY_AUTHENTICATION, "none");
            
            // Create initial context
            DirContext ctx = new InitialDirContext(env);
            
            // Construct the base DN from domain component
            String baseDN = "dc=" + domainComponent.replace(".", ",dc=");
            
            // Construct the search filter
            String searchFilter = "(uid=" + username + ")";
            
            // Set up search controls
            SearchControls searchControls = new SearchControls();
            searchControls.setSearchScope(SearchControls.SUBTREE_SCOPE);
            
            // Perform search
            NamingEnumeration<SearchResult> results = ctx.search(baseDN, searchFilter, searchControls);
            
            if (results.hasMore()) {
                SearchResult searchResult = results.next();
                Attributes attributes = searchResult.getAttributes();
                
                result.append("User found: ").append(searchResult.getNameInNamespace()).append("\\n");
                result.append("Attributes:\\n");
                
                NamingEnumeration<? extends Attribute> attrs = attributes.getAll();
                while (attrs.hasMore()) {
                    Attribute attr = attrs.next();
                    result.append("  ").append(attr.getID()).append(": ").append(attr.get()).append("\\n");
                }
            } else {
                result.append("User not found");
            }
            
            ctx.close();
            
        } catch (Exception e) {
            result.append("Error: ").append(e.getMessage());
        }
        
        return result.toString();
    }
    
    public static void main(String[] args) {
        if (args.length >= 2) {
            String domainComponent = args[0];
            String username = args[1];
            System.out.println(searchLdapUser(domainComponent, username));
        } else {
            // Test cases
            System.out.println("Test Case 1:");
            System.out.println(searchLdapUser("example.com", "john.doe"));
            System.out.println("\\n---\\n");
            
            System.out.println("Test Case 2:");
            System.out.println(searchLdapUser("company.org", "alice"));
            System.out.println("\\n---\\n");
            
            System.out.println("Test Case 3:");
            System.out.println(searchLdapUser("test.local", "bob.smith"));
            System.out.println("\\n---\\n");
            
            System.out.println("Test Case 4:");
            System.out.println(searchLdapUser("domain.net", "admin"));
            System.out.println("\\n---\\n");
            
            System.out.println("Test Case 5:");
            System.out.println(searchLdapUser("mycompany.com", "testuser"));
        }
    }
}
