#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void showMenu() {
    printf("\n=========================================\n");
    printf("   SMART HOSPITAL SYSTEM - MAIN MENU\n");
    printf("=========================================\n");
    printf("1. Register New Patient\n");
    printf("2. Display Bed Availability\n");
    printf("3. Display Triage Queue (Sorted)\n");
    printf("4. View Analytics & Reports\n");
    printf("5. Save & Exit\n");
    printf("Enter choice: ");
}

int main() {
    int choice = 0;
    while (choice != 5) {
        showMenu();
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                printf("Patient Registration (Pending implementation...)\n");
                break;
            case 2:
                printf("Bed Availability (Pending implementation...)\n");
                break;
            case 3:
                printf("Triage Queue (Pending implementation...)\n");
                break;
            case 4:
                printf("Analytics Report (Pending implementation...)\n");
                break;
            case 5:
                printf("Exiting System...\n");
                break;
            default:
                printf("Invalid choice! Try again.\n");
        }
    }
    return 0;
}
