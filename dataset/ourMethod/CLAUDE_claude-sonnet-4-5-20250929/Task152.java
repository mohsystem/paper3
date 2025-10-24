
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class Task152 {
    private static final int DEFAULT_CAPACITY = 16;
    private static final double LOAD_FACTOR = 0.75;
    private static final int MAX_CAPACITY = 1 << 30;

    private static class Entry {
        private final String key;
        private String value;
        private Entry next;

        public Entry(String key, String value) {
            if (key == null) {
                throw new IllegalArgumentException("Key cannot be null");
            }
            this.key = key;
            this.value = value;
            this.next = null;
        }

        public String getKey() {
            return key;
        }

        public String getValue() {
            return value;
        }

        public void setValue(String value) {
            this.value = value;
        }

        public Entry getNext() {
            return next;
        }

        public void setNext(Entry next) {
            this.next = next;
        }
    }

    private List<Entry> table;
    private int size;
    private int capacity;

    public Task152() {
        this.capacity = DEFAULT_CAPACITY;
        this.table = new ArrayList<>(capacity);
        for (int i = 0; i < capacity; i++) {
            table.add(null);
        }
        this.size = 0;
    }

    private int hash(String key) {
        if (key == null) {
            throw new IllegalArgumentException("Key cannot be null");
        }
        byte[] bytes = key.getBytes(StandardCharsets.UTF_8);
        int hash = 0;
        for (byte b : bytes) {
            hash = 31 * hash + (b & 0xFF);
        }
        return Math.abs(hash) % capacity;
    }

    public void insert(String key, String value) {
        if (key == null) {
            throw new IllegalArgumentException("Key cannot be null");
        }
        if (value == null) {
            throw new IllegalArgumentException("Value cannot be null");
        }
        if (key.length() > 1000) {
            throw new IllegalArgumentException("Key too long");
        }

        if (size >= capacity * LOAD_FACTOR) {
            resize();
        }

        int index = hash(key);
        Entry head = table.get(index);

        Entry current = head;
        while (current != null) {
            if (current.getKey().equals(key)) {
                current.setValue(value);
                return;
            }
            current = current.getNext();
        }

        Entry newEntry = new Entry(key, value);
        newEntry.setNext(head);
        table.set(index, newEntry);
        size++;
    }

    public String search(String key) {
        if (key == null) {
            throw new IllegalArgumentException("Key cannot be null");
        }
        if (key.length() > 1000) {
            throw new IllegalArgumentException("Key too long");
        }

        int index = hash(key);
        Entry current = table.get(index);

        while (current != null) {
            if (current.getKey().equals(key)) {
                return current.getValue();
            }
            current = current.getNext();
        }

        return null;
    }

    public boolean delete(String key) {
        if (key == null) {
            throw new IllegalArgumentException("Key cannot be null");
        }
        if (key.length() > 1000) {
            throw new IllegalArgumentException("Key too long");
        }

        int index = hash(key);
        Entry current = table.get(index);
        Entry prev = null;

        while (current != null) {
            if (current.getKey().equals(key)) {
                if (prev == null) {
                    table.set(index, current.getNext());
                } else {
                    prev.setNext(current.getNext());
                }
                size--;
                return true;
            }
            prev = current;
            current = current.getNext();
        }

        return false;
    }

    private void resize() {
        if (capacity >= MAX_CAPACITY) {
            return;
        }

        int newCapacity = capacity * 2;
        if (newCapacity < 0 || newCapacity > MAX_CAPACITY) {
            newCapacity = MAX_CAPACITY;
        }

        List<Entry> oldTable = table;
        int oldCapacity = capacity;
        capacity = newCapacity;
        table = new ArrayList<>(capacity);
        for (int i = 0; i < capacity; i++) {
            table.add(null);
        }
        size = 0;

        for (int i = 0; i < oldCapacity; i++) {
            Entry current = oldTable.get(i);
            while (current != null) {
                insert(current.getKey(), current.getValue());
                current = current.getNext();
            }
        }
    }

    public int getSize() {
        return size;
    }

    public static void main(String[] args) {
        Task152 hashTable = new Task152();

        System.out.println("Test 1: Insert and search");
        hashTable.insert("name", "Alice");
        hashTable.insert("age", "30");
        System.out.println("name: " + hashTable.search("name"));
        System.out.println("age: " + hashTable.search("age"));

        System.out.println("\\nTest 2: Update existing key");
        hashTable.insert("name", "Bob");
        System.out.println("name after update: " + hashTable.search("name"));

        System.out.println("\\nTest 3: Delete operation");
        boolean deleted = hashTable.delete("age");
        System.out.println("Deleted age: " + deleted);
        System.out.println("age after delete: " + hashTable.search("age"));

        System.out.println("\\nTest 4: Search non-existent key");
        System.out.println("non-existent: " + hashTable.search("non-existent"));

        System.out.println("\\nTest 5: Multiple operations");
        hashTable.insert("city", "NYC");
        hashTable.insert("country", "USA");
        hashTable.insert("zip", "10001");
        System.out.println("Size: " + hashTable.getSize());
        System.out.println("city: " + hashTable.search("city"));
        hashTable.delete("country");
        System.out.println("Size after delete: " + hashTable.getSize());
    }
}
