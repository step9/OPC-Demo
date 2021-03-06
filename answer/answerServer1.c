#include <signal.h>
#include <stdio.h>
#include "open62541.h"
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

static void
addVariable(UA_Server *server) {
    //Define the attribute of the myInteger variable node //
    UA_VariableAttributes attr;
    UA_VariableAttributes_init(&attr);
    UA_Int32 myInteger = 42;
    UA_Variant_setScalar(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT("en_US","the answer");
    attr.displayName = UA_LOCALIZEDTEXT("en_US","the answer");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;

    //Add the variable node to the information model //
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, "the answer");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NULL, attr, NULL, NULL);
}

static void
writeVariable(UA_Server *server, int answer) {
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");

    srand( (unsigned) time(NULL));
    //Write a different integer value //
    while(1){
        sleep(1);
        answer = rand()%100+1;
        UA_Int32 myInteger = answer;
        UA_Variant myVar;
        UA_Variant_init(&myVar);
        UA_Variant_setScalar(&myVar, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
        UA_Server_writeValue(server, myIntegerNodeId, myVar);

        //Set the status code of the value to an error code. The function
        //  * UA_Server_write provides access to the raw service. The above
        //  * UA_Server_writeValue is syntactic sugar for writing a specific node
        //  * attribute with the write service. //
        UA_WriteValue wv;
        UA_WriteValue_init(&wv);
        wv.nodeId = myIntegerNodeId;
        wv.attributeId = UA_ATTRIBUTEID_VALUE;
        wv.value.status = UA_STATUSCODE_BADNOTCONNECTED;
        wv.value.hasStatus = true;
        UA_Server_write(server, &wv);

        //Reset the variable to a good statuscode with a value //
        wv.value.hasStatus = false;
        wv.value.value = myVar;
        wv.value.hasValue = true;
        UA_Server_write(server, &wv);
        printf("\nwrite: %d success\n",answer);
    }
}

static void
writeWrongVariable(UA_Server *server) {
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");

    //Write a string //
    UA_String myString = UA_STRING("test");
    UA_Variant myVar;
    UA_Variant_init(&myVar);
    printf(">>>> init done\n");
    UA_Variant_setScalar(&myVar, &myString, &UA_TYPES[UA_TYPES_STRING]);
    printf(">>>> set scalar done\n");
    UA_StatusCode retval = UA_Server_writeValue(server, myIntegerNodeId, myVar);
    printf(">>>> write value done\n");
    printf("Writing a string returned statuscode %s\n", UA_StatusCode_name(retval));
    printf(">>>> write wrong value done\n");
}

UA_Boolean running = true;
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}

UA_ServerConfig initConfig(UA_ServerConfig config){
 
    UA_ServerNetworkLayer nl =
        UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, 16664);
    config.networkLayers = &nl;
    config.networkLayersSize = 1;
    return config;
}

static UA_Server* newServer(UA_ServerConfig config,int port){
    UA_Server *server = UA_Server_new(config);
    return server;
}

int runServer(UA_Server *server){
    UA_Server_run(server, &running);
    printf(">>>> run server done\n");
    UA_Server_delete(server);
    return 0;
}

int main_server(void) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_ServerConfig config = UA_ServerConfig_standard;
    UA_ServerNetworkLayer nl =
        UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, 16664);
    config.networkLayers = &nl;
    config.networkLayersSize = 1;
    UA_Server *server = UA_Server_new(config);

    addVariable(server);
    writeVariable(server,44);
    writeWrongVariable(server);

    UA_Server_run(server,&running);
    UA_Server_delete(server);
    nl.deleteMembers(&nl);
    return 0;
}

int main_loop(UA_Server* server){
    // main_server();
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_ServerConfig config = UA_ServerConfig_standard;
    UA_ServerNetworkLayer nl =
        UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, 16664);
    config.networkLayers = &nl;
    config.networkLayersSize = 1;

    server = UA_Server_new(config);
    printf("new server \n");
    addVariable(server);
    printf(">> add variable done \n");
    // writeVariable(server,44);
    printf(">> write variable done \n");
    // writeWrongVariable(server);
    printf(">>> write donw");
    // UA_Boolean running = true;
    runServer(server);
    printf("done");
    return 0;
}
int main(void){

    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_ServerConfig config = UA_ServerConfig_standard;
    UA_ServerNetworkLayer nl =
        UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, 16664);
    config.networkLayers = &nl;
    config.networkLayersSize = 1;

    UA_Server *server = UA_Server_new(config);
    printf("\nnew server \n");
    addVariable(server);

    pthread_t writeThread;
    int ret_thrd = pthread_create(&writeThread, NULL, (void *)&runServer,(void *) server);
    if (ret_thrd != 0){
        printf("\nwrite thread create fail\n");
    }else{
        printf("\nwrite thread create success\n");
    }

    sleep(2);
    writeVariable(server,66);

}
