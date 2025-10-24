
import java.util.*;

public class Task162 {
    static class Edge {
        int target;
        int weight;
        
        Edge(int target, int weight) {
            this.target = target;
            this.weight = weight;
        }
    }
    
    static class Node implements Comparable<Node> {
        int vertex;
        int distance;
        
        Node(int vertex, int distance) {
            this.vertex = vertex;
            this.distance = distance;
        }
        
        public int compareTo(Node other) {
            return Integer.compare(this.distance, other.distance);
        }
    }
    
    public static int[] dijkstra(List<List<Edge>> graph, int start, int n) {
        int[] distances = new int[n];
        Arrays.fill(distances, Integer.MAX_VALUE);
        distances[start] = 0;
        
        PriorityQueue<Node> pq = new PriorityQueue<>();
        pq.offer(new Node(start, 0));
        
        boolean[] visited = new boolean[n];
        
        while (!pq.isEmpty()) {
            Node current = pq.poll();
            int u = current.vertex;
            
            if (visited[u]) continue;
            visited[u] = true;
            
            for (Edge edge : graph.get(u)) {
                int v = edge.target;
                int weight = edge.weight;
                
                if (!visited[v] && distances[u] != Integer.MAX_VALUE && 
                    distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    pq.offer(new Node(v, distances[v]));
                }
            }
        }
        
        return distances;
    }
    
    public static void main(String[] args) {
        // Test Case 1: Simple graph
        int n1 = 5;
        List<List<Edge>> graph1 = new ArrayList<>();
        for (int i = 0; i < n1; i++) graph1.add(new ArrayList<>());
        graph1.get(0).add(new Edge(1, 4));
        graph1.get(0).add(new Edge(2, 1));
        graph1.get(2).add(new Edge(1, 2));
        graph1.get(1).add(new Edge(3, 1));
        graph1.get(2).add(new Edge(3, 5));
        graph1.get(3).add(new Edge(4, 3));
        
        System.out.println("Test Case 1:");
        System.out.println("Start node: 0");
        System.out.println("Shortest distances: " + Arrays.toString(dijkstra(graph1, 0, n1)));
        
        // Test Case 2: Disconnected node
        int n2 = 4;
        List<List<Edge>> graph2 = new ArrayList<>();
        for (int i = 0; i < n2; i++) graph2.add(new ArrayList<>());
        graph2.get(0).add(new Edge(1, 5));
        graph2.get(1).add(new Edge(2, 3));
        
        System.out.println("\\nTest Case 2:");
        System.out.println("Start node: 0");
        System.out.println("Shortest distances: " + Arrays.toString(dijkstra(graph2, 0, n2)));
        
        // Test Case 3: Complete graph
        int n3 = 3;
        List<List<Edge>> graph3 = new ArrayList<>();
        for (int i = 0; i < n3; i++) graph3.add(new ArrayList<>());
        graph3.get(0).add(new Edge(1, 2));
        graph3.get(0).add(new Edge(2, 5));
        graph3.get(1).add(new Edge(2, 1));
        
        System.out.println("\\nTest Case 3:");
        System.out.println("Start node: 0");
        System.out.println("Shortest distances: " + Arrays.toString(dijkstra(graph3, 0, n3)));
        
        // Test Case 4: Single node
        int n4 = 1;
        List<List<Edge>> graph4 = new ArrayList<>();
        for (int i = 0; i < n4; i++) graph4.add(new ArrayList<>());
        
        System.out.println("\\nTest Case 4:");
        System.out.println("Start node: 0");
        System.out.println("Shortest distances: " + Arrays.toString(dijkstra(graph4, 0, n4)));
        
        // Test Case 5: Multiple paths
        int n5 = 6;
        List<List<Edge>> graph5 = new ArrayList<>();
        for (int i = 0; i < n5; i++) graph5.add(new ArrayList<>());
        graph5.get(0).add(new Edge(1, 7));
        graph5.get(0).add(new Edge(2, 9));
        graph5.get(0).add(new Edge(5, 14));
        graph5.get(1).add(new Edge(2, 10));
        graph5.get(1).add(new Edge(3, 15));
        graph5.get(2).add(new Edge(3, 11));
        graph5.get(2).add(new Edge(5, 2));
        graph5.get(3).add(new Edge(4, 6));
        graph5.get(4).add(new Edge(5, 9));
        
        System.out.println("\\nTest Case 5:");
        System.out.println("Start node: 0");
        System.out.println("Shortest distances: " + Arrays.toString(dijkstra(graph5, 0, n5)));
    }
}
