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

void showMenu() {
    printf("\n=========================================\n");
    printf("    SMART HOSPITAL SYSTEM - MAIN MENU\n");
    printf("=========================================\n");
    printf("1. Register New Patient\n");
    printf("2. Display Bed Availability (Wards)\n");
    printf("3. Display Triage Queue (Sorted)\n");
    printf("4. View Analytics & Reports (Specialties)\n");
    printf("5. Save & Exit\n");
    printf("Enter choice: ");
}

int main() {
    int choice = 0;
    while (choice != 5) {
        showMenu();
        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input! Please enter a number (1-5).\n");
            while(getchar() != '\n'); // clear input buffer
            continue;
        }

        switch (choice) {
            case 1:
                printf("\n[1] Patient Registration (Pending implementation...)\n");
                break;
            case 2:
                printf("\n--- Ward Availability & Rates ---\n");
                for (int i = 0; i < 4; i++) {
                    printf("%d. %-20s | Rate: Rs. %8.2f | Capacity: %2d beds\n",
                           i+1, WARD_NAMES[i], WARD_RATES[i], WARD_CAP[i]);
                }
                break;
            case 3:
                printf("\n[3] Triage Queue (Sorted) (Pending implementation...)\n");
                break;
            case 4:
                printf("\n--- Specialty Details & Analytics Preview ---\n");
                for (int i = 0; i < 4; i++) {
                    printf("%d. %-22s | Fee: Rs. %6.2f | Time: %2d min | Cap: %2d\n",
                           i+1, SPEC_NAMES[i], SPEC_FEES[i], SPEC_TIME[i], SPEC_CAP[i]);
                }
                break;
            case 5:
                printf("\nSaving data and Exiting System...\n");
                break;
            default:
                printf("\nInvalid choice! Please choose between 1 and 5.\n");
        }
    }
    return 0;
}
