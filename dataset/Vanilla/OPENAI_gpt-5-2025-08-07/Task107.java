import javax.naming.*;
import javax.naming.spi.*;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.net.URI;

public class Task107 {
    public static void installBuilderOnce() {
        try {
            if (!NamingManager.hasInitialContextFactoryBuilder()) {
                NamingManager.setInitialContextFactoryBuilder(new SimpleICFB());
            }
        } catch (Exception e) {
            // Ignore if already set or not permitted
        }
    }

    public static Context getInitialContext(Properties env) throws NamingException {
        installBuilderOnce();
        Hashtable<String, Object> table = new Hashtable<>();
        for (String name : env.stringPropertyNames()) {
            table.put(name, env.getProperty(name));
        }
        return new InitialContext(table);
    }

    public static void jndiBind(Context ctx, String name, Object obj) throws NamingException {
        ctx.bind(name, obj);
    }

    public static void jndiRebind(Context ctx, String name, Object obj) throws NamingException {
        ctx.rebind(name, obj);
    }

    public static <T> T jndiLookup(Context ctx, String name, Class<T> type) throws NamingException {
        Object obj = ctx.lookup(name);
        return type.cast(obj);
    }

    static class CustomResource {
        final String id;
        final int value;
        CustomResource(String id, int value) { this.id = id; this.value = value; }
        public String toString() { return "CustomResource{id=" + id + ", value=" + value + "}"; }
    }

    static class SimpleContext implements Context {
        private final Map<String,Object> map = new ConcurrentHashMap<>();
        private final Hashtable<String,Object> env;

        SimpleContext(Hashtable<String,?> environment) {
            this.env = new Hashtable<>();
            if (environment != null) {
                for (Map.Entry<String,?> e : environment.entrySet()) {
                    this.env.put(e.getKey(), (Object)e.getValue());
                }
            }
        }

        @Override public Object lookup(String name) throws NamingException {
            if (name == null) throw new InvalidNameException("null");
            if ("".equals(name)) return this;
            if (!map.containsKey(name)) throw new NameNotFoundException(name);
            return map.get(name);
        }
        @Override public Object lookup(Name name) throws NamingException { return lookup(name.toString()); }

        @Override public void bind(String name, Object obj) throws NamingException {
            if (map.putIfAbsent(name, obj) != null) throw new NameAlreadyBoundException(name);
        }
        @Override public void bind(Name name, Object obj) throws NamingException { bind(name.toString(), obj); }

        @Override public void rebind(String name, Object obj) throws NamingException { map.put(name, obj); }
        @Override public void rebind(Name name, Object obj) throws NamingException { rebind(name.toString(), obj); }

        @Override public void unbind(String name) throws NamingException { map.remove(name); }
        @Override public void unbind(Name name) throws NamingException { unbind(name.toString()); }

        @Override public void rename(String oldName, String newName) throws NamingException {
            Object obj = map.remove(oldName);
            if (obj == null) throw new NameNotFoundException(oldName);
            map.put(newName, obj);
        }
        @Override public void rename(Name oldName, Name newName) throws NamingException { rename(oldName.toString(), newName.toString()); }

        @Override public NamingEnumeration<NameClassPair> list(String name) throws NamingException {
            final Iterator<String> it = map.keySet().iterator();
            return new NamingEnumeration<NameClassPair>() {
                @Override public NameClassPair next() { String n = it.next(); return new NameClassPair(n, map.get(n).getClass().getName()); }
                @Override public boolean hasMore() { return it.hasNext(); }
                @Override public void close() {}
                @Override public boolean hasMoreElements() { return hasMore(); }
                @Override public NameClassPair nextElement() { return next(); }
            };
        }
        @Override public NamingEnumeration<NameClassPair> list(Name name) throws NamingException { return list(name.toString()); }

