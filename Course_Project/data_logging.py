import serial
from sendtxt_v1 import send_message
# from Cloud_Connect_v1 import send_to_cloud


# Replace 'COM3' with the appropriate port your Arduino is connected to
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)  

file_path = 'sensor_data6.txt'  # Change the file path as needed
doorflag = 0
sensor_values = {
    "Time": None,
    "Soil Moisture": None,
    "Door State": None,
    "Humidity (%)": None,
    "DHT_Temperature (C)": None,
    "Light intensity": None
}


# Modify the update_sensor_values functio
def get_sensor_values(data, sensor_values):
    for item in data:
        # Splitting the string into time and message
        _, message = item.split(" -> ", 1)

        # Check if the message contains '=' indicating a key-value pair
        if '=' in message:
            key, value = message.split(" = ", 1)
            try:
                # Special handling for Door State
                if key == "Door State":
                    value = 1 if value == "Opened" else 0
                else:
                    # Trying to convert other values to a float
                    value = float(value)
            except ValueError:
                # If conversion fails, store the value as is (for non-numeric values like "Opened")
                pass

            # Update the sensor value
            sensor_values[key] = value

    return sensor_values

# Function to extract numbers from the strings
def extract_serial(data):
    extracted_values = {}
    for item in data:
        # Splitting the string into key and value using a colon
        key, value = item.split(": ")
        try:
            # Trying to convert the value to a float
            extracted_values[key] = float(value)
        except ValueError:
            # If conversion fails, store the value as is
            extracted_values[key] = value
    return extracted_values


if __name__ == "__main__": 
    with open(file_path, 'w') as file:
        while True:
            # Read data from Arduino
            data = ser.readline().decode().strip()
            
            if data:  # If data is received
                print("Data received:", data)
                
                # Write data to the text file
                file.write(data + '\n')
                
            if(data == 'Door State = Opened'):
                doorflag = 1
                send_message("ALERT : Door Open")

            if(data == 'Dry soil! turning PUMP ON'):
                send_message("INFO : Dry Soil, tuning pump ON !")

            if(data == 'Too hot! turning FANS ON'):
                send_message("INFO : Too Hot, Turning FANS ON !")

            if(data == 'Night Time! turning LIGHTS ON'):
                send_message("INFO : Night Time, Turning LIGHTS ON !")

            if(data == 'Door State = Closed' and doorflag == 1):
                doorflag = 0
                send_message("INFO : Door Closed NOW !!") 

            if data.startswith("Time"):
                humidity = sensor_values["Humidity (%)"]
                time = sensor_values["Time"]
                soil_m = sensor_values["Soil Moisture"]
                door_state = sensor_values["Door State"]
                temp = sensor_values["DHT_Temperature (C)"]
                light = sensor_values["Light intensity"]

                # send_to_cloud(temp,humidity,light,door_state,soil_m)
