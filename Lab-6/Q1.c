#include <string.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "LCD.h"
#include "Scankey.h"
#include "clk.h"      

// --- ???? (??????) ---
#define KEY_UP 4
#define KEY_DOWN 6
#define KEY_S 5
#define KEY_R 7  // R?
#define KEY_B 8
#define KEY_C 9
// --------------------------------------------------

// --- ??????? (Random) ?? ---
static uint32_t g_seed; // ???????
void my_srand(uint32_t seed) { g_seed = seed; }
int my_rand(void)
{
    // ?? g_seed ? 0 (?????),??????????
    if (g_seed == 0) g_seed = 1;
   
    g_seed = (1103515245 * g_seed + 12345) & 0x7FFFFFFF; // Standard LCG values
    return (int)g_seed;
}
// --------------------------------------------------

// --- ????? itoa (Integer-to-String) ?? ---
void simple_itoa(int val, char *buf)
{
    int i = 0;
    char temp[10];
    int j = 0;
   
    // Handle 0 explicitly
    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
   
    // Process number in reverse order
    while (val > 0) {
        temp[j++] = (val % 10) + '0'; // Get last digit
        val /= 10;                     // Remove last digit
    }
   
    // Reverse the temporary string into the output buffer
    while (j > 0) {
        buf[i++] = temp[--j];
    }
   
    // Null-terminate the string
    buf[i] = '\0';
}
// --------------------------------------------------


// --- ???? ---
int numbers[4];         // ?? 4 ????
int sum = 0;            // ??????
int cursor_pos = 0;     // ???? (0-3)
int view_offset = 0;    // LCD ????? (0 ? 1)
int selected_numbers[4]; // ??,?????????? (?? Backspace)
int selected_count = 0; // ?????? (0-4)


// --- LED ??? ---
void init_leds(void)
{
    GPIO_SetMode(PC, BIT12 | BIT13 | BIT14 | BIT15, GPIO_PMD_OUTPUT);
    PC12 = 1; PC13 = 1; PC14 = 1; PC15 = 1; // LEDs off (assuming active-low)
}

// --- ?? LED ?? ---
void update_leds(void)
{
    // LEDs turn on based on the number of selected items (active-low)
    PC12 = (selected_count > 0) ? 0 : 1;
    PC13 = (selected_count > 1) ? 0 : 1;
    PC14 = (selected_count > 2) ? 0 : 1;
    PC15 = (selected_count > 3) ? 0 : 1;
}

// --- Buzzer ??? (Active-Low) ---
void init_buzzer(void)
{
    // Set PB11 as output mode
    GPIO_SetMode(PB, BIT11, GPIO_PMD_OUTPUT);
    // Default OFF (PB11 = 1 for active-low buzzer)
    PB11 = 1;
}

// --- ???? Buzz ?? ---
void Buzz(int number)
{
    int i;
    for (i=0; i<number; i++) {
        PB11=0; // PB11 = 0 to turn ON Buzzer
        CLK_SysTickDelay(100000);     // Delay 100ms
        PB11=1; // PB11 = 1 to turn OFF Buzzer
        CLK_SysTickDelay(100000);     // Delay 100ms
    }
}


// --- ???????? ---
void generate_numbers(void)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        // Generate a two-digit number (10-99)
        numbers[i] = my_rand() % 90 + 10;
    }
}

