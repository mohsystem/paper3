import javax.naming.*;
import javax.naming.spi.InitialContextFactory;
import java.io.Serializable;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;

// Step 1: Problem understanding
// Purpose: Provide a secure, in-memory JNDI-like context and helper functions to bind and lookup resources.
// Inputs: A map of initial bindings (name->value as String) and a list of lookup names.
// Output: A map of lookup results (name->value as String), using "<NOT_FOUND>" for missing entries.
// Operations: Validate names/values, bind safely, then perform lookups.

// Step 2: Security requirements
// - No external network access or remote code loading.
// - No ObjectFactory/Reference resolution.
// - Validate names and values strictly (allowed chars, size limits).
// - Constrain types to String only to avoid deserialization risks.

// Step 3 + 4: Secure coding generation and review embedded inline.
// Step 5: Secure code output produced in this single file.

public class Task107 {

    // Allowed name pattern: alphanum, underscore, dash, dot, slash. No leading/trailing slash. Max length 256.
    private static final Pattern NAME_PATTERN = Pattern.compile("^(?!/)(?!.*//)[A-Za-z0-9_.\\-/]{1,256}(?<!/)$");
    private static final int MAX_VALUE_LENGTH = 8192;

    // Public API: performs secure bindings then lookups, returning results.
    public static Map<String, String> performLookups(Map<String, String> initialBindings, List<String> lookupNames) throws NamingException {
        Properties env = new Properties();
        env.put(Context.INITIAL_CONTEXT_FACTORY, InMemoryInitialContextFactory.class.getName());
        InitialContext ctx = new InitialContext(env);

        // Bind initial entries securely
        if (initialBindings != null) {
            for (Map.Entry<String, String> e : initialBindings.entrySet()) {
                String name = e.getKey();
                String value = e.getValue();
                validateName(name);
                validateValue(value);
                ctx.rebind(name, value);
            }
        }

        // Lookup requested names
        Map<String, String> results = new LinkedHashMap<>();
        if (lookupNames != null) {
            for (String name : lookupNames) {
                if (!isNameValid(name)) {
                    results.put(name, "<INVALID_NAME>");
                    continue;
                }
                try {
                    Object o = ctx.lookup(name);
                    if (o instanceof String) {
                        results.put(name, (String) o);
                    } else if (o == null) {
                        results.put(name, "<NULL>");
                    } else {
                        results.put(name, "<UNSAFE_TYPE_BLOCKED>");
                    }
                } catch (NameNotFoundException nf) {
                    results.put(name, "<NOT_FOUND>");
                }
            }
        }
        ctx.close();
        return results;
    }

    private static void validateName(String name) throws NamingException {
        if (!isNameValid(name)) {
            throw new InvalidNameException("Invalid name: " + name);
        }
    }

    private static boolean isNameValid(String name) {
        return name != null && NAME_PATTERN.matcher(name).matches();
    }

