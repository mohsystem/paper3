import javax.naming.Context;
import javax.naming.Name;
import javax.naming.NameAlreadyBoundException;
import javax.naming.NameNotFoundException;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.OperationNotSupportedException;
import javax.naming.spi.InitialContextFactory;
import javax.naming.directory.Attributes;
import java.util.Hashtable;
import java.util.Map;
import java.util.Set;
import java.util.Collections;
import java.util.Arrays;
import java.util.HashSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import javax.naming.InitialContext;

public class Task107 {

    // Simple immutable configuration object
    static final class AppConfig {
        private final String appName;
        private final String env;
        private final int maxConn;

        AppConfig(String appName, String env, int maxConn) {
            this.appName = appName;
            this.env = env;
            this.maxConn = maxConn;
        }

        public String getAppName() { return appName; }
        public String getEnv() { return env; }
        public int getMaxConn() { return maxConn; }

        @Override
        public String toString() {
            return "AppConfig{appName='" + appName + "', env='" + env + "', maxConn=" + maxConn + "}";
        }
    }

    // Simple service interface to demonstrate typed lookup
    interface CounterService {
        long incrementAndGet();
        long get();
    }

    static final class InMemoryCounter implements CounterService {
        private final java.util.concurrent.atomic.AtomicLong value = new java.util.concurrent.atomic.AtomicLong(0L);
        @Override public long incrementAndGet() { return value.incrementAndGet(); }
        @Override public long get() { return value.get(); }
        @Override public String toString() { return "InMemoryCounter(" + value.get() + ")"; }
    }

    // Secure name validation
    private static final Pattern SAFE_NAME = Pattern.compile("^[a-zA-Z0-9._/-]+$");
    private static void validateNameOrThrow(String name) throws NamingException {
        if (name == null || name.isEmpty()) {
            throw new NamingException("Name must not be null or empty");
        }
        if (!SAFE_NAME.matcher(name).matches()) {
            throw new NamingException("Illegal JNDI name: " + name);
        }
        if (name.contains("..") || name.contains("//") || name.contains(":") || name.contains("\\"))
            throw new NamingException("Suspicious JNDI name: " + name);
    }

    // Safe bind with whitelist
    public static void safeBind(Context ctx, String name, Object obj, Set<String> allowList) throws NamingException {
        validateNameOrThrow(name);
        if (allowList == null || !allowList.contains(name)) {
            throw new NamingException("Binding not permitted for name: " + name);
        }
        // Avoid rebinding over an existing name unintentionally
        try {
            Object existing = ctx.lookup(name);
            if (existing != null) {
                throw new NameAlreadyBoundException("Name already bound: " + name);
            }
        } catch (NameNotFoundException ignored) {
            // expected if not bound
        }
        ctx.bind(name, obj);
    }

    // Safe rebind, explicitly allowed
    public static void safeRebind(Context ctx, String name, Object obj, Set<String> allowList) throws NamingException {
        validateNameOrThrow(name);
        if (allowList == null || !allowList.contains(name)) {
            throw new NamingException("Rebinding not permitted for name: " + name);
        }
        ctx.rebind(name, obj);
    }

    // Safe lookup with whitelist and strict typing
    public static <T> T safeLookup(Context ctx, String name, Class<T> type, Set<String> allowList) throws NamingException {
        validateNameOrThrow(name);
        if (allowList == null || !allowList.contains(name)) {
            throw new NamingException("Lookup not permitted for name: " + name);
        }
        Object obj = ctx.lookup(name);
        if (obj == null) {
            throw new NameNotFoundException("Not found: " + name);
        }
        if (!type.isInstance(obj)) {
            throw new NamingException("Type mismatch for name '" + name + "': expected " + type.getName() + ", found " + obj.getClass().getName());
        }
        return type.cast(obj);
    }

    // In-memory JNDI InitialContextFactory
    public static final class MemoryContextFactory implements InitialContextFactory {
        @Override
        public Context getInitialContext(Hashtable<?, ?> environment) {
            return MemoryContext.getInstance(environment);
        }
    }

    // Minimal in-memory Context implementation (thread-safe)
    static final class MemoryContext implements Context {
        private static final MemoryContext SINGLETON = new MemoryContext();
        private final Map<String, Object> bindings = new ConcurrentHashMap<>();
        private final Hashtable<String, Object> env = new Hashtable<>();