// --- ?? LCD ?? (????, no sprintf) ---
void update_display(void)
{
    char line_buffer[16 + 1]; // Buffer for LCD line (16 chars + null terminator)
    char num_buffer[10];      // Buffer for integer conversion
    int i, j;

    // --- Line 0: Display SUM ---
    // 1. Clear buffer with spaces
    for(i=0; i<16; i++) line_buffer[i] = ' ';
   
    // 2. Manually place "SUM = "
    line_buffer[0] = 'S'; line_buffer[1] = 'U'; line_buffer[2] = 'M';
    line_buffer[3] = ' '; line_buffer[4] = '='; line_buffer[5] = ' ';
   
    // 3. Convert sum to string
    simple_itoa(sum, num_buffer);
   
    // 4. Manually copy number string into buffer
    i = 0;
    while (num_buffer[i] != '\0' && (6 + i) < 16) {
        line_buffer[6 + i] = num_buffer[i];
        i++;
    }
    line_buffer[16] = '\0'; // Ensure null termination (print_Line might need it)
    print_Line(0, line_buffer);

    // --- Line 1-3: Display 3 numbers from the list ---
    for (j = 0; j < 3; j++)
    {
        int num_index = j + view_offset; // Index of the number to display
        int has_cursor = (num_index == cursor_pos);

        // 1. Clear buffer with spaces
        for(i=0; i<16; i++) line_buffer[i] = ' ';

        // 2. Place cursor '>' or space
        line_buffer[0] = (has_cursor ? '>' : ' ');
        line_buffer[1] = ' '; // Space after cursor

        // 3. Convert number to string
        simple_itoa(numbers[num_index], num_buffer);

        // 4. Manually copy number string
        i = 0;
        while (num_buffer[i] != '\0' && (2 + i) < 16) {
            line_buffer[2 + i] = num_buffer[i];
            i++;
        }
        line_buffer[16] = '\0'; // Ensure null termination
        print_Line(j + 1, line_buffer); // Display on lines 1, 2, 3
    }
}

// --- ??? ---
int main(void)
{
    uint8_t keyin;
    // Variable for Key-Release detection logic
    uint8_t last_keyin = 0;
    // Your random seed counter
    uint32_t count = 0;

    // --- Hardware Initialization ---
    SYS_Init();
    init_LCD();
    clear_LCD();
    OpenKeyPad();
    init_leds();
    init_buzzer(); // Initialize the buzzer

    // --- Initial State ---
    // (We don't seed here, so the first run produces predictable numbers)
    generate_numbers();
    update_display();   // Update LCD display

    // --- Main Loop ---
    while (1)
    {
        count++; // Increment count in the loop for seeding randomness
        keyin = ScanKey(); // Scan for key press
       
        // --- Key-Release Detection Logic (avoids Buzz() conflict) ---
        // Only process the key action when the key is *released*
        if (keyin == 0 && last_keyin != 0)
        {
            // Process the key that was just released (stored in last_keyin)
            switch (last_keyin)
            {
            case KEY_UP: // Up arrow
                if (cursor_pos > 0)
                {
                    cursor_pos--;
                    // Adjust view offset if cursor moves out of the 3 visible lines
                    view_offset = (cursor_pos == 3) ? 1 : 0;
                }
                break;

            case KEY_DOWN: // Down arrow
                if (cursor_pos < 3)
                {
                    cursor_pos++;
                    // Adjust view offset if cursor moves to the 4th item
                    view_offset = (cursor_pos == 3) ? 1 : 0;
                }
                break;

            case KEY_S: // Select
                // Only add if less than 4 numbers are selected
                if (selected_count < 4)
                {
                    int selected_val = numbers[cursor_pos];
                    sum += selected_val;
                    // Store the selected number for Backspace functionality
                    selected_numbers[selected_count] = selected_val;
                    selected_count++;
                    update_leds(); // Update LED status
                    Buzz(1);       // Beep once on selection (when key is released)
                }
                break;

            case KEY_R: // Reset
                my_srand(count); // Use the current count as the random seed
                generate_numbers(); // Generate new random numbers
                sum = 0;            // Reset sum
                selected_count = 0; // Reset selected count
                cursor_pos = 0;     // Reset cursor position
                view_offset = 0;    // Reset view offset
                update_leds();      // Turn off LEDs
                break;

            case KEY_B: // Backspace
                // Only perform backspace if numbers have been selected
                if (selected_count > 0)
                {
                    selected_count--; // Decrement selected count
                    // Subtract the last selected number from the sum
                    sum -= selected_numbers[selected_count];
                    update_leds(); // Update LED status
                }
                break;

            case KEY_C: // Clear
                sum = 0;            // Reset sum
                selected_count = 0; // Reset selected count
                // Cursor position and generated numbers remain unchanged
                update_leds();      // Turn off LEDs
                break;
               
            default:
                // Ignore other keys (1, 2, 3)
                break;
            }
           
            // Update the display only after processing the key release
            update_display();
        }
       
        // Update last_keyin for the next loop iteration
        last_keyin = keyin;
       
        // Small delay to reduce CPU load from constant polling
        CLK_SysTickDelay(10000); // 10ms delay
    }
    // The program should never exit the while(1) loop
    // return 0; // Usually unreachable in embedded systems
}
