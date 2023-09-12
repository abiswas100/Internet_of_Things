#include <FlashStorage.h>
// Reserve a portion of flash memory to store an "int" variable
// and call it "my_flash_store".
FlashStorage(my_flash_store, int);
// Note: the area of flash memory reserved for the variable is
// lost every time the sketch is uploaded on the board.
void setup() {
    while(!SerialUSB);
    SerialUSB.begin(9600);
    int number;
    // Read the content of "my_flash_store" and assign it to "number"
    number = my_flash_store.read();
    // Print the current number on the serial monitor
    SerialUSB.println(number);
    // Save into "my_flash_store" the number increased by 1 for the
    // next run of the sketch
    my_flash_store.write(number + 1);
}
void loop() {}