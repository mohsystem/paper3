import javax.naming.*;
import javax.naming.spi.InitialContextFactory;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.nio.charset.StandardCharsets;

public class Task107 {

    // Validate JNDI names: only allow alnum, '_', '-', '.', '/' and length 1..64
    public static boolean isValidName(String name) {
        if (name == null) return false;
        int len = name.length();
        if (len < 1 || len > 64) return false;
        for (int i = 0; i < len; i++) {
            char c = name.charAt(i);
            if (!(Character.isLetterOrDigit(c) || c == '_' || c == '-' || c == '.' || c == '/')) {
                return false;
            }
        }
        return true;
    }

    // Perform lookups using a secure in-memory JNDI provider
    public static List<String> jndiLookup(Map<String, Object> initialBindings, List<String> names) {
        Objects.requireNonNull(initialBindings, "initialBindings");
        Objects.requireNonNull(names, "names");
        Hashtable<String, String> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, SafeMemoryContextFactory.class.getName());

        List<String> results = new ArrayList<>(names.size());
        try {
            InitialContext ctx = new InitialContext(env);
            try {
                // Bind initial entries safely
                for (Map.Entry<String, Object> e : initialBindings.entrySet()) {
                    String key = e.getKey();
                    if (!isValidName(key)) {
                        continue; // skip invalid keys
                    }
                    Object val = e.getValue();
                    // Only allow simple safe types for demo: String, Number, Boolean or SafeResource
                    if (val instanceof String || val instanceof Number || val instanceof Boolean || val instanceof SafeResource) {
                        try {
                            // Use rebind to allow overriding for repeated tests
                            ctx.rebind(key, val);
                        } catch (NameAlreadyBoundException ex) {
                            ctx.rebind(key, val);
                        }
                    }
                }

                for (String name : names) {
                    if (!isValidName(name)) {
                        results.add("ERROR: invalid-name");
                        continue;
                    }
                    try {
                        Object obj = ctx.lookup(name);
                        results.add(safeToString(obj));
                    } catch (NameNotFoundException nf) {
                        results.add("NOT_FOUND");
                    } catch (NamingException ne) {
                        results.add("ERROR: " + sanitize(ne.getMessage()));
                    }
                }
            } finally {
                try {
                    ctx.close();
                } catch (NamingException ignored) {
                    // ignore close exception
                }
            }
        } catch (NamingException e) {
            // catastrophic init error; fail closed with errors for each name
            for (int i = 0; i < names.size(); i++) {
                results.add("ERROR: initialization-failed");
            }
        }
        return results;
    }

    private static String safeToString(Object obj) {
        if (obj == null) return "null";
        if (obj instanceof String) {
            String s = (String) obj;
            return s.length() > 256 ? s.substring(0, 256) : s;
        }
        if (obj instanceof Number || obj instanceof Boolean) {
            return obj.toString();
        }
        if (obj instanceof SafeResource) {
            return obj.toString();
        }
        return obj.getClass().getName();
    }

    private static String sanitize(String msg) {
        if (msg == null) return "unknown";
        StringBuilder sb = new StringBuilder(Math.min(128, msg.length()));
        for (int i = 0; i < msg.length() && i < 128; i++) {
            char c = msg.charAt(i);
            if (c >= 32 && c < 127) sb.append(c);
            else sb.append('?');
        }
        return sb.toString();
    }

    // A simple safe resource for demonstration
    public static final class SafeResource {
        private final String name;
        private final int capacity;

        public SafeResource(String name, int capacity) {
            this.name = name == null ? "resource" : name;
            this.capacity = Math.max(0, Math.min(capacity, 1_000_000));
        }

        @Override
        public String toString() {
            return "SafeResource{name=" + name + ",capacity=" + capacity + "}";
        }
    }

    public static void main(String[] args) {
        Map<String, Object> initial = new HashMap<>();
        initial.put("config/db/url", "jdbc:postgresql://localhost:5432/app");
        initial.put("config/maxConnections", Integer.valueOf(20));
        initial.put("service/cache", new SafeResource("CacheService", 128));
        initial.put("feature/enabled", Boolean.TRUE);
        initial.put("metrics.endpoint", "http://localhost:8080/metrics");

        // Test case 1: Found entries
        List<String> r1 = jndiLookup(initial, Arrays.asList(
                "config/db/url", "config/maxConnections", "service/cache"));
        System.out.println("Test1: " + r1);

        // Test case 2: Not found and found mixed
        List<String> r2 = jndiLookup(initial, Arrays.asList(
                "config/missing", "feature/enabled", "service/unknown"));
        System.out.println("Test2: " + r2);

        // Test case 3: Invalid names
        List<String> r3 = jndiLookup(initial, Arrays.asList(
                "bad name", "toolongtoolongtoolongtoolongtoolongtoolongtoolongtoolongX", "valid.name-OK/1"));
        System.out.println("Test3: " + r3);

        // Test case 4: Override a binding via rebind
        Map<String, Object> initial2 = new HashMap<>(initial);
        initial2.put("config/maxConnections", Integer.valueOf(50));
        List<String> r4 = jndiLookup(initial2, Arrays.asList("config/maxConnections", "metrics.endpoint"));
        System.out.println("Test4: " + r4);

        // Test case 5: Edge characters allowed
        List<String> r5 = jndiLookup(initial, Arrays.asList("metrics.endpoint", "valid.name-OK/1"));
        System.out.println("Test5: " + r5);
    }
}

