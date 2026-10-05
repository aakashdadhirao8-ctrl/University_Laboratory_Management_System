#include "lab_system.h"

// --- UI & Buffer Helpers ---
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pause_screen() {
    printf("\nPress [ENTER] to return to the menu...");
    fflush(stdout);
    getchar(); // Cleanly waits for Enter because buffer is flushed properly now
}

// --- Save and Load Labs/Students state (txt) ---
void save_database() {
    pthread_mutex_lock(&data_mutex);
    FILE *f_labs = fopen("labs.txt", "w");
    if (f_labs) {
        fprintf(f_labs, "%d\n", lab_count);
        for (int i = 0; i < lab_count; i++) {
            fprintf(f_labs, "%d\n%s\n%d\n%d\n", lab_array[i].id, lab_array[i].name, lab_array[i].free_hours, lab_array[i].slots);
            fprintf(f_labs, "%d\n", lab_array[i].primary_count);
            for (int j = 0; j < lab_array[i].primary_count; j++) fprintf(f_labs, "%s\n", lab_array[i].primary_faculty[j]);
            fprintf(f_labs, "%d\n", lab_array[i].sub_count);
            for (int j = 0; j < lab_array[i].sub_count; j++) fprintf(f_labs, "%s\n", lab_array[i].sub_faculty[j]);
            fprintf(f_labs, "%d\n", lab_array[i].eq_count);
            for (int j = 0; j < lab_array[i].eq_count; j++) {
                fprintf(f_labs, "%s\n%s\n%d\n", lab_array[i].equipments[j].name, lab_array[i].equipments[j].specs, lab_array[i].equipments[j].units);
            }
        }
        fclose(f_labs);
    }
    FILE *f_stu = fopen("students.txt", "w");
    if (f_stu) {
        fprintf(f_stu, "%d\n", student_count);
        for (int i = 0; i < student_count; i++) {
            fprintf(f_stu, "%d\n%s\n%d\n", students[i].id, students[i].name, students[i].assigned_lab_id);
        }
        fclose(f_stu);
    }
    pthread_mutex_unlock(&data_mutex);
}

void load_database() {
    FILE *f_labs = fopen("labs.txt", "r");
    if (f_labs) {
        if (fscanf(f_labs, "%d\n", &lab_count) == 1) {
            for (int i = 0; i < lab_count; i++) {
                fscanf(f_labs, "%d\n", &lab_array[i].id);
                fgets(lab_array[i].name, 50, f_labs); lab_array[i].name[strcspn(lab_array[i].name, "\n")] = 0; 
                fscanf(f_labs, "%d\n%d\n", &lab_array[i].free_hours, &lab_array[i].slots);
                
                fscanf(f_labs, "%d\n", &lab_array[i].primary_count);
                for (int j = 0; j < lab_array[i].primary_count; j++) {
                    fgets(lab_array[i].primary_faculty[j], 50, f_labs); lab_array[i].primary_faculty[j][strcspn(lab_array[i].primary_faculty[j], "\n")] = 0;
                }
                fscanf(f_labs, "%d\n", &lab_array[i].sub_count);
                for (int j = 0; j < lab_array[i].sub_count; j++) {
                    fgets(lab_array[i].sub_faculty[j], 50, f_labs); lab_array[i].sub_faculty[j][strcspn(lab_array[i].sub_faculty[j], "\n")] = 0;
                }
                fscanf(f_labs, "%d\n", &lab_array[i].eq_count);
                for (int j = 0; j < lab_array[i].eq_count; j++) {
                    fgets(lab_array[i].equipments[j].name, 50, f_labs); lab_array[i].equipments[j].name[strcspn(lab_array[i].equipments[j].name, "\n")] = 0;
                    fgets(lab_array[i].equipments[j].specs, 100, f_labs); lab_array[i].equipments[j].specs[strcspn(lab_array[i].equipments[j].specs, "\n")] = 0;
                    fscanf(f_labs, "%d\n", &lab_array[i].equipments[j].units);
                    sem_init(&lab_array[i].equipments[j].sem, 0, lab_array[i].equipments[j].units);
                }
            }
        }
        fclose(f_labs);
    }
    
    FILE *f_stu = fopen("students.txt", "r");
    if (f_stu) {
        if (fscanf(f_stu, "%d\n", &student_count) == 1) {
            for (int i = 0; i < student_count; i++) {
                fscanf(f_stu, "%d\n", &students[i].id);
                fgets(students[i].name, 50, f_stu); students[i].name[strcspn(students[i].name, "\n")] = 0;
                fscanf(f_stu, "%d\n", &students[i].assigned_lab_id);
            }
        }
        fclose(f_stu);
    }
}

