#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MENU_VIEW_MEMORY 1
#define MENU_VIEW_PAGE_TABLE 2
#define MENU_CREATE_PROCESS 3
#define MENU_EXIT 4

#define INITIAL_PROCESS_LIST_CAPACITY 10
#define INPUT_BUFFER_SIZE 100

typedef struct
{
    int process_id;
    int process_size;
    int number_of_pages;
    int *page_table;
} Process;

typedef struct
{
    unsigned char *memory;
    int total_size;
    int page_size;
    int number_of_frames;
    int *free_frames;
    int free_frame_count;
} PhysicalMemory;

typedef struct
{
    Process *processes;
    int count;
    int capacity;
} ProcessList;

/**
 * Checks if a number is a power of two.
 *
 * @param number The number to check.
 * @return 1 if the number is a power of two, 0 otherwise.
 */
int is_power_of_two(int number);

/**
 * Initializes the physical memory structure.
 *
 * @param phys_mem Pointer to the PhysicalMemory structure to initialize.
 * @param total_size Total size of physical memory in bytes.
 * @param page_size Size of each page/frame in bytes.
 */
void initialize_physical_memory(PhysicalMemory *phys_mem, int total_size, int page_size);

/**
 * Initializes the process list structure.
 *
 * @param proc_list Pointer to the ProcessList structure to initialize.
 */
void initialize_process_list(ProcessList *proc_list);

/**
 * Allocates free frames for a process.
 *
 * @param phys_mem Pointer to the PhysicalMemory structure.
 * @param required_frames Number of frames required.
 * @param allocated_frames Array to store allocated frame indices.
 * @return 1 if allocation is successful, 0 otherwise.
 */
int allocate_frames(PhysicalMemory *phys_mem, int required_frames, int *allocated_frames);

/**
 * Creates a new process, allocates memory, and initializes its page table.
 *
 * @param phys_mem Pointer to the PhysicalMemory structure.
 * @param proc_list Pointer to the ProcessList structure.
 * @param max_process_size Maximum allowed size for a process in bytes.
 */
void create_process(PhysicalMemory *phys_mem, ProcessList *proc_list, int max_process_size);

/**
 * Displays the current state of physical memory, including free frames and frame statuses.
 *
 * @param phys_mem Pointer to the PhysicalMemory structure.
 */
void view_physical_memory(const PhysicalMemory *phys_mem);

/**
 * Displays the page table of a specified process.
 *
 * @param proc_list Pointer to the ProcessList structure.
 */
void view_page_table(const ProcessList *proc_list);

/**
 * Frees all dynamically allocated memory before exiting the program.
 *
 * @param phys_mem Pointer to the PhysicalMemory structure.
 * @param proc_list Pointer to the ProcessList structure.
 */
void free_memory(PhysicalMemory *phys_mem, ProcessList *proc_list);

/**
 * Clears the input buffer to handle invalid inputs.
 */
void clear_input_buffer(void);

int main()
{
    srand((unsigned int)time(NULL));

    PhysicalMemory phys_mem;
    ProcessList proc_list;
    int total_memory_size, page_size, max_process_size;

    printf("=== Memory Paging Simulator ===\n\n");
    printf("Initial Configuration:\n");

    while (1)
    {
        printf("Enter the size of physical memory in bytes (power of 2): ");
        if (scanf("%d", &total_memory_size) != 1)
        {
            printf("Invalid input. Please enter a valid integer.\n");
            clear_input_buffer();
            continue;
        }
        if (!is_power_of_two(total_memory_size))
        {
            printf("Error: Size must be a power of 2.\n");
            continue;
        }
        break;
    }

    while (1)
    {
        printf("Enter the size of a page/frame in bytes (power of 2): ");
        if (scanf("%d", &page_size) != 1)
        {
            printf("Invalid input. Please enter a valid integer.\n");
            clear_input_buffer();
            continue;
        }
        if (!is_power_of_two(page_size))
        {
            printf("Error: Page size must be a power of 2.\n");
            continue;
        }
        if (page_size > total_memory_size)
        {
            printf("Error: Page size cannot exceed total memory size.\n");
            continue;
        }
        break;
    }

    while (1)
    {
        printf("Enter the maximum size of a process in bytes (power of 2): ");
        if (scanf("%d", &max_process_size) != 1)
        {
            printf("Invalid input. Please enter a valid integer.\n");
            clear_input_buffer();
            continue;
        }
        if (!is_power_of_two(max_process_size))
        {
            printf("Error: Maximum process size must be a power of 2.\n");
            continue;
        }
        if (max_process_size > total_memory_size)
        {
            printf("Error: Maximum process size cannot exceed total memory size.\n");
            continue;
        }
        break;
    }

    initialize_physical_memory(&phys_mem, total_memory_size, page_size);
    initialize_process_list(&proc_list);

    int choice;
    while (1)
    {
        printf("\n+------------------------------------------+\n");
        printf("|                MAIN MENU                 |\n");
        printf("+------------------------------------------+\n");
        printf("| 1. View Physical Memory                  |\n");
        printf("| 2. View Process Page Table               |\n");
        printf("| 3. Create Process                        |\n");
        printf("| 4. Exit                                  |\n");
        printf("+------------------------------------------+\n");
        printf("Select an option: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Please enter a valid option.\n");
            clear_input_buffer();
            continue;
        }

        switch (choice)
        {
        case MENU_VIEW_MEMORY:
            view_physical_memory(&phys_mem);
            break;
        case MENU_CREATE_PROCESS:
            create_process(&phys_mem, &proc_list, max_process_size);
            break;
        case MENU_VIEW_PAGE_TABLE:
            view_page_table(&proc_list);
            break;
        case MENU_EXIT:
            printf("Exiting the simulator...\n");
            free_memory(&phys_mem, &proc_list);
            return 0;
        default:
            printf("Invalid option. Please select a valid option from the menu.\n");
        }
    }

    return 0;
}

