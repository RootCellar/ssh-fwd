/*
 *
 * Darian Marvel / RootCellar
 * 1/11/2023
 *
 * Memory allocator that keeps track of allocated pointers in a list.
 * This allows allocated bytes and pointers to be kept track of,
 * and for pointers to never really be lost.
 *
 * Does not currently support resizing the pointer list.
 *
*/

#ifndef MEMORY_H
#define MEMORY_H

#define DEFAULT_POINTER_LIST_SIZE 16384

struct ptr_data {
  void* ptr;
  unsigned long int size;
};

struct ptr_data* POINTER_LIST = 0;
int POINTER_LIST_SIZE = 0;

// Returns the number of allocations.
// Could be used to see if a function leaks memory on it's own
int tGetTotalAllocs() {
  unsigned long int count = 0;
  debug_print("Counting all pointers...\n");

  for(int i = 0; i < POINTER_LIST_SIZE; i++) {
    if(POINTER_LIST[i].ptr > 0) {
      debug_printf("Found pointer in slot %d\n", i);
      count += 1;
    }
  }

  return count;
}

unsigned long int tGetTotalAllocSize() {
  unsigned long int sum = 0;
  debug_print("Finding total size of all pointers...\n");

  for(int i = 0; i < POINTER_LIST_SIZE; i++) {
    if(POINTER_LIST[i].ptr > 0) {
      debug_printf("Found pointer in slot %d\n", i);
      sum += POINTER_LIST[i].size;
    }
  }

  return sum;
}

void tPrintStatus() {
  debug_printf("There are %lu bytes allocated, amongst %d pointers.\n", tGetTotalAllocSize(), tGetTotalAllocs());
}

// Find the given pointer.
// Handy trick: tFindSpot(0) can be used to find an empty slot
// to track a new pointer
int tFindSpot(void* ptr) {
  debug_printf("Finding pointer %p\n", ptr);
  for(int i = 0; i < POINTER_LIST_SIZE; i++) {
    if(POINTER_LIST[i].ptr == ptr) {
      debug_printf("Found pointer in slot %d\n", i);
      return i;
    }
  }

  return -1;
}

// Add the given pointer, with len bytes allocated,
// to the list of tracked pointers
int tAdd(void* ptr, unsigned long int len) {
  if(POINTER_LIST <= 0) {
    POINTER_LIST = malloc( sizeof(struct ptr_data) * DEFAULT_POINTER_LIST_SIZE );
    if(POINTER_LIST <= 0) {
      // Could not create pointer list
      EXIT_FAIL();
    }
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

// Allocate memory for a pointer and return the pointer,
// adding it to the tracked list if the memory allocation was successful
void* tMalloc(unsigned long int len) {
  debug_printf("Allocating %lu bytes\n", len);

  void* toRet = malloc(len);

  if(toRet > 0) {
    debug_printf("Successfully allocated %lu bytes\n", len);
    int failed = tAdd(toRet, len);
    if(failed) {
      debug_print("Could not add pointer to list!");
      free(toRet);
      toRet = 0;
    }
    else {
      debug_print("Successfully added pointer to list\n");
      tPrintStatus();
    }
  }

  return toRet;
}

// Free the given pointer and remove it from the list,
// if it is in the list and is a valid pointer
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
  tPrintStatus();
  return 0;
}

#endif
