
#include <liblmice.h>
#include <stdio.h>
#include <stdlib.h>


#define ERROR printf("error : %s\n", lmice_error_get()); exit(0)
	
#define OK printf("ok\n");

void setdata(LMiceResult *result)
{
    printf("setting data\n");
    int i;
    for (i=0;i<result->count;i++) {
	result->data[i]->raw_x = 2;
	result->data[i]->raw_y = 3;
    }
}

bool testdata(LMiceResult *result)
{
    int i;
    for (i=0;i<result->count;i++) {
	if (result->data[i]->raw_x != 2 || result->data[i]->raw_y != 3)
	    return false;
    }
    return true;
}

void test(int system)
{
    printf("init ...\n");
    int mice = lmice_init(system);
    if (mice < 0) {
	ERROR;
    } 
    OK;
    printf("%d mice found\n", mice);
    
    printf("creating result\n");
    LMiceResult *result = lmice_result_new();
    
    printf("read ... ");
    if (!lmice_read(result, true)) {
	ERROR;
    }
    OK;
    
    printf("count ... ");
    if (lmice_count() < 0) {
	ERROR;
    }
    OK;
    
    printf("lmice_count() == (return) lmcie_init() ...");
    if (lmice_count() != mice) {
	printf("lmice_init returned %d and lmice_count retuned %d\n", mice, lmice_count());
	ERROR;
    }
    OK;
    
    printf("clear ... (void)\n");
    lmice_clear();
    OK;
    
    setdata(result);
    printf("config_set ...");
    if (lmice_config_set(result) == false) {
	ERROR;
    }
    OK;

    printf("config_save ...");
    if (lmice_config_save(NULL) == false) {
	ERROR;
    }
    OK;

    printf("config_load ...");
    if (lmice_config_load(NULL) == false) {
	ERROR;
    }
    OK; 
   
    printf("uninit ... (void)\n");
    lmice_uninit(false);
    OK;
    lmice_result_delete(result);
}

void test_result()
{
    printf("new ...");
    LMiceResult *result;
    result = lmice_result_new();
    if (result == NULL) {
	ERROR;
    }
    OK;
    printf("clear ...");
    if (!lmice_result_clear(result)) {
	ERROR;
    }
    OK;
    printf("dup ...");
    LMiceResult *dupe = lmice_result_dup(result);
    if (!dupe) {
	ERROR;
    }
    OK;
    printf("copy ...");
    if (!lmice_result_copy(dupe, result)) {
	ERROR;
    }
    OK;
    printf("delete 1 ...");
    if (!lmice_result_delete(result)) {
	ERROR;
    }
    OK;
    printf("delete 2 ...");
    if (!lmice_result_delete(dupe)) {
	ERROR;
    }
    OK;	
}

int main()
{
    printf("Fast test for all functions in liblmice.h\n\n");
    printf("TESTING RESULT STORAGES WHILE UNINITIALIZED\n");
    test_result();
    
    printf("TESTING DUMMY\n");
    test(LMICE_SYSTEM_DUMMY);
    printf("TESTING DEVFS\n");
    test(LMICE_SYSTEM_DEVFS);
    printf("TESTING LIBUSB\n");
    test(LMICE_SYSTEM_LIBUSB);
}