class SafeMemoryContextFactory implements InitialContextFactory {
    @Override
    public Context getInitialContext(Hashtable<?, ?> environment) {
        Hashtable<String, Object> envCopy = new Hashtable<>();
        if (environment != null) {
            for (Map.Entry<?, ?> e : environment.entrySet()) {
                Object k = e.getKey();
                Object v = e.getValue();
                if (k instanceof String && v != null) {
                    envCopy.put((String) k, v);
                }
            }
        }
        return new SafeMemoryContext(envCopy);
    }
}

class SafeMemoryContext implements Context {
    private final Map<String, Object> bindings = new ConcurrentHashMap<>();
    private final Hashtable<String, Object> env;
    private volatile boolean closed = false;

    SafeMemoryContext(Hashtable<String, Object> environment) {
        this.env = (environment == null) ? new Hashtable<>() : new Hashtable<>(environment);
    }

    private void ensureOpen() throws NamingException {
        if (closed) throw new NamingException("Context is closed");
    }

    private void validate(String name) throws InvalidNameException {
        if (!Task107.isValidName(name)) throw new InvalidNameException("Invalid name");
    }

    @Override
    public Object lookup(Name name) throws NamingException {
        return lookup(name.toString());
    }

    @Override
    public Object lookup(String name) throws NamingException {
        ensureOpen();
        validate(name);
        Object v = bindings.get(name);
        if (v == null) throw new NameNotFoundException("Name not found: " + name);
        return v;
    }

    @Override
    public void bind(Name name, Object obj) throws NamingException {
        bind(name.toString(), obj);
    }

    @Override
    public void bind(String name, Object obj) throws NamingException {
        ensureOpen();
        validate(name);
        Objects.requireNonNull(obj, "obj");
        if (bindings.containsKey(name)) throw new NameAlreadyBoundException("Already bound: " + name);
        bindings.put(name, obj);
    }

    @Override
    public void rebind(Name name, Object obj) throws NamingException {
        rebind(name.toString(), obj);
    }

    @Override
    public void rebind(String name, Object obj) throws NamingException {
        ensureOpen();
        validate(name);
        Objects.requireNonNull(obj, "obj");
        bindings.put(name, obj);
    }

    @Override
    public void unbind(Name name) throws NamingException {
        unbind(name.toString());
    }

    @Override
    public void unbind(String name) throws NamingException {
        ensureOpen();
        validate(name);
        bindings.remove(name);
    }

    @Override
    public void rename(Name oldName, Name newName) throws NamingException {
        rename(oldName.toString(), newName.toString());
    }

    @Override
    public void rename(String oldName, String newName) throws NamingException {
        ensureOpen();
        validate(oldName);
        validate(newName);
        if (!bindings.containsKey(oldName)) throw new NameNotFoundException("Name not found: " + oldName);
        if (bindings.containsKey(newName)) throw new NameAlreadyBoundException("Already bound: " + newName);
        Object obj = bindings.remove(oldName);
        bindings.put(newName, obj);
    }

    @Override
    public NamingEnumeration<NameClassPair> list(Name name) throws NamingException {
        throw new OperationNotSupportedException("list not supported");
    }

    @Override
    public NamingEnumeration<NameClassPair> list(String name) throws NamingException {
        throw new OperationNotSupportedException("list not supported");
    }

    @Override
    public NamingEnumeration<Binding> listBindings(Name name) throws NamingException {
        throw new OperationNotSupportedException("listBindings not supported");
    }

    @Override
    public NamingEnumeration<Binding> listBindings(String name) throws NamingException {
        throw new OperationNotSupportedException("listBindings not supported");
    }

    @Override
    public void destroySubcontext(Name name) throws NamingException {
        throw new OperationNotSupportedException("destroySubcontext not supported");
    }

    @Override
    public void destroySubcontext(String name) throws NamingException {
        throw new OperationNotSupportedException("destroySubcontext not supported");
    }

    @Override
    public Context createSubcontext(Name name) throws NamingException {
        throw new OperationNotSupportedException("createSubcontext not supported");
    }

    @Override
    public Context createSubcontext(String name) throws NamingException {
        throw new OperationNotSupportedException("createSubcontext not supported");
    }

    @Override
    public Object lookupLink(Name name) throws NamingException {
        throw new OperationNotSupportedException("lookupLink not supported");
    }

    @Override
    public Object lookupLink(String name) throws NamingException {
        throw new OperationNotSupportedException("lookupLink not supported");
    }

    @Override
    public NameParser getNameParser(Name name) throws NamingException {
        throw new OperationNotSupportedException("getNameParser not supported");
    }

    @Override
    public NameParser getNameParser(String name) throws NamingException {
        throw new OperationNotSupportedException("getNameParser not supported");
    }

    @Override
    public Name composeName(Name name, Name prefix) throws NamingException {
        throw new OperationNotSupportedException("composeName not supported");
    }

    @Override
    public String composeName(String name, String prefix) throws NamingException {
        throw new OperationNotSupportedException("composeName not supported");
    }

    @Override
    public Object addToEnvironment(String propName, Object propVal) throws NamingException {
        ensureOpen();
        return env.put(propName, propVal);
    }

    @Override
    public Object removeFromEnvironment(String propName) throws NamingException {
        ensureOpen();
        return env.remove(propName);
    }

    @Override
    public Hashtable<?, ?> getEnvironment() throws NamingException {
        ensureOpen();
        return new Hashtable<>(env);
    }

    @Override
    public void close() {
        closed = true;
        bindings.clear();
        env.clear();
    }

    @Override
    public String getNameInNamespace() throws NamingException {
        return "";
    }
}