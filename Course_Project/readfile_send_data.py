import random
import time
from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents
import keys
from sendtxt_v1 import send_message

doorflag = 0
lightflag = 0
fanflag = 0
pumpflag = 0


def send_to_cloud(iotc, temperature, humidity, light_intensity, soil_moisture, door_state, light_state, fan_state, pump_state):
    if iotc.is_connected():
        iotc.send_telemetry({
            'temperature': temperature,
            'humidity': humidity,
            'light_intensity': light_intensity,
            'soil_moisture': soil_moisture,
            "door_state": door_state,
            "light_state": light_state,
            "fan_state": fan_state,
            "pump_state": pump_state
        })
        

def on_commands(command: Command):
    print(f"{command.name} command was sent")
    command.reply()

if __name__ == "__main__":
    
    '''
    Initialize and connect the IoT client
    '''
    iotc = IoTCClient(
        keys.device_id,
        keys.scope_id,
        IOTCConnectType.IOTC_CONNECT_DEVICE_KEY,
        keys.device_key)

    iotc.connect()
    iotc.on(IOTCEvents.IOTC_COMMAND, on_commands)

    
    '''
    Initialize the sensor data
    '''
    # Initialize sensor data variables
    temperature, humidity, light_intensity, soil_moisture = 0.0, 0.0, 0.0, 0.0
    door_state, light_state, fan_state, pump_state = 0, 0, 0, 0

    '''
    parse the file
    '''
    
    # Read and process the sensor data file
    with open('sensor_data8.txt', 'r') as file:
        for line in file:
            if "DHT_Temperature" in line:
                temperature = float(line.split('=')[-1].strip())
            elif "Humidity" in line:
                humidity = float(line.split('=')[-1].strip())
            elif "Light intensity" in line:
                light_intensity = float(line.split('=')[-1].strip())
            elif "Soil Moisture" in line:
                soil_moisture = float(line.split('=')[-1].strip())
            elif "Door State" in line:
                door_state = 0 if "Closed" in line else 1
            elif "LIGHTS" in line:
                light_state = 1 if "ON" in line else 0
            elif "FANS ON" in line:
                fan_state = 1 if "ON" in line else 0
            elif "PUMP" in line:
                pump_state = 1 if "ON" in line else 0

            # Send the parsed data to the cloud
            send_to_cloud(iotc, temperature, humidity, light_intensity, soil_moisture, door_state, light_state, fan_state, pump_state)
            time.sleep(2)

    # Disconnect after all data has been sent
    iotc.disconnect()


