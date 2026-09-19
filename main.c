#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATIENTS 50

// Constants
const char SPEC_NAMES[4][30] = {"General Practice (OPD)", "Paediatrics", "Cardiology", "Neurology"};
const double SPEC_FEES[4] = {1500.0, 2500.0, 4500.0, 5000.0};
const int SPEC_TIME[4] = {15, 20, 30, 30};
const int SPEC_CAP[4] = {30, 20, 12, 10};

const char WARD_NAMES[4][25] = {"General Ward", "Paediatric Ward", "Surgical Ward", "ICU"};
const double WARD_RATES[4] = {3000.0, 6000.0, 12000.0, 25000.0};
const int WARD_CAP[4] = {20, 10, 10, 5};

// Parallel Arrays for Patient Storage
char p_names[MAX_PATIENTS][50];
int p_ages[MAX_PATIENTS];
int p_urgency[MAX_PATIENTS]; // 1 (low) to 5 (critical)
int p_specID[MAX_PATIENTS];
int p_isAdmitted[MAX_PATIENTS]; // 0 = OPD, 1 = Admitted
int p_wardID[MAX_PATIENTS];
int p_days[MAX_PATIENTS];
int p_bedNum[MAX_PATIENTS];
int p_queuePos[MAX_PATIENTS];

// Calculated billing data
double p_baseFee[MAX_PATIENTS];
double p_surcharge[MAX_PATIENTS];
double p_wardCost[MAX_PATIENTS];
double p_gross[MAX_PATIENTS];
double p_discount[MAX_PATIENTS];
double p_netPayable[MAX_PATIENTS];
double p_waitTime[MAX_PATIENTS];

int patientCount = 0;
int specQueueCount[4] = {0, 0, 0, 0};
int bedOccupancy[4][20];

void initBeds() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 20; j++) {
            bedOccupancy[i][j] = 0;
        }
    }
    // Initial mock occupancy for testing
    bedOccupancy[0][0] = 1;
    bedOccupancy[0][3] = 1;
    bedOccupancy[3][0] = 1;
}

void displayBedStatus() {
    printf("\n--- BED OCCUPANCY STATUS ---\n");
    for (int i = 0; i < 4; i++) {
        printf("%-18s (Cap: %2d): [ ", WARD_NAMES[i], WARD_CAP[i]);
        int free_count = 0;
        for (int j = 0; j < WARD_CAP[i]; j++) {
            printf("%d ", bedOccupancy[i][j]);
            if (bedOccupancy[i][j] == 0) free_count++;
        }
        printf("] -> Free: %d\n", free_count);
    }
}

void registerPatient() {
    if (patientCount >= MAX_PATIENTS) {
        printf("\nPatient limit reached!\n");
        return;
    }

    int idx = patientCount;
    while(getchar() != '\n'); // clear input buffer

    printf("\n--- REGISTER NEW PATIENT ---\n");
    printf("Patient Name: ");
    fgets(p_names[idx], sizeof(p_names[idx]), stdin);
    p_names[idx][strcspn(p_names[idx], "\n")] = 0;

    printf("Age: ");
    scanf("%d", &p_ages[idx]);

    printf("Specialty (0-GP, 1-Paediatrics, 2-Cardiology, 3-Neurology): ");
    scanf("%d", &p_specID[idx]);
    if(p_specID[idx] < 0 || p_specID[idx] > 3) p_specID[idx] = 0;

    printf("Urgency level (1-5, 5 critical): ");
    scanf("%d", &p_urgency[idx]);

    printf("Admit to ward? (0 = No, 1 = Yes): ");
    scanf("%d", &p_isAdmitted[idx]);

    p_baseFee[idx] = SPEC_FEES[p_specID[idx]];
    p_surcharge[idx] = (p_urgency[idx] >= 4) ? p_baseFee[idx] * 0.25 : 0.0;

    if (p_isAdmitted[idx] == 1) {
        printf("Ward (0-General, 1-Paediatric, 2-Surgical, 3-ICU): ");
        scanf("%d", &p_wardID[idx]);
        if(p_wardID[idx] < 0 || p_wardID[idx] > 3) p_wardID[idx] = 0;

        printf("Number of days: ");
        scanf("%d", &p_days[idx]);

        int allocated = -1;
        for (int j = 0; j < WARD_CAP[p_wardID[idx]]; j++) {
            if (bedOccupancy[p_wardID[idx]][j] == 0) {
                bedOccupancy[p_wardID[idx]][j] = 1;
                allocated = j;
                break;
            }
        }
        p_bedNum[idx] = allocated;
        p_wardCost[idx] = WARD_RATES[p_wardID[idx]] * p_days[idx];
        if (allocated == -1) {
            printf("Warning: Ward full! No physical bed allocated.\n");
        }
    } else {
        p_wardID[idx] = -1;
        p_days[idx] = 0;
        p_bedNum[idx] = -1;
        p_wardCost[idx] = 0.0;
    }

    p_gross[idx] = p_baseFee[idx] + p_surcharge[idx] + p_wardCost[idx];
    p_discount[idx] = (p_ages[idx] >= 60) ? p_gross[idx] * 0.10 : 0.0;
    p_netPayable[idx] = p_gross[idx] - p_discount[idx];
    p_waitTime[idx] = SPEC_TIME[p_specID[idx]] * (specQueueCount[p_specID[idx]] + 1);

    specQueueCount[p_specID[idx]]++;
    p_queuePos[idx] = specQueueCount[p_specID[idx]];
    patientCount++;

    printf("\nRegistered! Net Bill: Rs. %.2f | Est Wait: %.0f min\n",
           p_netPayable[idx], p_waitTime[idx]);
}

