

import random
import time

from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

import keys

import random
import time

from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

# scope_id = '0ne00B6D4A2'
# device_id = 'scv5qwawrz'
# device_key = 'xdtpUXfIqjTO9St6mlweOCa2Gx9qFawjkfMjgckvyi4='

def send_to_cloud(temperature, humidity, light_intensity, soil_moisture, door_state, light_state, fan_state, pump_state):
    def on_commands(command: Command):
        print(f"{command.name} command was sent")
        command.reply()

    iotc = IoTCClient(
        keys.device_id,
        keys.scope_id,
            IOTCConnectType.IOTC_CONNECT_DEVICE_KEY,
            keys.device_key)

    iotc.connect()

    iotc.on(IOTCEvents.IOTC_COMMAND, on_commands)

    # iotc.send_property({
    #     "LastTurnedOn": time.time()
    # })

    # while iotc.is_connected():

    if iotc.is_connected():
        iotc.send_telemetry({
            'temperature': temperature,
            'humidity' : humidity,
            'light_intensity' : light_intensity,
            'soil_moisture' : soil_moisture,
            "door_state": door_state,
            "light_state" : light_state,
            "fan_state" :fan_state,
            "pump_state" : pump_state
        })
        iotc.disconnect()
        # time.sleep(10)
    return 1


if __name__ == "__main__":
    
    temperature, humidity, light_intensity, soil_moisture = 0.0, 0.0, 0.0, 0.0
    door_state, light_state, fan_state, pump_state = 0,0,0,0

    with open('sensor_data8.txt', 'r') as file:
        for line in file:
            if "DHT_Temperature" in line:
                    temperature = line.split('=')[-1].strip()
            elif "Humidity" in line:
                humidity = line.split('=')[-1].strip()
            elif "Light intensity" in line:
                light_intensity = line.split('=')[-1].strip()
            elif "Soil Moisture" in line:
                soil_moisture = line.split('=')[-1].strip()
            elif "Door State" in line:
                door_state = 0 if "Closed" in line else 1
            elif "LIGHTS" in line:
                light_state = 1 if "ON" in line else 0
            elif "FANS ON" in line:
                fan_state = 1 if "ON" in line else 0
            elif "PUMP" in line:
                pump_state = 1 if "ON" in line else 0
                
            print(temperature, humidity, light_intensity, soil_moisture, door_state, light_state, fan_state, pump_state)
            print("here")
            returned = send_to_cloud(temperature, humidity, light_intensity, soil_moisture, door_state, light_state, fan_state, pump_state)
            
            