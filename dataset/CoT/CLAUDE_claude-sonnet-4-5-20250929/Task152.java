
import java.util.LinkedList;

class Task152 {
    private static final int TABLE_SIZE = 10;
    private LinkedList<Entry>[] table;
    
    static class Entry {
        String key;
        int value;
        
        Entry(String key, int value) {
            this.key = key;
            this.value = value;
        }
    }
    
    @SuppressWarnings("unchecked")
    public Task152() {
        table = new LinkedList[TABLE_SIZE];
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = new LinkedList<>();
        }
    }
    
    private int hash(String key) {
        if (key == null) return 0;
        int hash = 0;
        for (int i = 0; i < key.length(); i++) {
            hash = (hash * 31 + key.charAt(i)) % TABLE_SIZE;
        }
        return Math.abs(hash);
    }
    
    public void insert(String key, int value) {
        if (key == null) {
            System.out.println("Error: Key cannot be null");
            return;
        }
        
        int index = hash(key);
        LinkedList<Entry> bucket = table[index];
        
        // Update if key exists
        for (Entry entry : bucket) {
            if (entry.key.equals(key)) {
                entry.value = value;
                return;
            }
        }
        
        // Insert new entry
        bucket.add(new Entry(key, value));
    }
    
    public Integer search(String key) {
        if (key == null) {
            System.out.println("Error: Key cannot be null");
            return null;
        }
        
        int index = hash(key);
        LinkedList<Entry> bucket = table[index];
        
        for (Entry entry : bucket) {
            if (entry.key.equals(key)) {
                return entry.value;
            }
        }
        
        return null;
    }
    
    public boolean delete(String key) {
        if (key == null) {
            System.out.println("Error: Key cannot be null");
            return false;
        }
        
        int index = hash(key);
        LinkedList<Entry> bucket = table[index];
        
        for (int i = 0; i < bucket.size(); i++) {
            if (bucket.get(i).key.equals(key)) {
                bucket.remove(i);
                return true;
            }
        }
        
        return false;
    }
    
    public static void main(String[] args) {
        Task152 hashTable = new Task152();
        
        // Test Case 1: Insert and search
        System.out.println("Test 1: Insert and Search");
        hashTable.insert("apple", 100);
        hashTable.insert("banana", 200);
        System.out.println("Search 'apple': " + hashTable.search("apple"));
        System.out.println("Search 'banana': " + hashTable.search("banana"));
        
        // Test Case 2: Update existing key
        System.out.println("\\nTest 2: Update Existing Key");
        hashTable.insert("apple", 150);
        System.out.println("Search 'apple' after update: " + hashTable.search("apple"));
        
        // Test Case 3: Delete operation
        System.out.println("\\nTest 3: Delete Operation");
        System.out.println("Delete 'banana': " + hashTable.delete("banana"));
        System.out.println("Search 'banana' after delete: " + hashTable.search("banana"));
        
        // Test Case 4: Search non-existent key
        System.out.println("\\nTest 4: Search Non-existent Key");
        System.out.println("Search 'orange': " + hashTable.search("orange"));
        
        // Test Case 5: Null key handling
        System.out.println("\\nTest 5: Null Key Handling");
        hashTable.insert(null, 300);
        System.out.println("Search null: " + hashTable.search(null));
        System.out.println("Delete null: " + hashTable.delete(null));
    }
}
