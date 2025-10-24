#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>
#include <xmlrpc-c/client.h>

// Compilation command:
// gcc task.c -o task_server_c $(xmlrpc-c-config abyss-server --cflags) $(xmlrpc-c-config abyss-server client --libs) -pthread

// Global server instance for shutdown
static TServer global_server;
volatile sig_atomic_t server_running = 0;

// Rule #8: Use xmlrpc_env for error handling.
static void die_if_fault_occurred(xmlrpc_env *env) {
    if (env->fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n", env->fault_string, env->fault_code);
        exit(1);
    }
}

// Method handler for "sample.add"
static xmlrpc_value* sample_add(xmlrpc_env*   const env,
                                xmlrpc_value* const param_array,
                                void*         const user_data) {
    xmlrpc_int32 a, b;
    // Rule #3: Validate input types and arguments.
    // Unpack the parameters. xmlrpc-c handles type mismatches.
    xmlrpc_parse_value(env, param_array, "(ii)", &a, &b);
    if (env->fault_occurred) {
        return NULL; // Propagate fault
    }
    return xmlrpc_build_value(env, "i", a + b);
}

// Method handler for "sample.divide"
static xmlrpc_value* sample_divide(xmlrpc_env*   const env,
                                   xmlrpc_value* const param_array,
                                   void*         const user_data) {
    double a, b;
    xmlrpc_parse_value(env, param_array, "(dd)", &a, &b);
    if (env->fault_occurred) {
        return NULL;
    }

    // Rule #3: Validate input value.
    if (b == 0.0) {
        // Rule #8: Set a fault for invalid operations.
        xmlrpc_env_set_fault(env, 1, "Division by zero is not allowed.");
        return NULL;
    }

    return xmlrpc_build_value(env, "d", a / b);
}

// Thread function to run the server
void* server_thread_func(void* arg) {
    int port = *(int*)arg;
    
    xmlrpc_env env;
    xmlrpc_registry* registry;
    
    xmlrpc_env_init(&env);

    registry = xmlrpc_registry_new(&env);
    die_if_fault_occurred(&env);

    // Rule #4: Map specific names to safe, pre-defined C functions.
    xmlrpc_registry_add_method(&env, registry, NULL, "sample.add", &sample_add, NULL);
    die_if_fault_occurred(&env);
    xmlrpc_registry_add_method(&env, registry, NULL, "sample.divide", &sample_divide, NULL);
    die_if_fault_occurred(&env);

    // Rule #6: xmlrpc-c's parser is safe against XXE by default.
    struct xmlrpc_server_abyss_parms server_parms;
    server_parms.config_file_name = NULL;
    server_parms.registryP = registry;
    server_parms.port_number = port;
    server_parms.log_file_name = "/dev/null"; // Suppress log for clean output

    printf("Starting XML-RPC server on port %d\n", port);

    xmlrpc_server_abyss_create(&env, &server_parms, XMLRPC_APSIZE(log_file_name), &global_server);
    die_if_fault_occurred(&env);

    server_running = 1;
    xmlrpc_server_abyss_run_server(global_server);
    
    // This part runs after the server is terminated
    printf("Server has been shut down.\n");
    server_running = 0;
    xmlrpc_registry_free(registry);
    xmlrpc_env_clean(&env);

    return NULL;
}


void run_client_tests(int port) {
    // Wait for the server to be ready
    while(!server_running) {
        usleep(10000); // 10ms
    }

    xmlrpc_env env;
    xmlrpc_value* result;
    const char* server_url = "http://localhost:8080/RPC2";

    printf("\n--- Running Client Test Cases ---\n");
    
    // Test Case 1: Add
    xmlrpc_env_init(&env);
    result = xmlrpc_client_call(&env, server_url, "sample.add", "(ii)", (xmlrpc_int32)10, (xmlrpc_int32)20);
    if (!env.fault_occurred) {
        xmlrpc_int32 sum;
        xmlrpc_read_int(&env, result, &sum);
        printf("Test 1: 10 + 20 = %d\n", sum);
        xmlrpc_DECREF(result);
    } else {
        printf("Test 1 failed: %s\n", env.fault_string);
    }
    xmlrpc_env_clean(&env);

    // Test Case 2: Subtract (using add)
    xmlrpc_env_init(&env);
    result = xmlrpc_client_call(&env, server_url, "sample.add", "(ii)", (xmlrpc_int32)100, (xmlrpc_int32)-33);
    if (!env.fault_occurred) {
        xmlrpc_int32 diff;
        xmlrpc_read_int(&env, result, &diff);
        printf("Test 2: 100 - 33 = %d\n", diff);
        xmlrpc_DECREF(result);
    } else {
        printf("Test 2 failed: %s\n", env.fault_string);
    }
    xmlrpc_env_clean(&env);
    
    // Test Case 3: Divide
    xmlrpc_env_init(&env);
    result = xmlrpc_client_call(&env, server_url, "sample.divide", "(dd)", 10.0, 4.0);
    if (!env.fault_occurred) {
        double quot;
        xmlrpc_read_double(&env, result, &quot);
        printf("Test 3: 10.0 / 4.0 = %f\n", quot);
        xmlrpc_DECREF(result);
    } else {
        printf("Test 3 failed: %s\n", env.fault_string);
    }
    xmlrpc_env_clean(&env);
    
    // Test Case 4: Divide by zero (error)
    xmlrpc_env_init(&env);
    result = xmlrpc_client_call(&env, server_url, "sample.divide", "(dd)", 5.0, 0.0);
    if (env.fault_occurred) {
        printf("Test 4: 5.0 / 0.0 -> Fault: %s\n", env.fault_string);
    } else {
        printf("Test 4 failed (expected a fault).\n");
        xmlrpc_DECREF(result);
    }
    xmlrpc_env_clean(&env);

    // Test Case 5: Method not found (error)
    xmlrpc_env_init(&env);
    result = xmlrpc_client_call(&env, server_url, "sample.multiply", "(ii)", 5, 3);
    if (env.fault_occurred) {
        printf("Test 5: call multiply -> Fault: %s\n", env.fault_string);
    } else {
        printf("Test 5 failed (expected a fault).\n");
        xmlrpc_DECREF(result);
    }
    xmlrpc_env_clean(&env);
}

int main() {
    int port = 8080;
    pthread_t server_thread_id;

    xmlrpc_client_setup_global_consts(NULL);

    if (pthread_create(&server_thread_id, NULL, server_thread_func, &port)) {
        fprintf(stderr, "Error creating server thread\n");
        return 1;
    }

    run_client_tests(port);

    if (server_running) {
        printf("\nShutting down server...\n");
        xmlrpc_server_abyss_terminate(global_server);
    }

    pthread_join(server_thread_id, NULL);

    xmlrpc_client_teardown_global_consts();
    return 0;
}