int is_power_of_two(int number)
{
    return (number > 0) && ((number & (number - 1)) == 0);
}

void initialize_physical_memory(PhysicalMemory *phys_mem, int total_size, int page_size)
{
    phys_mem->total_size = total_size;
    phys_mem->page_size = page_size;
    phys_mem->memory = (unsigned char *)calloc(total_size, sizeof(unsigned char));
    if (phys_mem->memory == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate physical memory.\n");
        exit(EXIT_FAILURE);
    }

    phys_mem->number_of_frames = total_size / page_size;
    phys_mem->free_frames = (int *)malloc(phys_mem->number_of_frames * sizeof(int));
    if (phys_mem->free_frames == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate free frames list.\n");
        free(phys_mem->memory);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < phys_mem->number_of_frames; i++)
    {
        phys_mem->free_frames[i] = i;
    }
    phys_mem->free_frame_count = phys_mem->number_of_frames;
}

void initialize_process_list(ProcessList *proc_list)
{
    proc_list->capacity = INITIAL_PROCESS_LIST_CAPACITY;
    proc_list->count = 0;
    proc_list->processes = (Process *)malloc(proc_list->capacity * sizeof(Process));
    if (proc_list->processes == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate process list.\n");
        exit(EXIT_FAILURE);
    }
}

int allocate_frames(PhysicalMemory *phys_mem, int required_frames, int *allocated_frames)
{
    if (phys_mem->free_frame_count < required_frames)
    {
        return 0;
    }

    for (int i = 0; i < required_frames; i++)
    {
        allocated_frames[i] = phys_mem->free_frames[phys_mem->free_frame_count - 1];
        phys_mem->free_frame_count--;
    }

    return 1;
}

