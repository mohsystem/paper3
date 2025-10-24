public class Task69 {
    public static boolean deleteFile(String path) {
        if (path == null || path.isEmpty()) return false;
        try {
            java.nio.file.Path p = java.nio.file.Paths.get(path);
            return java.nio.file.Files.deleteIfExists(p);
        } catch (Exception e) {
            return false;
        }
    }

    public static void main(String[] args) throws Exception {
        if (args != null && args.length > 0) {
            for (String arg : args) {
                boolean result = deleteFile(arg);
                System.out.println(arg + " -> " + result);
            }
            return;
        }

        java.util.List<String> paths = new java.util.ArrayList<>();
        // Create 3 temp files to delete
        java.nio.file.Path f1 = java.nio.file.Files.createTempFile("Task69_test_1_", ".tmp");
        java.nio.file.Files.write(f1, "data1".getBytes(java.nio.charset.StandardCharsets.UTF_8));
        java.nio.file.Path f2 = java.nio.file.Files.createTempFile("Task69_test_2_", ".tmp");
        java.nio.file.Files.write(f2, "data2".getBytes(java.nio.charset.StandardCharsets.UTF_8));
        java.nio.file.Path f3 = java.nio.file.Files.createTempFile("Task69_test_3_", ".tmp");
        java.nio.file.Files.write(f3, "data3".getBytes(java.nio.charset.StandardCharsets.UTF_8));

        paths.add(f1.toString()); // 1: existing file
        paths.add(f2.toString()); // 2: existing file
        paths.add(f3.toString()); // 3: existing file

        // 4: non-existent file
        String nonExistent = java.nio.file.Paths.get(System.getProperty("java.io.tmpdir"),
                "Task69_nonexistent_" + System.nanoTime() + ".tmp").toString();
        paths.add(nonExistent);

        // 5: try to delete the first file again (already deleted)
        paths.add(f1.toString());

        for (String p : paths) {
            boolean res = deleteFile(p);
            System.out.println(p + " -> " + res);
        }
    }
}