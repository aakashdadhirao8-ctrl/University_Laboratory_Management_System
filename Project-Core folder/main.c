#include "lab_system.h"

// --- Global Variable Definitions ---
Lab lab_array[MAX_LABS];
int lab_count = 0;

Student students[MAX_STUDENTS];
int student_count = 0;

LabRequest request_queue[QUEUE_SIZE];
int queue_count = 0, queue_front = 0, queue_rear = 0;

pthread_mutex_t data_mutex;
pthread_mutex_t queue_mutex;
pthread_mutex_t log_mutex;
pthread_cond_t queue_cond;

volatile sig_atomic_t server_running = 1;
FILE *log_file;

void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("\n\n[SYSTEM] Ctrl+C detected. Saving data and shutting down gracefully...\n");
        server_running = 0;
        save_database(); 
        pthread_cond_broadcast(&queue_cond); 
    }
}

int main() {
    pthread_mutex_init(&data_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_mutex_init(&log_mutex, NULL);
    pthread_cond_init(&queue_cond, NULL);
    signal(SIGINT, handle_signal);

    log_file = fopen("server.log", "a");
    log_event("=== SYSTEM STARTED ===");

    // Load txt state
    load_database();

    pthread_t workers[NUM_WORKERS];
    for (int i = 0; i < NUM_WORKERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&workers[i], NULL, worker_thread, id);
    }

    int choice;
    while (server_running) {
        // --- NEW: Clear terminal screen every time menu loads! ---
        system("clear"); 

        printf("=========================================\n");
        printf("  UNIVERSITY LABORATORY MANAGEMENT SERVER  \n");
        printf("=========================================\n");
        printf("1. Create a New Lab\n");
        printf("2. Edit an Existing Lab\n");
        printf("3. Register & Assign Student\n");
        printf("4. Query Student Details\n");
        printf("5. Simulate Student Access Request\n");
        printf("6. Monitor Server & Resources\n");
        printf("7. Exit System\n");
        printf("8. Manage Attendance (Manual)\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            continue;
        }
        clear_input_buffer();

        if (!server_running) break;

        switch (choice) {
            case 1: create_lab(); pause_screen(); break;
            case 2: edit_lab(); pause_screen(); break;
            case 3: assign_student(); pause_screen(); break;
            case 4: query_student(); pause_screen(); break;
            case 5: simulate_request(); pause_screen(); break;
            case 6: view_monitor(); pause_screen(); break;
            case 8: manage_attendance(); pause_screen(); break;
            case 7: 
                server_running = 0; 
                save_database();
                pthread_cond_broadcast(&queue_cond);
                break;
            default: 
                printf("Invalid choice.\n"); 
                pause_screen();
        }
    }

    printf("\n[SYSTEM] Waiting for active threads to finish...\n");
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }
    
    for(int i = 0; i < lab_count; i++) {
        for(int e = 0; e < lab_array[i].eq_count; e++) {
            sem_destroy(&lab_array[i].equipments[e].sem);
        }
    }
    pthread_mutex_destroy(&data_mutex);
    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&log_mutex);
    pthread_cond_destroy(&queue_cond);
    if (log_file) fclose(log_file);

    return 0;
}
