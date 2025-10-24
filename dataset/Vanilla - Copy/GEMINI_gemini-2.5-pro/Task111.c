/*
 * This code requires the XML-RPC for C/C++ (xmlrpc-c) library.
 * You need to install it first. On Debian/Ubuntu:
 * sudo apt-get install libxmlrpc-core-c3-dev
 *
 * To compile and run:
 * gcc -o server.out your_script_name.c $(xmlrpc-c-config abyss-server --cflags --libs)
 * ./server.out
 *
 * A client can then connect to http://localhost:8083/RPC2
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>

#define DIE_IF_FAULT_OCCURRED(env) \
    if ((env)->fault_occurred) { \
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n", \
                (env)->fault_string, (env)->fault_code); \
        exit(1); \
    }

// Test Case 1: Add two integers
static xmlrpc_value * add(xmlrpc_env *   const env,
                          xmlrpc_value * const param_array,
                          void *         const user_data ATTR_UNUSED) {
    xmlrpc_int32 x, y;
    xmlrpc_parse_value(env, param_array, "(ii)", &x, &y);
    DIE_IF_FAULT_OCCURRED(env);
    printf("Called add(%d, %d)\n", x, y);
    return xmlrpc_build_value(env, "i", x + y);
}

// Test Case 2: Subtract two doubles
static xmlrpc_value * subtract(xmlrpc_env *   const env,
                               xmlrpc_value * const param_array,
                               void *         const user_data ATTR_UNUSED) {
    xmlrpc_double d1, d2;
    xmlrpc_parse_value(env, param_array, "(dd)", &d1, &d2);
    DIE_IF_FAULT_OCCURRED(env);
    printf("Called subtract(%f, %f)\n", d1, d2);
    return xmlrpc_build_value(env, "d", d1 - d2);
}

// Test Case 3: Greet a user
static xmlrpc_value * greet(xmlrpc_env *   const env,
                             xmlrpc_value * const param_array,
                             void *         const user_data ATTR_UNUSED) {
    const char *name;
    xmlrpc_parse_value(env, param_array, "(s)", &name);
    DIE_IF_FAULT_OCCURRED(env);
    printf("Called greet(\"%s\")\n", name);
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Hello, %s!", name);
    free((void*)name);
    return xmlrpc_build_value(env, "s", buffer);
}

// Test Case 4: Get a struct
static xmlrpc_value * get_struct(xmlrpc_env *   const env,
                                 xmlrpc_value * const param_array,
                                 void *         const user_data ATTR_UNUSED) {
    const char *name;
    xmlrpc_int32 age;
    xmlrpc_parse_value(env, param_array, "(si)", &name, &age);
    DIE_IF_FAULT_OCCURRED(env);
    printf("Called get_struct(\"%s\", %d)\n", name, age);
    
    xmlrpc_value* result = xmlrpc_build_value(env,
        "{s:s,s:i,s:b}",
        "name", name,
        "age", age,
        "isAdult", age >= 18);
    free((void*)name);
    return result;
}

// Test Case 5: Get an array
static xmlrpc_value * get_array(xmlrpc_env *   const env,
                                xmlrpc_value * const param_array,
                                void *         const user_data ATTR_UNUSED) {
    // No parameters to parse for this one.
    // xmlrpc_parse_value(env, param_array, "()");
    printf("Called get_array()\n");

    return xmlrpc_build_value(env, "[ssss]", "Java", "Python", "CPP", "C");
}


int main(int argc, char **argv) {
    int const port = 8083;
    xmlrpc_server_abyss_parms serverparms;
    xmlrpc_registry * registryP;
    xmlrpc_env env;

    xmlrpc_env_init(&env);

    registryP = xmlrpc_registry_new(&env);
    DIE_IF_FAULT_OCCURRED(&env);

    xmlrpc_registry_add_method(&env, registryP, NULL, "add", &add, NULL);
    DIE_IF_FAULT_OCCURRED(&env);
    xmlrpc_registry_add_method(&env, registryP, NULL, "subtract", &subtract, NULL);
    DIE_IF_FAULT_OCCURRED(&env);
    xmlrpc_registry_add_method(&env, registryP, NULL, "greet", &greet, NULL);
    DIE_IF_FAULT_OCCURRED(&env);
    xmlrpc_registry_add_method(&env, registryP, NULL, "get_struct", &get_struct, NULL);
    DIE_IF_FAULT_OCCURRED(&env);
    xmlrpc_registry_add_method(&env, registryP, NULL, "get_array", &get_array, NULL);
    DIE_IF_FAULT_OCCURRED(&env);
    
    serverparms.config_file_name = NULL;
    serverparms.registryP        = registryP;
    serverparms.port_number      = port;
    serverparms.log_file_name    = "/dev/null"; /* Or a log file path */

    printf("Starting XML-RPC server on port %d...\n", port);
    printf("Available methods: add, subtract, greet, get_struct, get_array\n");

    xmlrpc_server_abyss(&env, &serverparms, XMLRPC_APSIZE(log_file_name));
    DIE_IF_FAULT_OCCURRED(&env);

    /* This part is never reached */
    return 0;
}