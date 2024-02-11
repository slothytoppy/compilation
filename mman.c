#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define INITIAL_SIZE 10

void *realloc_array(void *ptr, size_t old_size, size_t new_size) {
    // Use mmap to allocate a new memory block
    void *new_ptr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (new_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Copy the contents from the old block to the new block
    memcpy(new_ptr, ptr, old_size);

    // Unmap the old memory block
    if (munmap(ptr, old_size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    return new_ptr;
}

int main() {
    int *arr = mmap(NULL, INITIAL_SIZE * sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (arr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Initialize the array with some values
    for (size_t i = 0; i < INITIAL_SIZE; ++i) {
        arr[i] = i + 1;
    }

    // Use realloc_array to resize the array
    size_t new_size = INITIAL_SIZE * 2;
    arr = realloc_array(arr, INITIAL_SIZE * sizeof(int), new_size * sizeof(int));
    arr[INITIAL_SIZE]=5;
    // Print the resized array
    for (size_t i = 0; i < new_size; ++i) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // Unmap the final memory block
    if (munmap(arr, new_size * sizeof(int)) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    return 0;
}