void create_process(PhysicalMemory *phys_mem, ProcessList *proc_list, int max_process_size)
{
    int pid, size;

    printf("\n=== Create New Process ===\n");

    while (1)
    {
        printf("Enter Process ID (integer): ");
        if (scanf("%d", &pid) != 1)
        {
            printf("Invalid input. Please enter a valid integer.\n");
            clear_input_buffer();
            continue;
        }

        int duplicate = 0;
        for (int i = 0; i < proc_list->count; i++)
        {
            if (proc_list->processes[i].process_id == pid)
            {
                duplicate = 1;
                break;
            }
        }

        if (duplicate)
        {
            printf("Error: Process ID must be unique. Please enter a different ID.\n");
            continue;
        }
        break;
    }

    while (1)
    {
        printf("Enter Process Size in bytes (power of 2, max %d): ", max_process_size);
        if (scanf("%d", &size) != 1)
        {
            printf("Invalid input. Please enter a valid integer.\n");
            clear_input_buffer();
            continue;
        }

        if (!is_power_of_two(size))
        {
            printf("Error: Process size must be a power of 2.\n");
            continue;
        }

        if (size > max_process_size)
        {
            printf("Error: Process size exceeds the maximum allowed size of %d bytes.\n", max_process_size);
            continue;
        }

        break;
    }

    int pages_needed = (int)ceil((double)size / phys_mem->page_size);

    int *allocated_frames = (int *)malloc(pages_needed * sizeof(int));
    if (allocated_frames == NULL)
    {
        printf("Error: Unable to allocate memory for frame allocation.\n");
        return;
    }

    if (!allocate_frames(phys_mem, pages_needed, allocated_frames))
    {
        printf("Error: Insufficient physical memory to allocate the process.\n");
        free(allocated_frames);
        return;
    }

    unsigned char *logical_memory = (unsigned char *)malloc(size * sizeof(unsigned char));
    if (logical_memory == NULL)
    {
        printf("Error: Unable to allocate logical memory for the process.\n");
        free(allocated_frames);
        return;
    }

    for (int i = 0; i < size; i++)
    {
        logical_memory[i] = (unsigned char)(rand() % 256);
    }

    for (int i = 0; i < pages_needed; i++)
    {
        int frame_index = allocated_frames[i];
        int logical_start = i * phys_mem->page_size;
        int physical_start = frame_index * phys_mem->page_size;

        for (int j = 0; j < phys_mem->page_size && (logical_start + j) < size; j++)
        {
            phys_mem->memory[physical_start + j] = logical_memory[logical_start + j];
        }
    }

    free(logical_memory);

    if (proc_list->count >= proc_list->capacity)
    {
        proc_list->capacity *= 2;
        Process *temp = (Process *)realloc(proc_list->processes, proc_list->capacity * sizeof(Process));
        if (temp == NULL)
        {
            printf("Error: Unable to expand the process list.\n");
            free(allocated_frames);
            return;
        }
        proc_list->processes = temp;
    }

    Process new_process;
    new_process.process_id = pid;
    new_process.process_size = size;
    new_process.number_of_pages = pages_needed;
    new_process.page_table = allocated_frames;

    proc_list->processes[proc_list->count++] = new_process;

    printf("Process created successfully!\n");
    printf("Process ID: %d\n", pid);
    printf("Process Size: %d bytes\n", size);
    printf("Number of Pages: %d\n", pages_needed);
}

void view_physical_memory(const PhysicalMemory *phys_mem)
{
    printf("\n=== Physical Memory Status ===\n");
    printf("Total Physical Memory: %d bytes\n", phys_mem->total_size);
    printf("Page Size: %d bytes\n", phys_mem->page_size);
    printf("Total Number of Frames: %d\n", phys_mem->number_of_frames);
    printf("Free Frames: %d (%.2f%%)\n",
           phys_mem->free_frame_count,
           ((double)phys_mem->free_frame_count / phys_mem->number_of_frames) * 100.0);

    printf("\nFrame Status:\n");
    printf("Frame\tStatus\n");
    for (int i = 0; i < phys_mem->number_of_frames; i++)
    {
        int is_free = 0;
        for (int j = 0; j < phys_mem->free_frame_count; j++)
        {
            if (phys_mem->free_frames[j] == i)
            {
                is_free = 1;
                break;
            }
        }
        printf("%d\t%s\n", i, is_free ? "Free" : "Occupied");
    }
}

void view_page_table(const ProcessList *proc_list)
{
    if (proc_list->count == 0)
    {
        printf("\nNo processes available to display.\n");
        return;
    }

    int pid;
    printf("\n=== View Process Page Table ===\n");
    printf("Enter Process ID: ");
    if (scanf("%d", &pid) != 1)
    {
        printf("Invalid input. Please enter a valid integer.\n");
        clear_input_buffer();
        return;
    }

    Process *target_process = NULL;
    for (int i = 0; i < proc_list->count; i++)
    {
        if (proc_list->processes[i].process_id == pid)
        {
            target_process = &proc_list->processes[i];
            break;
        }
    }

    if (target_process == NULL)
    {
        printf("Error: Process with ID %d not found.\n", pid);
        return;
    }

    printf("\nPage Table for Process ID %d:\n", pid);
    printf("Process Size: %d bytes\n", target_process->process_size);
    printf("Number of Pages: %d\n", target_process->number_of_pages);
    printf("Page\tFrame\n");
    for (int i = 0; i < target_process->number_of_pages; i++)
    {
        printf("%d\t%d\n", i, target_process->page_table[i]);
    }
}

void free_memory(PhysicalMemory *phys_mem, ProcessList *proc_list)
{
    free(phys_mem->memory);
    free(phys_mem->free_frames);

    for (int i = 0; i < proc_list->count; i++)
    {
        free(proc_list->processes[i].page_table);
    }

    free(proc_list->processes);
}

void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}