void create_lab() {
    system("clear");
    pthread_mutex_lock(&data_mutex);
    if (lab_count >= MAX_LABS) {
        printf("Maximum labs reached!\n");
        pthread_mutex_unlock(&data_mutex);
        return;
    }

    Lab *l = &lab_array[lab_count];
    l->id = lab_count;
    l->eq_count = 0;
    l->primary_count = 0;
    l->sub_count = 0;
    
    printf("=== CREATE A NEW LAB ===\n");
    printf("Enter Lab Name: ");
    scanf(" %[^\n]", l->name); clear_input_buffer();
    printf("Enter Number of Free Hours: ");
    scanf("%d", &l->free_hours); clear_input_buffer();
    printf("Enter Total Slots: ");
    scanf("%d", &l->slots); clear_input_buffer();

    int choice;
    do {
        printf("\nEnter Primary Faculty Name (#%d): ", l->primary_count + 1);
        scanf(" %[^\n]", l->primary_faculty[l->primary_count]); clear_input_buffer();
        l->primary_count++;
        if (l->primary_count < MAX_FACULTY) {
            printf("1. Add another Primary Faculty\n2. Continue to Substitutes\nEnter choice: ");
            scanf("%d", &choice); clear_input_buffer();
        } else choice = 2;
    } while (choice == 1);

    do {
        printf("\nEnter Substitute Faculty Name (#%d): ", l->sub_count + 1);
        scanf(" %[^\n]", l->sub_faculty[l->sub_count]); clear_input_buffer();
        l->sub_count++;
        if (l->sub_count < MAX_SUBS) {
            printf("1. Add another Substitute Faculty\n2. Continue to Equipment\nEnter choice: ");
            scanf("%d", &choice); clear_input_buffer();
        } else choice = 2;
    } while (choice == 1);

    do {
        if (l->eq_count >= MAX_EQUIPMENT) break;
        Equipment *eq = &l->equipments[l->eq_count];
        printf("\n--- Equipment Setup (#%d) ---\n", l->eq_count + 1);
        printf("Enter Equipment Name: ");
        scanf(" %[^\n]", eq->name); clear_input_buffer();
        
        printf("Enter Equipment Specs (e.g., 'i9, 32GB RAM'): ");
        scanf(" %[^\n]", eq->specs); clear_input_buffer();
        
        printf("Enter Units (Availability): ");
        scanf("%d", &eq->units); clear_input_buffer();

        sem_init(&eq->sem, 0, eq->units);
        l->eq_count++;

        printf("1. Add another equipment type\n2. Save Lab\nEnter your choice: ");
        scanf("%d", &choice); clear_input_buffer();
    } while (choice == 1 && l->eq_count < MAX_EQUIPMENT);

    lab_count++;
    pthread_mutex_unlock(&data_mutex);
    save_database(); 
    printf("\n--> Lab created and saved successfully!\n");
}

void edit_lab() {
    system("clear");
    pthread_mutex_lock(&data_mutex);
    printf("=== EDIT EXISTING LAB ===\n");
    for (int i = 0; i < lab_count; i++) printf("ID: %d - Name: %s\n", lab_array[i].id, lab_array[i].name);
    
    int id;
    printf("Enter Lab ID to edit: ");
    scanf("%d", &id); clear_input_buffer();

    if (id >= 0 && id < lab_count) {
        printf("New Free Hours (current: %d): ", lab_array[id].free_hours);
        scanf("%d", &lab_array[id].free_hours); clear_input_buffer();
        printf("\n--> Lab updated successfully!\n");
    } else {
        printf("Invalid ID.\n");
    }
    pthread_mutex_unlock(&data_mutex);
    save_database();
}