        @Override public NamingEnumeration<Binding> listBindings(String name) throws NamingException {
            final Iterator<Map.Entry<String,Object>> it = map.entrySet().iterator();
            return new NamingEnumeration<Binding>() {
                @Override public Binding next() { Map.Entry<String,Object> e = it.next(); return new Binding(e.getKey(), e.getValue()); }
                @Override public boolean hasMore() { return it.hasNext(); }
                @Override public void close() {}
                @Override public boolean hasMoreElements() { return hasMore(); }
                @Override public Binding nextElement() { return next(); }
            };
        }
        @Override public NamingEnumeration<Binding> listBindings(Name name) throws NamingException { return listBindings(name.toString()); }

        @Override public void destroySubcontext(String name) throws NamingException { map.remove(name); }
        @Override public void destroySubcontext(Name name) throws NamingException { destroySubcontext(name.toString()); }

        @Override public Context createSubcontext(String name) throws NamingException { return this; }
        @Override public Context createSubcontext(Name name) throws NamingException { return this; }

        @Override public Object lookupLink(String name) throws NamingException { return lookup(name); }
        @Override public Object lookupLink(Name name) throws NamingException { return lookup(name); }

        @Override public NameParser getNameParser(String name) throws NamingException {
            return new NameParser() { @Override public Name parse(String name) throws NamingException { return new CompositeName(name); } };
        }
        @Override public NameParser getNameParser(Name name) throws NamingException { return getNameParser(name.toString()); }

        @Override public Name composeName(Name name, Name prefix) throws NamingException {
            Name result = (Name)prefix.clone();
            result.addAll(name);
            return result;
        }
        @Override public String composeName(String name, String prefix) throws NamingException {
            if (prefix == null || prefix.isEmpty()) return name;
            if (name == null || name.isEmpty()) return prefix;
            return prefix + "/" + name;
        }

        @Override public Object addToEnvironment(String propName, Object propVal) throws NamingException { return env.put(propName, propVal); }
        @Override public Object removeFromEnvironment(String propName) throws NamingException { return env.remove(propName); }
        @Override public Hashtable<?, ?> getEnvironment() throws NamingException { return (Hashtable<?,?>)env.clone(); }
        @Override public void close() throws NamingException { map.clear(); }
        @Override public String getNameInNamespace() throws NamingException { return ""; }
    }

    static class SimpleICF implements InitialContextFactory {
        private final Context ctx;
        @SuppressWarnings("unchecked")
        SimpleICF(Hashtable<?,?> env) {
            this.ctx = new SimpleContext((Hashtable<String,?>)env);
        }
        @Override public Context getInitialContext(Hashtable<?,?> environment) { return ctx; }
    }

    static class SimpleICFB implements InitialContextFactoryBuilder {
        @Override public InitialContextFactory createInitialContextFactory(Hashtable<?,?> env) {
            return new SimpleICF(env);
        }
    }

    public static void main(String[] args) throws Exception {
        Properties env = new Properties();
        Context ctx = getInitialContext(env);

        jndiBind(ctx, "config/appName", "MyApp");
        String appName = jndiLookup(ctx, "config/appName", String.class);
        System.out.println("Test1: " + appName);

        jndiBind(ctx, "limits/maxUsers", Integer.valueOf(100));
        Integer maxUsers = jndiLookup(ctx, "limits/maxUsers", Integer.class);
        System.out.println("Test2: " + maxUsers);

        jndiBind(ctx, "service/url", new URI("https://example.com/api"));
        URI serviceUrl = jndiLookup(ctx, "service/url", URI.class);
        System.out.println("Test3: " + serviceUrl);

        jndiBind(ctx, "cache/enabled", Boolean.TRUE);
        Boolean cacheEnabled = jndiLookup(ctx, "cache/enabled", Boolean.class);
        System.out.println("Test4: " + cacheEnabled);

        jndiBind(ctx, "object/custom", new CustomResource("res1", 42));
        CustomResource cr = jndiLookup(ctx, "object/custom", CustomResource.class);
        System.out.println("Test5: " + cr);
    }
}