    private static void validateValue(String value) throws NamingException {
        if (value == null) {
            throw new NamingException("Null value not allowed");
        }
        if (value.length() > MAX_VALUE_LENGTH) {
            throw new NamingException("Value too large");
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases

        // Test 1: Basic binding and lookup
        Map<String, String> bind1 = new LinkedHashMap<>();
        bind1.put("config/db/url", "jdbc:h2:mem:test1");
        bind1.put("feature/flagA", "true");
        List<String> look1 = Arrays.asList("config/db/url", "feature/flagA", "missing/name");
        Map<String, String> res1 = performLookups(bind1, look1);
        System.out.println("Test1: " + res1);

        // Test 2: Rebinding to new values within one call
        Map<String, String> bind2 = new LinkedHashMap<>();
        bind2.put("service/endpoint", "https://api.v1.example.com");
        bind2.put("service/endpoint", "https://api.v2.example.com"); // map keeps last value
        List<String> look2 = Arrays.asList("service/endpoint");
        Map<String, String> res2 = performLookups(bind2, look2);
        System.out.println("Test2: " + res2);

        // Test 3: Hierarchical names
        Map<String, String> bind3 = new LinkedHashMap<>();
        bind3.put("app/env/prod/log/level", "INFO");
        bind3.put("app/env/prod/threads", "16");
        List<String> look3 = Arrays.asList("app/env/prod/log/level", "app/env/prod/threads");
        Map<String, String> res3 = performLookups(bind3, look3);
        System.out.println("Test3: " + res3);

        // Test 4: Invalid name handling
        Map<String, String> bind4 = new LinkedHashMap<>();
        bind4.put("/bad/leading/slash", "x"); // invalid
        List<String> look4 = Arrays.asList("/bad/leading/slash", "also//bad");
        try {
            Map<String, String> res4 = performLookups(bind4, look4);
            System.out.println("Test4: " + res4);
        } catch (NamingException ne) {
            System.out.println("Test4: Caught expected exception for invalid binding: " + ne.getMessage());
        }

        // Test 5: Large but valid values and missing names
        StringBuilder big = new StringBuilder();
        for (int i = 0; i < 1000; i++) big.append("0123456789");
        Map<String, String> bind5 = new LinkedHashMap<>();
        bind5.put("blob/data", big.toString());
        List<String> look5 = Arrays.asList("blob/data", "not/present");
        Map<String, String> res5 = performLookups(bind5, look5);
        System.out.println("Test5: value sizes -> blob/data=" + res5.get("blob/data").length() + ", not/present=" + res5.get("not/present"));
    }
}

// Secure, in-memory InitialContextFactory (no external providers)
class InMemoryInitialContextFactory implements InitialContextFactory, Serializable {
    private static final long serialVersionUID = 1L;
    private static final InMemoryContext SINGLETON = new InMemoryContext();

    @Override
    public Context getInitialContext(Hashtable<?, ?> environment) {
        return SINGLETON;
    }
}

// Minimal, secure in-memory Context implementation
class InMemoryContext implements Context, Serializable {
    private static final long serialVersionUID = 1L;
    private final Map<String, Object> bindings = new ConcurrentHashMap<>();
    private final Hashtable<String, Object> env = new Hashtable<>();

    private static final Pattern NAME_PATTERN = Pattern.compile("^(?!/)(?!.*//)[A-Za-z0-9_.\\-/]{1,256}(?<!/)$");
    private static final int MAX_VALUE_LENGTH = 8192;

    private void ensureValidName(String name) throws InvalidNameException {
        if (name == null || !NAME_PATTERN.matcher(name).matches()) {
            throw new InvalidNameException("Invalid name: " + name);
        }
    }

    private void ensureValidValue(Object obj) throws NamingException {
        if (!(obj instanceof String)) {
            throw new NamingException("Only String values are allowed");
        }
        String s = (String) obj;
        if (s.length() > MAX_VALUE_LENGTH) {
            throw new NamingException("Value too large");
        }
    }

    @Override
    public Object lookup(Name name) throws NamingException {
        return lookup(name.toString());
    }

    @Override
    public Object lookup(String name) throws NamingException {
        ensureValidName(name);
        if (!bindings.containsKey(name)) {
            throw new NameNotFoundException("Name not found: " + name);
        }
        return bindings.get(name);
    }

    @Override
    public void bind(Name name, Object obj) throws NamingException {
        bind(name.toString(), obj);
    }

    @Override
    public void bind(String name, Object obj) throws NamingException {
        ensureValidName(name);
        ensureValidValue(obj);
        if (bindings.containsKey(name)) {
            throw new NameAlreadyBoundException("Already bound: " + name);
        }
        bindings.put(name, obj);
    }

    @Override
    public void rebind(Name name, Object obj) throws NamingException {
        rebind(name.toString(), obj);
    }

