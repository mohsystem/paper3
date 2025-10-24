
import javax.naming.*;
import javax.naming.directory.*;
import java.util.*;
import java.util.regex.*;

public class Task68 {
    private static final String LDAP_URL = "ldap://localhost:389";
    private static final int MAX_INPUT_LENGTH = 256;
    private static final Pattern DC_PATTERN = Pattern.compile("^[a-zA-Z0-9][a-zA-Z0-9-]*[a-zA-Z0-9]$");
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]+$");
    
    public static String searchUser(String dcInput, String username) {
        if (dcInput == null || username == null) {
            return "Error: Input cannot be null";
        }
        
        if (dcInput.length() > MAX_INPUT_LENGTH || username.length() > MAX_INPUT_LENGTH) {
            return "Error: Input exceeds maximum length";
        }
        
        if (dcInput.isEmpty() || username.isEmpty()) {
            return "Error: Input cannot be empty";
        }
        
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            return "Error: Invalid username format";
        }
        
        String[] dcComponents = dcInput.split("\\\\.");
        if (dcComponents.length == 0 || dcComponents.length > 10) {
            return "Error: Invalid domain component format";
        }
        
        for (String dc : dcComponents) {
            if (!DC_PATTERN.matcher(dc).matches()) {
                return "Error: Invalid domain component: " + dc;
            }
        }
        
        StringBuilder baseDN = new StringBuilder();
        for (int i = 0; i < dcComponents.length; i++) {
            if (i > 0) {
                baseDN.append(",");
            }
            baseDN.append("dc=").append(dcComponents[i]);
        }
        
        String sanitizedUsername = username.replace("\\\\", "\\\\5c")\n                                             .replace("*", "\\\\2a")\n                                             .replace("(", "\\\\28")\n                                             .replace(")", "\\\\29")\n                                             .replace("\\0", "\\\\00");\n        \n        String filter = "(uid=" + sanitizedUsername + ")";\n        \n        Hashtable<String, String> env = new Hashtable<>();\n        env.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");\n        env.put(Context.PROVIDER_URL, LDAP_URL);\n        env.put(Context.SECURITY_AUTHENTICATION, "none");\n        env.put("com.sun.jndi.ldap.connect.timeout", "5000");\n        env.put("com.sun.jndi.ldap.read.timeout", "5000");\n        \n        DirContext ctx = null;\n        NamingEnumeration<SearchResult> results = null;\n        \n        try {\n            ctx = new InitialDirContext(env);\n            \n            SearchControls controls = new SearchControls();\n            controls.setSearchScope(SearchControls.SUBTREE_SCOPE);\n            controls.setCountLimit(10);\n            controls.setTimeLimit(5000);\n            controls.setReturningAttributes(new String[]{"uid", "cn", "mail"});\n            \n            results = ctx.search(baseDN.toString(), filter, controls);\n            \n            StringBuilder result = new StringBuilder();\n            int count = 0;\n            \n            while (results.hasMore() && count < 10) {\n                SearchResult searchResult = results.next();\n                Attributes attrs = searchResult.getAttributes();\n                \n                if (count > 0) {\n                    result.append("\
---\
");\n                }\n                \n                result.append("DN: ").append(searchResult.getNameInNamespace()).append("\
");\n                \n                NamingEnumeration<? extends Attribute> allAttrs = attrs.getAll();\n                while (allAttrs.hasMore()) {\n                    Attribute attr = allAttrs.next();\n                    result.append(attr.getID()).append(": ").append(attr.get()).append("\
");\n                }\n                \n                count++;\n            }\n            \n            if (count == 0) {\n                return "No user found";\n            }\n            \n            return result.toString();\n            \n        } catch (NamingException e) {\n            return "Error: LDAP operation failed - " + e.getClass().getSimpleName();\n        } finally {\n            try {\n                if (results != null) {\n                    results.close();\n                }\n                if (ctx != null) {\n                    ctx.close();\n                }\n            } catch (NamingException e) {\n                // Ignore cleanup errors\n            }\n        }\n    }\n    \n    public static void main(String[] args) {\n        System.out.println("Test 1: Valid input");\n        System.out.println(searchUser("example.com", "testuser"));\n        System.out.println();\n        \n        System.out.println("Test 2: Invalid username with special chars");\n        System.out.println(searchUser("example.com", "test*user"));\n        System.out.println();\n        \n        System.out.println("Test 3: Null input");\n        System.out.println(searchUser(null, "testuser"));\n        System.out.println();\n        \n        System.out.println("Test 4: Empty input");\n        System.out.println(searchUser("", "testuser"));\n        System.out.println();\n        \n        System.out.println("Test 5: Invalid domain component");\n        System.out.println(searchUser("exam ple.com", "testuser"));
    }
}