        private MemoryContext() {}

        public static MemoryContext getInstance(Hashtable<?, ?> environment) {
            if (environment != null) {
                for (Map.Entry<?, ?> e : environment.entrySet()) {
                    Object k = e.getKey();
                    Object v = e.getValue();
                    if (k instanceof String && v != null) {
                        // Security: ignore any object factory settings
                        if (!"java.naming.factory.object".equals(k)) {
                            SINGLETON.env.put((String) k, v);
                        }
                    }
                }
            }
            return SINGLETON;
        }

        private static String normalize(String name) throws NamingException {
            if (name == null) throw new NamingException("Null name");
            String n = name.trim();
            if (n.startsWith("/")) n = n.substring(1);
            return n;
        }

        @Override public Object lookup(Name name) throws NamingException { return lookup(name.toString()); }
        @Override public Object lookup(String name) throws NamingException {
            String key = normalize(name);
            Object val = bindings.get(key);
            if (val == null) throw new NameNotFoundException("Name not found: " + key);
            return val;
        }

        @Override public void bind(Name name, Object obj) throws NamingException { bind(name.toString(), obj); }
        @Override public void bind(String name, Object obj) throws NamingException {
            String key = normalize(name);
            if (bindings.putIfAbsent(key, obj) != null) throw new NameAlreadyBoundException("Already bound: " + key);
        }

        @Override public void rebind(Name name, Object obj) throws NamingException { rebind(name.toString(), obj); }
        @Override public void rebind(String name, Object obj) throws NamingException {
            String key = normalize(name);
            bindings.put(key, obj);
        }

        @Override public void unbind(Name name) throws NamingException { unbind(name.toString()); }
        @Override public void unbind(String name) throws NamingException {
            String key = normalize(name);
            if (bindings.remove(key) == null) throw new NameNotFoundException("Not bound: " + key);
        }

        @Override public void rename(Name oldName, Name newName) throws NamingException { rename(oldName.toString(), newName.toString()); }
        @Override public void rename(String oldName, String newName) throws NamingException {
            String oldKey = normalize(oldName);
            String newKey = normalize(newName);
            Object obj = bindings.remove(oldKey);
            if (obj == null) throw new NameNotFoundException("Not bound: " + oldKey);
            if (bindings.putIfAbsent(newKey, obj) != null) {
                // put back original to keep state consistent
                bindings.put(oldKey, obj);
                throw new NameAlreadyBoundException("Already bound: " + newKey);
            }
        }

        // Listing not required for this task; provide minimal unsupported implementations
        @Override public NamingEnumeration<NameClassPair> list(Name name) throws NamingException { throw new OperationNotSupportedException("list not supported"); }
        @Override public NamingEnumeration<NameClassPair> list(String name) throws NamingException { throw new OperationNotSupportedException("list not supported"); }
        @Override public NamingEnumeration<Binding> listBindings(Name name) throws NamingException { throw new OperationNotSupportedException("listBindings not supported"); }
        @Override public NamingEnumeration<Binding> listBindings(String name) throws NamingException { throw new OperationNotSupportedException("listBindings not supported"); }

        @Override public void destroySubcontext(Name name) throws NamingException { throw new OperationNotSupportedException("destroySubcontext not supported"); }
        @Override public void destroySubcontext(String name) throws NamingException { throw new OperationNotSupportedException("destroySubcontext not supported"); }
        @Override public Context createSubcontext(Name name) throws NamingException { throw new OperationNotSupportedException("createSubcontext not supported"); }
        @Override public Context createSubcontext(String name) throws NamingException { throw new OperationNotSupportedException("createSubcontext not supported"); }

        @Override public Object lookupLink(Name name) throws NamingException { return lookup(name); }
        @Override public Object lookupLink(String name) throws NamingException { return lookup(name); }

        @Override public NameParser getNameParser(Name name) throws NamingException { throw new OperationNotSupportedException("getNameParser not supported"); }
        @Override public NameParser getNameParser(String name) throws NamingException { throw new OperationNotSupportedException("getNameParser not supported"); }

