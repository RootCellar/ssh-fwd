#ifndef MEMORY_H
#define MEMORY_H

#define DEFAULT_POINTER_LIST_SIZE 16384

//unsigned long int BYTES_ALLOCATED = 0;
struct ptr_data {
  void* ptr;
  unsigned long int size;
};

struct ptr_data* POINTER_LIST = 0;
int POINTER_LIST_SIZE = 0;

int tFindSpot(void* ptr) {
  for(int i = 0; i < POINTER_LIST_SIZE; i++) {
    if(POINTER_LIST[i].ptr == ptr) {
      return i;
    }
  }

  return -1;
}

int tAdd(void* ptr, unsigned long int len) {
  if(POINTER_LIST <= 0) {
    POINTER_LIST = malloc( sizeof(struct ptr_data) * DEFAULT_POINTER_LIST_SIZE );
    POINTER_LIST_SIZE = DEFAULT_POINTER_LIST_SIZE;
  }

  int spot = tFindSpot(0);
  if(spot < 0) {
    return 1;
  }

  struct ptr_data* ptrData= &POINTER_LIST[spot];
  ptrData->ptr = ptr;
  ptrData->size = len;

  return 0;
}

void* tMalloc(unsigned long int len) {
  debug_printf("Allocating %lu bytes\n", len);

  void* toRet = malloc(len);

  if(toRet > 0) {
    int failed = tAdd(toRet, len);
    if(failed) {
      debug_print("Could not add pointer to list!");
      free(toRet);
      toRet = 0;
    }
  }

  return toRet;
}

int tFree(void* ptr) {

  if(ptr <= 0) return 1; // invalid pointer

  debug_print("Attempting to free a pointer\n");

  // Find the pointer in the list
  // if we don't have it, then we don't manage it
  int spot = tFindSpot(ptr);
  if(spot < 0) {
    debug_print("Could not find pointer in list!\n");
    return 1;
  }

  struct ptr_data* ptrData = &POINTER_LIST[spot];

  // Free the pointer, remove it from the list
  free(ptr);
  ptrData->ptr = 0;
  debug_printf("Freed %lu bytes\n", ptrData->size );
  return 0;
}

#endif