void displayTriageQueue() {
    if (patientCount == 0) {
        printf("\nNo patients registered yet.\n");
        return;
    }

    int indices[MAX_PATIENTS];
    for(int i = 0; i < patientCount; i++) indices[i] = i;

    // Sort descending by urgency
    for(int i = 0; i < patientCount - 1; i++) {
        for(int j = 0; j < patientCount - i - 1; j++) {
            if(p_urgency[indices[j]] < p_urgency[indices[j+1]]) {
                int tmp = indices[j];
                indices[j] = indices[j+1];
                indices[j+1] = tmp;
            }
        }
    }

    printf("\n--- TRIAGE QUEUE (Sorted by Urgency) ---\n");
    printf("%-5s | %-22s | Age | Urg | Specialty             | Status\n", "Rank", "Name");
    printf("--------------------------------------------------------------------------\n");
    for(int i = 0; i < patientCount; i++) {
        int idx = indices[i];
        printf("%-5d | %-22s | %3d |  %d  | %-21s | %s\n",
               i+1, p_names[idx], p_ages[idx], p_urgency[idx],
               SPEC_NAMES[p_specID[idx]], p_isAdmitted[idx] ? "Admitted" : "OPD");
    }
}

void viewAnalytics() {
    printf("\n--- ANALYTICS & REPORTS ---\n");
    printf("Total Patients: %d\n", patientCount);
    double totalRevenue = 0;
    for(int i = 0; i < patientCount; i++) totalRevenue += p_netPayable[i];
    printf("Total Revenue : Rs. %.2f\n", totalRevenue);

    printf("\nQueue Load per Specialty:\n");
    for(int i = 0; i < 4; i++) {
        printf("- %-24s: %d registrations\n", SPEC_NAMES[i], specQueueCount[i]);
    }
}

void showMenu() {
    printf("\n=========================================\n");
    printf("    SMART HOSPITAL SYSTEM - MAIN MENU\n");
    printf("=========================================\n");
    printf("1. Register New Patient\n");
    printf("2. Display Bed Availability\n");
    printf("3. Display Triage Queue (Sorted)\n");
    printf("4. View Analytics & Reports\n");
    printf("5. Save & Exit\n");
    printf("Enter choice: ");
}

int main() {
    initBeds();
    int choice = 0;

    while (choice != 5) {
        showMenu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Numbers only!\n");
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: registerPatient(); break;
            case 2: displayBedStatus(); break;
            case 3: displayTriageQueue(); break;
            case 4: viewAnalytics(); break;
            case 5: printf("Saving and exiting...\n"); break;
            default: printf("Invalid choice! Pick 1-5.\n");
        }
    }
    return 0;
}
