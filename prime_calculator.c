#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <math.h>
/*CONCURRENT PROGRAMMING by: Nur Rustempašić, Nedžla Šehović and Nejira Subašić
STRUCTURE:
1.NON OBVIOUS PART
2.STRUCT DEFINITION 
3.CALCULATING PRIME NUMBERS
4.CALCULATING PRIME NUMBERS RANGE
5. SLAVE FUNCTION
6. PROCESS FUNCTION
7. MAIN 
  7.A. PARSING
  7.B. PROCESS PART
  7.B. THREAD PART
*/

// 1. NON OBVIOUS PART:
// In the thread-based approach, the calculate_primes function, originally designed for processes, is adapted to threads.
// The challenge lies in updating the global array with computed primes since threads share the same memory space.
// Here, the struct slave_info is used to pass thread-specific information, including the thread's ID and the range to calculate primes.
// The calculated primes are then stored in the global array, ensuring proper synchronization among threads.

// Define a constant for the maximum length of the method argument
#define METHOD_MAX_LENGTH 10

unsigned long max_prime;
int n_slaves;
unsigned long *prime_results;  // Global array to store prime numbers


// 2. STRUCT DEFINITION for storing information related to each process or thread
// - 'slave_num': An integer representing the identifier of the process or thread.
// - 'primes_count': An unsigned long variable to store the number of prime numbers computed by the process or thread.
// - 'start_time': A struct timespec variable to record the start time of the process or thread execution.
// - 'end_time': A struct timespec variable to record the end time of the process or thread execution.
struct slave_info {
    int slave_num;
    unsigned long primes_count;  // Variable to store the number of primes computed by the slave
    struct timespec start_time;
    struct timespec end_time;
};

// 3. CALCULATING PRIME NUMBERS
// Function to calculate prime numbers within a specific range for a given process or thread
// - 'slave_id': An integer identifier for the current process or thread.
// - 'num_slaves': The total number of processes or threads.
// - 'max_prime': The maximum value up to which prime numbers are calculated.
// - Returns the count of prime numbers within the specified range.
unsigned long calculate_primes(int slave_id, int num_slaves, unsigned long max_prime) {
    unsigned long num, i, primes = 0, flag;
    // Initialize 'num' to the starting value for the current process or thread
    num = 3 + 2 * slave_id;
    // Iterate through the range of numbers, checking for prime numbers
    while (num < max_prime) {
        flag = 0;
        // Check for factors within the second half of the current 'num'
        for (i = 2; i <= num / 2; i++) {
            if (num % i == 0) {
                flag = 1;
                break;
            }
        }
        // If 'num' is prime, increment the 'primes' count
        if (flag == 0 && (num > 1)) {
            ++primes;
        }
        // Move to the next number in the range for the current process or thread
        num += 2 * num_slaves;
    }
    // Return the total count of prime numbers within the specified range
    return primes;
}

// 4. CALCULATING PRIME NUMBERS RANGE
// Function to calculate prime numbers within a specific range for a given process or thread
// and update the global array with the computed primes.
// - 'slave_id': An integer identifier for the current process or thread.
// - 'num_slaves': The total number of processes or threads.
// - 'start': The starting value for the range of numbers.
// - 'end': The ending value for the range of numbers.
void calculate_primes_range(int slave_id, int num_slaves, unsigned long start, unsigned long end) {
    // Call the 'calculate_primes' function to compute the prime numbers within the specified range
    unsigned long primes = calculate_primes(slave_id, num_slaves, end);  // Call the provided function

    // Update the global array with the calculated primes
    prime_results[slave_id] = primes;
}

// 5. SLAVE FUNCTION
// Function executed by each thread to calculate prime numbers within a specific range
// and record the start and end times of the thread execution.
// - 'arg': A void pointer to a struct containing information about the current thread.
//          Casted to 'struct slave_info*' to access thread-specific data.
void *slave_function(void *arg) {
    // Extract information about the current thread from the argument
    struct slave_info *info = (struct slave_info *)arg;

    // Calculate the range for the current slave
    unsigned long start = 3 + 2 * info->slave_num;
    unsigned long end = max_prime;

    // Record the start time of the thread execution
    clock_gettime(CLOCK_MONOTONIC, &(info->start_time));

    // Call the function to calculate primes within the specified range and update the global array
    calculate_primes_range(info->slave_num, n_slaves, start, end);

    // Record the end time of the thread execution
    clock_gettime(CLOCK_MONOTONIC, &(info->end_time));

    // Exit the thread
    pthread_exit(NULL);
    return NULL;
}

