/*
 *
 * Darian Marvel / RootCellar
 * 1/11/2023
 *
 * Memory allocator that keeps track of allocated pointers in a list.
 * This allows allocated bytes and pointers to be kept track of,
 * and for pointers to never really be lost.
 *
 * Supports manual list resizing - does not resize automatically.
 *
*/

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#include "debug.h"

#define DEFAULT_POINTER_LIST_SIZE 1024

struct ptr_data {
  void* ptr;

  //unsigned long int size;
  size_t size;
};

struct ptr_data* POINTER_LIST = 0;
int POINTER_LIST_SIZE = 0;

// Determines whether or not the given address is owned
// based on what's in the pointer list
int tOwnsAddress(void* adr) {
  if( !(POINTER_LIST > 0) ) return 0;

  struct ptr_data* current;
  for(int i = 0; i < POINTER_LIST_SIZE; i++) {

    if(POINTER_LIST[i].ptr > 0) {

      current = &POINTER_LIST[i];
      if( (uintptr_t) current->ptr <= (uintptr_t) adr 
      && (uintptr_t) adr < (uintptr_t) (current->ptr + current->size) ) {
        return 1;
      }

    }

  }

  return 0;
}

// Returns the number of allocations.
// Could be used to see if a function leaks memory on it's own
int tGetTotalAllocs() {
  if( !(POINTER_LIST > 0) ) return 0;

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
  if( !(POINTER_LIST > 0) ) return 0;

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
  if( !(POINTER_LIST > 0) ) return -1;

  debug_printf("Finding pointer %p\n", ptr);
  for(int i = 0; i < POINTER_LIST_SIZE; i++) {
    if(POINTER_LIST[i].ptr == ptr) {
      debug_printf("Found pointer in slot %d\n", i);
      return i;
    }
  }

  return -1;
}

// Get the size of the given pointer
// Must be kept tracked of in our list.
// Returns 0 if the size is zero or if the pointer is not found.
size_t tGetSize(void* ptr) {
  int spot = tFindSpot(ptr);
  if(spot < 0) { return 0; }

  return POINTER_LIST[spot].size;
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

  if(len <= 0) {
    debug_printf("Refusing to allocate %lu bytes", len);
    return 0;
  }

  void* toRet = malloc(len);

  if(toRet > 0) {
    debug_printf("Successfully allocated %lu bytes\n", len);
    int failed = tAdd(toRet, len);
    if(failed) {
      debug_print("Could not add pointer to list!\n");
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

  if(ptr <= 0) return 0; // invalid pointer

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

// Resize the pointer list to hold "len" items.
int tResize(int len) {

  int allocs = tGetTotalAllocs();
  debug_printf("Attempting to resize pointer list... %d -> %d\n", allocs, len);
  if(len < allocs) {
    debug_print("Refusing to allocate pointer list to be smaller than current number of allocations\n");
    return 1;
  }

  void* new_pointer_list = malloc(len * sizeof(struct ptr_data));
  if(new_pointer_list <= 0) {
    debug_print("Could not allocate new pointer list!\n");
    return 1;
  }

  if(POINTER_LIST > 0) {
    debug_print("Replacing old pointer list...\n");
    memcpy(new_pointer_list, POINTER_LIST, POINTER_LIST_SIZE);
    free(POINTER_LIST);
  }

  debug_print("Success\n");

  POINTER_LIST_SIZE = len;
  POINTER_LIST = new_pointer_list;
  return 0;
}

#endif
