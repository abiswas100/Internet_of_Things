import os
from flask import Flask, request, redirect
from twilio.twiml.messaging_response import MessagingResponse
from sendtxt_v1 import send_message

app = Flask(__name__)

file_path = 'sensor_data6.txt'

# Adjusting the do_analysis function to handle the specific formatting of the sensor names in the file

def do_analysis(file_path):
    # Dictionary to store the latest 5 seconds of data for each sensor
    # Adjusted to match the format in the file
    sensor_data = {
        "Humidity (%)": [],
        "DHT_Temperature  (C)": [],
        "Light intensity": [],
        "Soil Moisture": []
    }

    # Read the file and process each line
    with open(file_path, 'r') as file:
        for line in file:
            # Check if the line contains sensor data
            if "=" in line:
                # Extract the sensor name and value
                parts = line.split('=')
                sensor_name = parts[0].strip()
                sensor_value = parts[1].strip().split()[0]  # Extracting the first part of the value

                # Check if the sensor is one of the ones we're interested in
                if sensor_name in sensor_data:
                    try:
                        # Convert the sensor value to a float and add it to the list
                        sensor_data[sensor_name].append(float(sensor_value))
                    except ValueError:
                        # In case the conversion fails (e.g., non-numeric value)
                        pass

    # Compute the range for each sensor
    sensor_ranges = {sensor: (min(values), max(values)) for sensor, values in sensor_data.items() if values}
    return sensor_ranges

# Call the function and print the results
sensor_ranges = do_analysis(file_path)
sensor_ranges
 
        
def format_sensor_data(sensor_ranges):
    # Create a string representation of the sensor data
    message_body = "Sensor Data Change Ranges:\n"
    for sensor, ranges in sensor_ranges.items():
        message_body += f"{sensor}: {ranges[0]} to {ranges[1]}\n"
    return message_body

@app.route("/sms", methods=['GET', 'POST'])
def smsreply():
    # Extract the message text from the incoming request
    incoming_msg = request.form.get('Body').strip()

    # Print the incoming message to the console
    print(f"Received message: {incoming_msg}")

    # Create a Twilio response
    resp = MessagingResponse()

    # Check if the incoming message is "Give 5 sec data"
    if incoming_msg.lower() == "give 5 min data changes":
        times = 5
        sensor_ranges = do_analysis(file_path)
        
        formatted_text = format_sensor_data(sensor_ranges)
        
        # For example, send a specific response
        send_message("Performing the desired action for 'Give 5 min data'")
        send_message(formatted_text)

        # Respond with a confirmation message
        resp.message("Received 'Give 5 min data', performing action...")
    else:
        # Respond with a different message for other inputs
        resp.message("Unrecognized command.")
        send_message("Hello ! From Smart Green House ...")
    return str(resp)

if __name__ == '__main__':
    app.run(debug=True)