// 6. PROCESS FUNCTION
// Function executed by each process to calculate prime numbers within a specific range
// and print the results including the execution time.
// - 'process_id': An integer identifier for the current process.
// - 'num_processes': The total number of processes.
void process_function(int process_id, int num_processes) {
    // Calculate the range for the current process
    unsigned long start = 3 + 2 * process_id;
    unsigned long end = max_prime;

    // Record the start time of the process execution
    clock_t start_time = clock();

    // Call the function to calculate primes within the specified range and update the global array
    calculate_primes_range(process_id, num_processes, start, end);

    // Record the end time of the process execution
    clock_t end_time = clock();

    // Print the results, including the count of prime numbers and the execution time for the process
    printf("Process %d computed %lu prime numbers in %.0f milliseconds\n", process_id, prime_results[process_id], (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC);
}

// 7. MAIN 
int main(int argc, char *argv[]) {
    // Check if the correct number of command-line arguments is provided
    // If not, print an error message indicating the correct usage and exit the program.
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <max_prime> <n_slaves> <process/thread>\n", argv[0]);
        return -1;
    }

    // 7.A. PARSING
    // Parse command-line arguments to obtain the maximum prime value, the number of processes/threads, and allocate memory for the results array.
    // - 'strtoul': Convert the string representation of the maximum prime to an unsigned long.
    // - 'atoi': Convert the string representation of the number of processes/threads to an integer.
    // - Allocate memory for the array to store prime results based on the number of processes/threads.
    max_prime = strtoul(argv[1], NULL, 10);
    n_slaves = atoi(argv[2]);
    prime_results = (unsigned long *)malloc(n_slaves * sizeof(unsigned long));

    // Check if memory allocation for the prime_results array was successful
    // If allocation failed, print an error message using perror, indicating the failure reason, and exit the program with an error code.
    if (prime_results == NULL) {
        perror("Error allocating memory for prime_results array");
        return -1;
    }

    // Initialize a character array 'method' to store the specified method (process/thread)
    // - 'METHOD_MAX_LENGTH': A constant representing the maximum length of the method argument.
    // - Copy the method argument from the command line to the 'method' array using 'strncpy'.
    char method[METHOD_MAX_LENGTH];
    strncpy(method, argv[3], METHOD_MAX_LENGTH);

    //7.B. PROCESS PART
    // Check if the specified method is "process"
    // If true, initiate the process-based parallelization and record the start time for the entire program.
    // - 'strcmp': Compare the 'method' with the string "process".
    // - 'clock_t program_start_time': Variable to store the start time for the entire program.
    // - 'struct slave_info processes[n_slaves]': Array of 'struct slave_info' to store information about each process.
    if (strcmp(method, "process") == 0) {
         clock_t program_start_time = clock();  // Record the start time for the entire program
        struct slave_info processes[n_slaves];

        // Iterate through the number of processes specified
        // For each iteration, initialize process-specific information and fork a child process.
        // - 'processes[i].slave_num': Set the process identifier for the current iteration.
        // - 'pid_t child_pid = fork();': Fork a child process and obtain its process ID.
        //   - If 'child_pid' is 0, execute the child process and exit.
        //   - If 'child_pid' is negative, print an error message and exit the program.
        //   - For the parent process, initialize start and end times for the current process.
        for (int i = 0; i < n_slaves; ++i) {
            processes[i].slave_num = i;

            pid_t child_pid = fork();

            if (child_pid == 0) {
                // Child process
                process_function(i, n_slaves);
                exit(0);
            } else if (child_pid < 0) {
                // Error handling for fork failure
                fprintf(stderr, "Error forking process %d\n", i);
                free(prime_results);
                return -1;
            } else {
                // Parent process
                processes[i].start_time.tv_sec = 0;
                processes[i].end_time.tv_sec = 0;
            }
        }

        // Wait for each child process to finish and record the end time for each process.
        // - 'waitpid(-1, NULL, 0)': Wait for any child process to finish.
        // - For each iteration, record the end time for the corresponding process.
        for (int i = 0; i < n_slaves; ++i) {
            waitpid(-1, NULL, 0); // Wait for any child process to finish
            clock_gettime(CLOCK_MONOTONIC, &(processes[i].end_time));
        }
    clock_t program_end_time = clock();  // Record the end time for the entire program

    // Print the summary of the program execution, including the total time taken.
    // - 'max_prime': The maximum prime value specified in the command line.
    // - 'n_slaves': The number of child processes used.
    // - '(double)(program_end_time - program_start_time) * 1000.0 / CLOCKS_PER_SEC':
    //   Calculate and print the total execution time in milliseconds.
    printf("This machine calculated all prime numbers under %lu using %d children in %.0f milliseconds\n", max_prime, n_slaves, (double)(program_end_time - program_start_time) * 1000.0 / CLOCKS_PER_SEC);
    
    //7.B. THREAD PART
    // Check if the specified method is "thread"
    // If true, initiate the thread-based parallelization.
    // - 'strcmp': Compare the 'method' with the string "thread".
    // - 'struct slave_info threads[n_slaves]': Array of 'struct slave_info' to store information about each thread.
    // - 'pthread_t thread_ids[n_slaves]': Array to store thread IDs.
    } else if (strcmp(method, "thread") == 0) {
        struct slave_info threads[n_slaves];
        pthread_t thread_ids[n_slaves];

        // Iterate through the number of threads specified
        // For each iteration, initialize thread-specific information, setting the thread identifier.
        // - 'threads[i].slave_num': Set the thread identifier for the current iteration.
        for (int i = 0; i < n_slaves; ++i) {
            threads[i].slave_num = i;
    
            // Create threads and check for any errors in the thread creation process
            // If an error occurs, print an error message, and exit the program with an error code.
            // - 'pthread_create': Create a new thread and execute the 'slave_function'.
            // - If 'pthread_create' returns an error (non-zero), print an error message and exit the program.
            if (pthread_create(&(thread_ids[i]), NULL, &slave_function, &(threads[i])) != 0) {
                fprintf(stderr, "Error creating thread %d\n", i);
                return -1;
            }
        }

        // Wait for all threads to finish
        for (int i = 0; i < n_slaves; ++i) {
            pthread_join(thread_ids[i], NULL);
        }

// Process and display the results after executing threads or processes
// - 'double total_time': Variable to store the total execution time of all threads or processes.
// - Iterate through each thread or process, updating 'total_time' with the maximum execution time.
// - Print individual thread or process results, including the count of prime numbers and execution time.
// - Print the summary of the program execution, specifying the method used, the total execution time, and the specified maximum prime value.
        double total_time = 0;
        for (int i = 0; i < n_slaves; ++i) {
            total_time = fmax(total_time, (double)(threads[i].end_time.tv_sec - threads[i].start_time.tv_sec) * 1000.0 +
                          (threads[i].end_time.tv_nsec - threads[i].start_time.tv_nsec) / 1000000.0);

            printf("Thread %d computed %lu prime numbers in %.0f milliseconds\n", i, prime_results[i], (double)(threads[i].end_time.tv_sec - threads[i].start_time.tv_sec) * 1000.0 +
                          (threads[i].end_time.tv_nsec - threads[i].start_time.tv_nsec) / 1000000.0);
        }

        printf("This machine calculated all prime numbers under %lu using %d threads in %.0f milliseconds\n", max_prime, n_slaves, total_time);
    } else {
        fprintf(stderr, "Invalid argument for process/thread. Use 'process' for process or 'thread' for thread.\n");
        free(prime_results);
        return -1;
    }

    // Clean up and free allocated memory
    free(prime_results);

    return 0;
}