// --- FIXED: STUDENT REGISTRATION CONTINUOUS LOOP ---
void assign_student() {
    int choice;
    do {
        system("clear");
        pthread_mutex_lock(&data_mutex);
        printf("=== REGISTER & ASSIGN STUDENT ===\n\n");
        if (lab_count == 0) {
            printf("Error: No labs exist yet. Please create a lab first.\n");
            pthread_mutex_unlock(&data_mutex);
            return;
        }
        if (student_count >= MAX_STUDENTS) {
            printf("Maximum students limit (%d) reached!\n", MAX_STUDENTS);
            pthread_mutex_unlock(&data_mutex);
            return;
        }

        Student *s = &students[student_count];
        s->id = student_count;
        
        printf("Enter Student Name: ");
        scanf(" %[^\n]", s->name); clear_input_buffer();
        
        printf("\nAvailable Labs:\n");
        for (int i = 0; i < lab_count; i++) {
            printf("  [%d] - %s\n", lab_array[i].id, lab_array[i].name);
        }
        
        printf("\nEnter Lab ID to assign: ");
        scanf("%d", &s->assigned_lab_id); clear_input_buffer();

        if (s->assigned_lab_id >= 0 && s->assigned_lab_id < lab_count) {
            student_count++;
            printf("\n--> Success: Student '%s' (ID: %d) registered in Lab '%s'!\n", 
                   s->name, s->id, lab_array[s->assigned_lab_id].name);
        } else {
            printf("\nInvalid Lab ID. Registration failed for this entry.\n");
        }
        pthread_mutex_unlock(&data_mutex);
        save_database();

        printf("\nOptions:\n");
        printf("1. Continue (Register another student)\n");
        printf("2. Save & Close (Return to Menu)\n");
        printf("Enter choice: ");
        scanf("%d", &choice); clear_input_buffer();

    } while (choice == 1);
}

