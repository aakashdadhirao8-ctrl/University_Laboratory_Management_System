#ifndef LAB_SYSTEM_H
#define LAB_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h> 

#define MAX_LABS 10
#define MAX_STUDENTS 50
#define MAX_EQUIPMENT 5
#define MAX_FACULTY 3
#define MAX_SUBS 2
#define QUEUE_SIZE 100
#define NUM_WORKERS 4

// --- Data Structures ---
typedef struct {
    char name[50];       
    char specs[100];     // NEW: Equipment Specifications
    int units;           
    sem_t sem;           
} Equipment;

typedef struct {
    int id;
    char name[50];
    
    int primary_count;
    char primary_faculty[MAX_FACULTY][50];
    int sub_count;
    char sub_faculty[MAX_SUBS][50];
    
    int free_hours;
    int slots;
    Equipment equipments[MAX_EQUIPMENT]; 
    int eq_count;                        
} Lab;

typedef struct {
    int id;
    char name[50];
    int assigned_lab_id; 
} Student;

typedef struct {
    int student_id;
    int eq_index;
    int duration;
} LabRequest;

// --- Global Variables ---
extern Lab lab_array[MAX_LABS];
extern int lab_count;

extern Student students[MAX_STUDENTS];
extern int student_count;

extern LabRequest request_queue[QUEUE_SIZE];
extern int queue_count, queue_front, queue_rear;

extern pthread_mutex_t data_mutex;
extern pthread_mutex_t queue_mutex;
extern pthread_mutex_t log_mutex;
extern pthread_cond_t queue_cond;

extern volatile sig_atomic_t server_running;
extern FILE *log_file;

// --- Function Prototypes ---
void log_event(const char *msg);
void clear_input_buffer();
void pause_screen(); // NEW: UI Pause
void* worker_thread(void* arg);

void create_lab();
void edit_lab();
void assign_student();
void query_student();
void simulate_request();
void view_monitor();

void save_database();
void load_database();
void manage_attendance(); // NEW: Manual Attendance

void log_csv_allocation(int s_id, const char* s_name, const char* lab_name, const char* eq_name, int duration, const char* time_str);

#endif
