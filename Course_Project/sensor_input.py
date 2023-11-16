from pyfirmata import Arduino, util
import time

# Specify the port to which your Arduino is connected
PORT = '/dev/ttyACM0'  # Change this to your Arduino's port

# Initialize the board
board = Arduino(PORT)

# Define the built-in LED pin
led_pin = board.get_pin('d:13:o')  # 13 is the built-in LED pin for most Arduinos

while True:
    led_pin.write(1)  # Turn LED on
    time.sleep(1)     # Wait for 1 second
    led_pin.write(0)  # Turn LED off
    time.sleep(1)     # Wait for another 1 second
