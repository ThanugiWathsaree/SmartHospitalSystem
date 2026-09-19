#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATIENTS 50

// Constants for Specialties (0 to 3 internally)
const char SPEC_NAMES[4][30] = {"General Practice (OPD)", "Paediatrics", "Cardiology", "Neurology"};
const double SPEC_FEES[4] = {1500.0, 2500.0, 4500.0, 5000.0};
const int SPEC_TIME[4] = {15, 20, 30, 30};
const int SPEC_CAP[4] = {30, 20, 12, 10};

// Constants for Wards (0 to 3 internally)
const char WARD_NAMES[4][25] = {"General Ward", "Paediatric Ward", "Surgical Ward", "ICU"};
const double WARD_RATES[4] = {3000.0, 6000.0, 12000.0, 25000.0};
const int WARD_CAP[4] = {20, 10, 10, 5};

// Parallel Arrays for Patient Storage
char p_names[MAX_PATIENTS][50];
int p_ages[MAX_PATIENTS];
int p_urgency[MAX_PATIENTS]; // 1 (low) to 5 (critical)
int p_specID[MAX_PATIENTS];  // 0 to 3 internal index
int p_isAdmitted[MAX_PATIENTS]; // 0 = OPD, 1 = Admitted
int p_wardID[MAX_PATIENTS];  // 0 to 3 internal index, or -1
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

// System tracking variables
int patientCount = 0;
int specQueueCount[4] = {0, 0, 0, 0};
int bedOccupancy[4][20];

// Initialize bed occupancy matrix (default mock)
void initBeds() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 20; j++) {
            bedOccupancy[i][j] = 0;
        }
    }
    // Initial mock occupancy for testing fallback
    bedOccupancy[0][0] = 1;
    bedOccupancy[0][3] = 1;
    bedOccupancy[3][0] = 1;
}