        @Override public Name composeName(Name name, Name prefix) throws NamingException { throw new OperationNotSupportedException("composeName not supported"); }
        @Override public String composeName(String name, String prefix) throws NamingException { throw new OperationNotSupportedException("composeName not supported"); }

        @Override public Object addToEnvironment(String propName, Object propVal) {
            return env.put(propName, propVal);
        }
        @Override public Object removeFromEnvironment(String propName) {
            return env.remove(propName);
        }
        @Override public Hashtable<?, ?> getEnvironment() {
            return new Hashtable<>(env);
        }

        @Override public void close() { /* no-op for in-memory */ }
        @Override public String getNameInNamespace() { return ""; }

        // DirContext extension stubs to satisfy Context import path in some environments
        static final class NameClassPair implements javax.naming.NameClassPair {
            public NameClassPair(String name, String className) { super(name, className); }
        }
        static final class Binding extends javax.naming.Binding {
            public Binding(String name, Object obj) { super(name, obj); }
        }

        // Unused, for compilation compatibility in some environments
        public Object lookup(String name, Attributes attrs) throws NamingException { return lookup(name); }
    }

    public static void main(String[] args) throws Exception {
        // Harden JNDI-related system properties (defense-in-depth)
        System.setProperty("com.sun.jndi.ldap.object.trustURLCodebase", "false");
        System.setProperty("com.sun.jndi.rmi.object.trustURLCodebase", "false");
        System.setProperty("java.rmi.server.useCodebaseOnly", "true");

        // Configure InitialContext to use our safe in-memory context
        Hashtable<String, Object> env = new Hashtable<>();
        env.put(Context.INITIAL_CONTEXT_FACTORY, Task107.MemoryContextFactory.class.getName());
        // Explicitly null-out any object factory to avoid accidental use
        env.put("java.naming.factory.object", "");
        Context ctx = new InitialContext(env);

        // Define an allowlist of names permitted for bind/lookup
        Set<String> allowList = Collections.unmodifiableSet(new HashSet<>(Arrays.asList(
                "app/config",
                "app/version",
                "services/counter",
                "flags/featureX",
                "kv/store"
        )));

        // Bind resources safely
        safeBind(ctx, "app/config", new AppConfig("SampleApp", "prod", 32), allowList);
        safeBind(ctx, "app/version", "1.0.0", allowList);
        safeBind(ctx, "services/counter", new InMemoryCounter(), allowList);
        safeBind(ctx, "flags/featureX", Boolean.TRUE, allowList);
        safeBind(ctx, "kv/store", new java.util.concurrent.ConcurrentHashMap<String, String>(), allowList);

        // 5 test cases
        try {
            // 1) Successful lookup: version String
            String version = safeLookup(ctx, "app/version", String.class, allowList);
            System.out.println("Test1 - Version: " + version);
        } catch (Exception e) {
            System.out.println("Test1 - Failed: " + e.getMessage());
        }

        try {
            // 2) Successful lookup: CounterService and increment
            CounterService counter = safeLookup(ctx, "services/counter", CounterService.class, allowList);
            long v1 = counter.incrementAndGet();
            long v2 = counter.incrementAndGet();
            System.out.println("Test2 - Counter after two increments: " + v2 + " (first was " + v1 + ")");
        } catch (Exception e) {
            System.out.println("Test2 - Failed: " + e.getMessage());
        }

        try {
            // 3) Successful lookup: AppConfig
            AppConfig cfg = safeLookup(ctx, "app/config", AppConfig.class, allowList);
            System.out.println("Test3 - Config: " + cfg);
        } catch (Exception e) {
            System.out.println("Test3 - Failed: " + e.getMessage());
        }

        try {
            // 4) Disallowed name lookup (should fail)
            Boolean evil = safeLookup(ctx, "java:comp/env/evil", Boolean.class, allowList);
            System.out.println("Test4 - Unexpected success: " + evil);
        } catch (Exception e) {
            System.out.println("Test4 - Expected failure: " + e.getMessage());
        }

        try {
            // 5) Type mismatch lookup (should fail)
            Integer wrongType = safeLookup(ctx, "app/version", Integer.class, allowList);
            System.out.println("Test5 - Unexpected success: " + wrongType);
        } catch (Exception e) {
            System.out.println("Test5 - Expected failure: " + e.getMessage());
        }
    }
}