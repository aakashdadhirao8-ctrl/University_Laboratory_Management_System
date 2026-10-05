#include "lab_system.h"

void log_event(const char *msg) {
    pthread_mutex_lock(&log_mutex);
    if (log_file) {
        fprintf(log_file, "%s\n", msg);
        fflush(log_file);
    }
    pthread_mutex_unlock(&log_mutex);
}

void log_csv_allocation(int s_id, const char* s_name, const char* lab_name, const char* eq_name, int duration, const char* time_str) {
    pthread_mutex_lock(&log_mutex);
    FILE *fp = fopen("allocation.csv", "a");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        if (ftell(fp) == 0) fprintf(fp, "Timestamp,StudentID,StudentName,LabName,EquipmentName,DurationSec\n");
        fprintf(fp, "%s,%d,%s,%s,%s,%d\n", time_str, s_id, s_name, lab_name, eq_name, duration);
        fclose(fp);
    }
    pthread_mutex_unlock(&log_mutex);
}

void* worker_thread(void* arg) {
    int thread_id = *(int*)arg;
    free(arg);

    while (server_running) {
        pthread_mutex_lock(&queue_mutex);
        while (queue_count == 0 && server_running) {
            pthread_cond_wait(&queue_cond, &queue_mutex); 
        }
        
        if (!server_running) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }

        LabRequest req = request_queue[queue_front];
        queue_front = (queue_front + 1) % QUEUE_SIZE;
        queue_count--;
        pthread_mutex_unlock(&queue_mutex);

        pthread_mutex_lock(&data_mutex);
        int s_id = req.student_id;
        int eq_idx = req.eq_index;
        int duration = req.duration;

        int lab_id = students[s_id].assigned_lab_id;
        char student_name[50], lab_name[50], eq_name[50], eq_specs[100];
        strcpy(student_name, students[s_id].name);
        strcpy(lab_name, lab_array[lab_id].name);
        strcpy(eq_name, lab_array[lab_id].equipments[eq_idx].name);
        strcpy(eq_specs, lab_array[lab_id].equipments[eq_idx].specs);
        pthread_mutex_unlock(&data_mutex);

        // Request OS Semaphore
        sem_wait(&lab_array[lab_id].equipments[eq_idx].sem);
        
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char time_str[50];
        strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info); 

        // Write Allocation CSV
        log_csv_allocation(s_id, student_name, lab_name, eq_name, duration, time_str);

        // Expanded buffer to 512 bytes with snprintf to guarantee zero overflow
        char log_msg[512];
        snprintf(log_msg, sizeof(log_msg), "[THREAD %d] %s allocated '%s' [%s] in %s (for %d sec).", 
                 thread_id, student_name, eq_name, eq_specs, lab_name, duration);
        log_event(log_msg);

        // Simulate Work
        sleep(duration); 

        // Release OS Semaphore
        sem_post(&lab_array[lab_id].equipments[eq_idx].sem);
        snprintf(log_msg, sizeof(log_msg), "[THREAD %d] FINISHED: %s released '%s'.", 
                 thread_id, student_name, eq_name);
        log_event(log_msg);
    }
    return NULL;
}
