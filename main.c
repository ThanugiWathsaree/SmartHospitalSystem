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

// Initialize bed occupancy matrix
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

// Display bed status across all wards
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

// Allocate a physical bed (returns 1-based bed number, or -1 if full)
int allocateBed(int wardIdx, int occupancy[4][20]) {
    for (int j = 0; j < WARD_CAP[wardIdx]; j++) {
        if (occupancy[wardIdx][j] == 0) {
            occupancy[wardIdx][j] = 1; // Mark occupied
            return j + 1; // 1-based Bed Number
        }
    }
    return -1; // Ward full
}

// Calculate wait time for a specialty and increment queue count
double calcWaitTime(int specIdx) {
    double wait = specQueueCount[specIdx] * SPEC_TIME[specIdx];
    specQueueCount[specIdx]++;
    return wait;
}

// Calculate surcharge based on urgency level
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
    int sIdx = p_specID[idx]; // p_specID is already 0-3 internal index
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
    printf("\n======================================================\n");
    printf("         SMART HOSPITAL ADMISSION & BILL             \n");
    printf("======================================================\n");
    printf("Patient ID           : PAT-%d\n", 1001 + idx);
    printf("Patient Name         : %s\n", p_names[idx]);
    printf("Base Consultation Fee : LKR %10.2f\n", p_baseFee[idx]);
    printf("Emergency Surcharge   : LKR %10.2f\n", p_surcharge[idx]);
    printf("Ward Stay Cost       : LKR %10.2f\n", p_wardCost[idx]);
    printf("Gross Total Bill     : LKR %10.2f\n", p_gross[idx]);
    printf("Age Subsidy Discount : LKR -%9.2f\n", p_discount[idx]);
    printf("------------------------------------------------------\n");
    printf("Final Payable Amount : LKR %10.2f\n", p_netPayable[idx]);
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
        p_bedNum[idx] = allocateBed(p_wardID[idx], bedOccupancy);

        if (p_bedNum[idx] == -1) {
            printf("Warning: Ward full! No physical bed allocated.\n");
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

    patientCount++;

    printf("\nRegistered! Est Wait: %.0f min (Queue Pos: %d)\n",
           p_waitTime[idx], p_queuePos[idx]);

    // Print itemized admission and bill receipt
    printBill(idx);
}

// Display triage queue sorted by urgency (descending)
void displayTriageQueue() {
    if (patientCount == 0) {
        printf("\nNo patients registered yet.\n");
        return;
    }

    int indices[MAX_PATIENTS];
    for (int i = 0; i < patientCount; i++) indices[i] = i;

    // Sort descending by urgency using bubble sort
    for (int i = 0; i < patientCount - 1; i++) {
        for (int j = 0; j < patientCount - i - 1; j++) {
            if (p_urgency[indices[j]] < p_urgency[indices[j+1]]) {
                int tmp = indices[j];
                indices[j] = indices[j+1];
                indices[j+1] = tmp;
            }
        }
    }

    printf("\n--- TRIAGE QUEUE (Sorted by Urgency) ---\n");
    printf("%-5s | %-22s | Age | Urg | Specialty           | Status\n", "Rank", "Name");
    printf("--------------------------------------------------------------------------\n");
    for (int i = 0; i < patientCount; i++) {
        int idx = indices[i];
        printf("%-5d | %-22s | %3d |  %d  | %-21s | %s\n",
               i+1, p_names[idx], p_ages[idx], p_urgency[idx],
               SPEC_NAMES[p_specID[idx]], p_isAdmitted[idx] ? "Admitted" : "OPD");
    }
}

// View financial and queue analytics
void viewAnalytics() {
    printf("\n--- ANALYTICS & REPORTS ---\n");
    printf("Total Patients: %d\n", patientCount);
    double totalRevenue = 0;
    for (int i = 0; i < patientCount; i++) totalRevenue += p_netPayable[i];
    printf("Total Revenue : LKR %.2f\n", totalRevenue);

    printf("\nQueue Load per Specialty:\n");
    for (int i = 0; i < 4; i++) {
        printf("- %-24s: %d registrations\n", SPEC_NAMES[i], specQueueCount[i]);
    }
}

// Display main menu UI
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
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
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