    @Override
    public void rebind(String name, Object obj) throws NamingException {
        ensureValidName(name);
        ensureValidValue(obj);
        bindings.put(name, obj);
    }

    @Override
    public void unbind(Name name) throws NamingException {
        unbind(name.toString());
    }

    @Override
    public void unbind(String name) throws NamingException {
        ensureValidName(name);
        bindings.remove(name);
    }

    @Override
    public void rename(Name oldName, Name newName) throws NamingException {
        rename(oldName.toString(), newName.toString());
    }

    @Override
    public void rename(String oldName, String newName) throws NamingException {
        ensureValidName(oldName);
        ensureValidName(newName);
        if (!bindings.containsKey(oldName)) throw new NameNotFoundException(oldName);
        if (bindings.containsKey(newName)) throw new NameAlreadyBoundException(newName);
        Object val = bindings.remove(oldName);
        bindings.put(newName, val);
    }

    @Override
    public NamingEnumeration<NameClassPair> list(Name name) throws NamingException {
        return list(name.toString());
    }

    @Override
    public NamingEnumeration<NameClassPair> list(String name) throws NamingException {
        ensureValidName(name);
        List<NameClassPair> pairs = new ArrayList<>();
        String prefix = name.endsWith("/") ? name : name + "/";
        for (String k : bindings.keySet()) {
            if (k.startsWith(prefix)) {
                String local = k.substring(prefix.length());
                if (!local.contains("/")) {
                    Object v = bindings.get(k);
                    pairs.add(new NameClassPair(local, v != null ? v.getClass().getName() : "java.lang.Object"));
                }
            }
        }
        Iterator<NameClassPair> it = pairs.iterator();
        return new NamingEnumeration<NameClassPair>() {
            @Override public NameClassPair next() { return it.next(); }
            @Override public boolean hasMore() { return it.hasNext(); }
            @Override public void close() {}
            @Override public boolean hasMoreElements() { return it.hasNext(); }
            @Override public NameClassPair nextElement() { return it.next(); }
        };
    }

    @Override
    public NamingEnumeration<Binding> listBindings(Name name) throws NamingException { throw new OperationNotSupportedException(); }
    @Override
    public NamingEnumeration<Binding> listBindings(String name) throws NamingException { throw new OperationNotSupportedException(); }
    @Override
    public void destroySubcontext(Name name) throws NamingException { throw new OperationNotSupportedException(); }
    @Override
    public void destroySubcontext(String name) throws NamingException { throw new OperationNotSupportedException(); }
    @Override
    public Context createSubcontext(Name name) throws NamingException { throw new OperationNotSupportedException(); }
    @Override
    public Context createSubcontext(String name) throws NamingException { throw new OperationNotSupportedException(); }

    @Override
    public Object lookupLink(Name name) throws NamingException { return lookup(name); }
    @Override
    public Object lookupLink(String name) throws NamingException { return lookup(name); }

    @Override
    public NameParser getNameParser(Name name) {
        return CompositeName::new;
    }

    @Override
    public NameParser getNameParser(String name) {
        return CompositeName::new;
    }

    @Override
    public Name composeName(Name name, Name prefix) throws NamingException {
        Name res = (Name) prefix.clone();
        res.addAll(name);
        return res;
    }

    @Override
    public String composeName(String name, String prefix) {
        if (prefix == null || prefix.isEmpty()) return name;
        if (name == null || name.isEmpty()) return prefix;
        return prefix + "/" + name;
    }

    @Override
    public Object addToEnvironment(String propName, Object propVal) {
        return env.put(propName, propVal);
    }

    @Override
    public Object removeFromEnvironment(String propName) {
        return env.remove(propName);
    }

    @Override
    public Hashtable<?, ?> getEnvironment() {
        return (Hashtable<?, ?>) env.clone();
    }

    @Override
    public void close() {}

    @Override
    public String getNameInNamespace() {
        return "";
    }
}