// Save bed occupancy matrix to file
void saveBedStatus() {
    FILE *fp = fopen("beds_status.txt", "w");
    if (fp == NULL) {
        printf("Error: Could not save bed status to file!\n");
        return;
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < WARD_CAP[i]; j++) {
            fprintf(fp, "%d ", bedOccupancy[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

// Load bed occupancy matrix from file (fallbacks to initBeds if file not found)
void loadBedStatus() {
    FILE *fp = fopen("beds_status.txt", "r");
    if (fp == NULL) {
        initBeds();
        return;
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < WARD_CAP[i]; j++) {
            if (fscanf(fp, "%d", &bedOccupancy[i][j]) != 1) {
                bedOccupancy[i][j] = 0;
            }
        }
    }
    fclose(fp);
}

// Save all patient records cleanly using "w" to sync current dataset without duplicate rows
void savePatientRecords() {
    FILE *fp = fopen("patient_records.txt", "w");
    if (fp == NULL) {
        printf("Error: Could not save patient records to file!\n");
        return;
    }
    for (int i = 0; i < patientCount; i++) {
        fprintf(fp, "PAT-%d | %s | Net: LKR %.2f\n", 1001 + i, p_names[i], p_netPayable[i]);
    }
    fclose(fp);
}

// Display bed status across all wards
void displayBedStatus() {
    printf("\n--- CURRENT BED OCCUPANCY MATRIX ---\n");
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

// Allocate a physical bed (returns 1-based bed number, or -1 if full)
int allocateBed(int wardIdx) {
    if (wardIdx < 0 || wardIdx >= 4) return -1;
    for (int j = 0; j < WARD_CAP[wardIdx]; j++) {
        if (bedOccupancy[wardIdx][j] == 0) {
            bedOccupancy[wardIdx][j] = 1; // Mark occupied
            return j + 1; // 1-based Bed Number
        }
    }
    return -1; // Ward full
}

// Calculate wait time for a specialty and increment queue count
double calcWaitTime(int specIdx) {
    if (specIdx < 0 || specIdx >= 4) specIdx = 0;
    double wait = specQueueCount[specIdx] * SPEC_TIME[specIdx];
    specQueueCount[specIdx]++;
    return wait;
}

// Calculate surcharge based on urgency level (1-5)
double calcSurcharge(double baseFee, int urgency) {
    if (urgency == 2) return baseFee * 0.20;
    if (urgency == 3) return baseFee * 0.50;
    if (urgency >= 4) return baseFee * 0.25;
    return 0.0;
}

// Calculate ward cost based on admission status, internal ward index (0-3), and days
double calcWardCost(int isAdmitted, int wardIdx, int days) {
    if (isAdmitted == 1 && wardIdx >= 0 && wardIdx < 4) {
        return days * WARD_RATES[wardIdx];
    }
    return 0.0;
}

// Compute comprehensive bill details for a given patient index
void computeBillDetails(int idx) {
    int sIdx = p_specID[idx];
    if (sIdx < 0 || sIdx >= 4) sIdx = 0;

    p_baseFee[idx] = SPEC_FEES[sIdx];
    p_surcharge[idx] = calcSurcharge(p_baseFee[idx], p_urgency[idx]);
    p_wardCost[idx] = calcWardCost(p_isAdmitted[idx], p_wardID[idx], p_days[idx]);

    p_gross[idx] = p_baseFee[idx] + p_surcharge[idx] + p_wardCost[idx];

    // Age-based discount rule: < 5 or > 65 gets 15% discount
    if (p_ages[idx] < 5 || p_ages[idx] > 65) {
        p_discount[idx] = p_gross[idx] * 0.15;
    } else {
        p_discount[idx] = 0.0;
    }

    p_netPayable[idx] = p_gross[idx] - p_discount[idx];
}

// Print formatted patient admission and financial bill
void printBill(int idx) {
    if (idx < 0 || idx >= patientCount) return;
    int sIdx = p_specID[idx];
    if (sIdx < 0 || sIdx >= 4) sIdx = 0;

    printf("\n======================================================\n");
    printf("         SMART HOSPITAL ADMISSION & BILL             \n");
    printf("======================================================\n");
    printf("Patient ID           : PAT-%d\n", 1001 + idx);
    printf("Patient Name         : %s\n", p_names[idx]);
    printf("Specialty            : %s\n", SPEC_NAMES[sIdx]);
    printf("Base Consultation Fee: LKR %10.2f\n", p_baseFee[idx]);
    printf("Emergency Surcharge  : LKR %10.2f\n", p_surcharge[idx]);
    printf("Ward Stay Cost       : LKR %10.2f\n", p_wardCost[idx]);
    printf("Gross Total Bill     : LKR %10.2f\n", p_gross[idx]);
    printf("Age Subsidy Discount : LKR -%9.2f (15%%)\n", p_discount[idx]);
    printf("------------------------------------------------------\n");
    printf("Final Payable Amount : LKR %10.2f\n", p_netPayable[idx]);
    printf("Estimated Wait Time  : %.0f mins (Pos: %d)\n", p_waitTime[idx], p_queuePos[idx]);
    printf("======================================================\n");
}

// Register a new patient with 1-4 user-friendly input mapping, bed allocation, and modular calculations
void registerPatient() {
    if (patientCount >= MAX_PATIENTS) {
        printf("\nPatient limit reached!\n");
        return;
    }

    int idx = patientCount;
    int specInput, wardInput;

    // Clear input buffer safely if leftover newline exists
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    printf("\n--- REGISTER NEW PATIENT ---\n");
    printf("Patient Name: ");
    fgets(p_names[idx], sizeof(p_names[idx]), stdin);
    p_names[idx][strcspn(p_names[idx], "\n")] = 0;

    printf("Age: ");
    scanf("%d", &p_ages[idx]);

    // User-friendly 1-4 specialty choice
    printf("Specialty (1-General Practice, 2-Paediatrics, 3-Cardiology, 4-Neurology): ");
    scanf("%d", &specInput);
    if (specInput < 1 || specInput > 4) {
        printf("Invalid choice, defaulting to General Practice (1).\n");
        specInput = 1;
    }
    p_specID[idx] = specInput - 1; // Map to 0-3 internal index

    printf("Urgency level (1-5): ");
    scanf("%d", &p_urgency[idx]);
    if (p_urgency[idx] < 1) p_urgency[idx] = 1;
    if (p_urgency[idx] > 5) p_urgency[idx] = 5;

    printf("Admit to ward? (0 = No, 1 = Yes): ");
    scanf("%d", &p_isAdmitted[idx]);

    if (p_isAdmitted[idx] == 1) {
        // User-friendly 1-4 ward choice
        printf("Ward (1-General, 2-Paediatric, 3-Surgical, 4-ICU): ");
        scanf("%d", &wardInput);
        if (wardInput < 1 || wardInput > 4) {
            wardInput = 1;
        }
        p_wardID[idx] = wardInput - 1; // Map to 0-3 internal index

        printf("Number of days: ");
        scanf("%d", &p_days[idx]);
        if (p_days[idx] < 1) p_days[idx] = 1;

        // Allocate physical bed
        p_bedNum[idx] = allocateBed(p_wardID[idx]);

        if (p_bedNum[idx] == -1) {
            printf("Warning: Ward full! No physical bed allocated. Switching to OPD mode.\n");
            p_isAdmitted[idx] = 0;
            p_wardID[idx] = -1;
            p_days[idx] = 0;
            p_bedNum[idx] = -1;
        } else {
            printf("Assigned Bed #: %d\n", p_bedNum[idx]);
        }
    } else {
        p_wardID[idx] = -1;
        p_days[idx] = 0;
        p_bedNum[idx] = -1;
    }

    // Compute bill details via modular function
    computeBillDetails(idx);

    // Queue & Wait time computation
    p_waitTime[idx] = calcWaitTime(p_specID[idx]);
    p_queuePos[idx] = specQueueCount[p_specID[idx]];

    printBill(idx);
    patientCount++;
}

// Display emergency triage queue sorted by urgency (descending)
void displaySortedTriage() {
    if (patientCount == 0) {
        printf("\nNo patients registered yet.\n");
        return;
    }

    int order[MAX_PATIENTS];
    int i, j, temp;
    for (i = 0; i < patientCount; i++) order[i] = i;

    // Sort descending by urgency
    for (i = 0; i < patientCount - 1; i++) {
        for (j = 0; j < patientCount - i - 1; j++) {
            if (p_urgency[order[j]] < p_urgency[order[j + 1]]) {
                temp = order[j];
                order[j] = order[j + 1];
                order[j + 1] = temp;
            }
        }
    }

    printf("\n--- EMERGENCY TRIAGE QUEUE (SORTED) ---\n");
    for (i = 0; i < patientCount; i++) {
        int p = order[i];
        printf("PAT-%d | %-20s | Level %d | Wait: %.0f min | Status: %s\n",
               1001 + p, p_names[p], p_urgency[p], p_waitTime[p],
               p_isAdmitted[p] ? "Admitted" : "OPD");
    }
}

// Unified analytics & urgency report function
void viewAnalytics() {
    int urgencyCounts[6] = {0}; // index 1 to 5
    double totalRev = 0.0;

    for (int i = 0; i < patientCount; i++) {
        if (p_urgency[i] >= 1 && p_urgency[i] <= 5) {
            urgencyCounts[p_urgency[i]]++;
        }
        totalRev += p_netPayable[i];
    }

    printf("\n--- PERFORMANCE & ANALYTICS REPORT ---\n");
    printf("Total Patients : %d\n", patientCount);
    printf("Total Revenue  : LKR %.2f\n", totalRev);

    printf("\nUrgency Level Distribution:\n");
    for (int u = 1; u <= 5; u++) {
        printf("- Level %d      : %d patient(s)\n", u, urgencyCounts[u]);
    }

    printf("\nQueue Load per Specialty:\n");
    for (int i = 0; i < 4; i++) {
        printf("- %-24s: %d registrations\n", SPEC_NAMES[i], specQueueCount[i]);
    }
}

// Display main menu UI
void showMenu() {
    printf("\n======================================================\n");
    printf("    SMART HOSPITAL RESOURCE ALLOCATION SYSTEM\n");
    printf("======================================================\n");
    printf("1. Register New Patient & Generate Bill\n");
    printf("2. Display Bed Occupancy Matrix\n");
    printf("3. Display Emergency Triage Queue (Sorted)\n");
    printf("4. View Analytics & Performance Report\n");
    printf("5. Save & Exit\n");
    printf("Enter choice (1-5): ");
}

int main() {
    loadBedStatus(); // Load existing bed status from file or initialize defaults
    int choice = 0;

    while (choice != 5) {
        showMenu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Numbers only!\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        switch (choice) {
            case 1:
                registerPatient();
                break;
            case 2:
                displayBedStatus();
                break;
            case 3:
                displaySortedTriage();
                break;
            case 4:
                viewAnalytics();
                break;
            case 5:
                saveBedStatus();
                savePatientRecords();
                printf("Bed status and patient records saved. Exiting system...\n");
                break;
            default: printf("Invalid choice! Pick 1-5.\n");
        }
    }
    return 0;
}