// --- FIXED: LAB-WISE STUDENT DIRECTORY ---
void query_student() {
    system("clear");
    printf("=========================================\n");
    printf("           STUDENT DIRECTORY             \n");
    printf("=========================================\n");
    printf("1. View Students Lab-Wise (Select Lab)\n");
    printf("2. Search Student by Name\n");
    printf("3. View All Registered Students\n");
    printf("Enter choice: ");
    
    int q_choice;
    if (scanf("%d", &q_choice) != 1) {
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    pthread_mutex_lock(&data_mutex);

    // Option 1: Show Students Lab-wise
    if (q_choice == 1) {
        if (lab_count == 0) {
            printf("\nNo labs currently exist.\n");
        } else {
            printf("\n--- Available Labs ---\n");
            for (int i = 0; i < lab_count; i++) {
                printf("  [%d] - %s (Slots: %d, Free Hours: %d)\n", 
                       lab_array[i].id, lab_array[i].name, lab_array[i].slots, lab_array[i].free_hours);
            }
            
            int l_id;
            printf("\nEnter Lab ID to view enrolled students: ");
            if (scanf("%d", &l_id) == 1) {
                clear_input_buffer();
                if (l_id >= 0 && l_id < lab_count) {
                    printf("\n=======================================================\n");
                    printf(" LAB: %s (ID: %d)\n", lab_array[l_id].name, l_id);
                    printf("=======================================================\n");
                    printf("Supervising Faculty:\n");
                    for (int f = 0; f < lab_array[l_id].primary_count; f++) {
                        printf("  - Primary: %s\n", lab_array[l_id].primary_faculty[f]);
                    }
                    for (int sub = 0; sub < lab_array[l_id].sub_count; sub++) {
                        printf("  - Substitute: %s\n", lab_array[l_id].sub_faculty[sub]);
                    }
                    printf("\nAvailable Equipment:\n");
                    for (int e = 0; e < lab_array[l_id].eq_count; e++) {
                        printf("  - %s [%s] (Units: %d)\n", 
                               lab_array[l_id].equipments[e].name, 
                               lab_array[l_id].equipments[e].specs, 
                               lab_array[l_id].equipments[e].units);
                    }
                    printf("\nRegistered Students in this Lab:\n");
                    printf("%-8s | %-25s\n", "ID", "Student Name");
                    printf("-------------------------------------------------------\n");
                    int count = 0;
                    for (int i = 0; i < student_count; i++) {
                        if (students[i].assigned_lab_id == l_id) {
                            printf("%-8d | %-25s\n", students[i].id, students[i].name);
                            count++;
                        }
                    }
                    if (count == 0) {
                        printf("No students currently assigned to this lab.\n");
                    } else {
                        printf("-------------------------------------------------------\n");
                        printf("Total Students in Lab: %d\n", count);
                    }
                    printf("=======================================================\n");
                } else {
                    printf("Invalid Lab ID.\n");
                }
            } else {
                clear_input_buffer();
            }
        }
    } 
    // Option 2: Search by Name
    else if (q_choice == 2) {
        char search_name[50];
        printf("\nEnter Student Name to search: ");
        scanf(" %[^\n]", search_name); clear_input_buffer();

        int found = 0;
        for (int i = 0; i < student_count; i++) {
            if (strcasecmp(students[i].name, search_name) == 0) {
                int lab_id = students[i].assigned_lab_id;
                printf("\n--- Student Found ---\n");
                printf("Student ID : %d\n", students[i].id);
                printf("Name       : %s\n", students[i].name);
                printf("Lab        : %s\n", lab_array[lab_id].name);
                printf("Faculty    : %s (Primary)\n", 
                       lab_array[lab_id].primary_count > 0 ? lab_array[lab_id].primary_faculty[0] : "None");
                printf("----------------------\n");
                found = 1; 
                break;
            }
        }
        if (!found) printf("\nStudent '%s' not found.\n", search_name);
    }
    // Option 3: View All Students
    else if (q_choice == 3) {
        printf("\n=======================================================\n");
        printf("%-8s | %-20s | %-20s\n", "ID", "Student Name", "Assigned Lab");
        printf("-------------------------------------------------------\n");
        if (student_count == 0) {
            printf("No registered students found in the database.\n");
        } else {
            for (int i = 0; i < student_count; i++) {
                int lab_id = students[i].assigned_lab_id;
                printf("%-8d | %-20s | %-20s\n", students[i].id, students[i].name, lab_array[lab_id].name);
            }
        }
        printf("=======================================================\n");
    } else {
        printf("\nInvalid choice.\n");
    }

    pthread_mutex_unlock(&data_mutex);
}

void simulate_request() {
    system("clear");
    int s_id, eq_idx, duration;
    printf("=== SIMULATE ACCESS REQUEST ===\n");
    printf("Enter Student ID: ");
    scanf("%d", &s_id); clear_input_buffer();

    pthread_mutex_lock(&data_mutex);
    if (s_id < 0 || s_id >= student_count) {
        printf("Invalid Student ID.\n");
        pthread_mutex_unlock(&data_mutex);
        return;
    }
    int lab_id = students[s_id].assigned_lab_id;

    printf("\nAvailable Equipment in %s:\n", lab_array[lab_id].name);
    for (int e = 0; e < lab_array[lab_id].eq_count; e++) {
        printf("  [%d] - %s (%s) | Capacity: %d\n", e, lab_array[lab_id].equipments[e].name, lab_array[lab_id].equipments[e].specs, lab_array[lab_id].equipments[e].units);
    }
    printf("Enter Equipment Number: ");
    scanf("%d", &eq_idx); clear_input_buffer();
    printf("Enter usage duration (in seconds): ");
    scanf("%d", &duration); clear_input_buffer();
    pthread_mutex_unlock(&data_mutex);

    pthread_mutex_lock(&queue_mutex);
    if (queue_count < QUEUE_SIZE) {
        request_queue[queue_rear].student_id = s_id;
        request_queue[queue_rear].eq_index = eq_idx;
        request_queue[queue_rear].duration = duration;
        queue_rear = (queue_rear + 1) % QUEUE_SIZE;
        queue_count++;
        printf("\n--> Request Queued! Workers will allocate it in the background.\n");
        pthread_cond_signal(&queue_cond); 
    }
    pthread_mutex_unlock(&queue_mutex);
}

void manage_attendance() {
    system("clear");
    printf("=== ATTENDANCE MANAGEMENT ===\n");
    printf("1. Mark Manual Attendance (Present/Absent)\n");
    printf("2. View Attendance Logs (CSV)\n");
    printf("Choice: ");
    int choice;
    scanf("%d", &choice); clear_input_buffer();

    if (choice == 1) {
        int s_id;
        char status_input;
        printf("\nEnter Student ID: ");
        scanf("%d", &s_id); clear_input_buffer();

        pthread_mutex_lock(&data_mutex);
        if (s_id >= 0 && s_id < student_count) {
            char s_name[50], lab_name[50];
            strcpy(s_name, students[s_id].name);
            strcpy(lab_name, lab_array[students[s_id].assigned_lab_id].name);
            pthread_mutex_unlock(&data_mutex);

            printf("Mark %s as Present (P) or Absent (A)? [P/A]: ", s_name);
            scanf(" %c", &status_input); clear_input_buffer();

            char status_str[20];
            if (status_input == 'P' || status_input == 'p') strcpy(status_str, "Present");
            else if (status_input == 'A' || status_input == 'a') strcpy(status_str, "Absent");
            else strcpy(status_str, "Unknown");

            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char time_str[50];
            strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info); 

            pthread_mutex_lock(&log_mutex);
            FILE *fp = fopen("attendance.csv", "a");
            if(fp) {
                fseek(fp, 0, SEEK_END);
                if (ftell(fp) == 0) fprintf(fp, "Timestamp,StudentID,StudentName,LabName,Status\n");
                fprintf(fp, "%s,%d,%s,%s,%s\n", time_str, s_id, s_name, lab_name, status_str);
                fclose(fp);
            }
            pthread_mutex_unlock(&log_mutex);
            printf("\n--> Successfully marked %s as %s in attendance.csv!\n", s_name, status_str);
        } else {
            pthread_mutex_unlock(&data_mutex);
            printf("Invalid Student ID.\n");
        }
    } 
    else if (choice == 2) {
        printf("\n--- Recent Attendance Logs ---\n");
        printf("%-20s | %-15s | %-15s | %-10s\n", "Timestamp", "Name", "Lab", "Status");
        printf("----------------------------------------------------------------------\n");

        FILE *fp = fopen("attendance.csv", "r");
        if(fp) {
            char line[256];
            int count = 0;
            while(fgets(line, sizeof(line), fp)) {
                char timestamp[50], name[50], lab[50], status[50];
                int id;
                if (sscanf(line, "%[^,],%d,%[^,],%[^,],%s", timestamp, &id, name, lab, status) == 5) {
                    printf("%-20s | %-15s | %-15s | %-10s\n", timestamp, name, lab, status);
                    count++;
                }
            }
            fclose(fp);
            if (count == 0) printf("No logs found.\n");
        } else {
            printf("No attendance data exists yet.\n");
        }
    }
}

void view_monitor() {
    system("clear");
    pthread_mutex_lock(&data_mutex);
    printf("==== LIVE OS SERVER MONITOR ====\n");
    for (int i = 0; i < lab_count; i++) {
        printf("\nLab '%s':\n", lab_array[i].name);
        for (int e = 0; e < lab_array[i].eq_count; e++) {
            int avail;
            sem_getvalue(&lab_array[i].equipments[e].sem, &avail);
            printf("  [%s] (%s) -> %d / %d units available\n", lab_array[i].equipments[e].name, lab_array[i].equipments[e].specs, avail, lab_array[i].equipments[e].units);
        }
    }
    printf("================================\n");
    pthread_mutex_unlock(&data_mutex);
}
