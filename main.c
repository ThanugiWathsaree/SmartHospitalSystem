#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Specialty Constants
const char SPEC_NAMES[4][30] = {"General Practice (OPD)", "Paediatrics", "Cardiology", "Neurology"};
const double SPEC_FEES[4] = {1500.0, 2500.0, 4500.0, 5000.0};
const int SPEC_TIME[4] = {15, 20, 30, 30};
const int SPEC_CAP[4] = {30, 20, 12, 10};

// Ward Constants
const char WARD_NAMES[4][25] = {"General Ward", "Paediatric Ward", "Surgical Ward", "ICU"};
const double WARD_RATES[4] = {3000.0, 6000.0, 12000.0, 25000.0};
const int WARD_CAP[4] = {20, 10, 10, 5};


int bedOccupancy[4][20];

void initBeds() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 20; j++) {
            bedOccupancy[i][j] = 0; // 0 = Available, 1 = Occupied
        }
    }

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
            case 1:
                printf("\n[Case 1] Patient Registration - Not implemented yet\n");
                break;
            case 2:
                displayBedStatus();
                break;
            case 3:
                printf("\n[Case 3] Triage Queue - Not implemented yet\n");
                break;
            case 4:
                printf("\n--- ANALYTICS (Specialty Rates) ---\n");
                for(int i=0; i<4; i++) {
                    printf("%d. %-24s | Fee: %.2f | Time: %d min\n",
                           i+1, SPEC_NAMES[i], SPEC_FEES[i], SPEC_TIME[i]);
                }
                break;
            case 5:
                printf("Saving and exiting...\n");
                break;
            default:
                printf("Invalid choice! Pick 1-5.\n");
        }
    }
    return 0;
}
