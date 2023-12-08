import random
import time

from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

scope_id = '0ne00B6D4A2'
device_id = 'scv5qwawrz'
device_key = 'xdtpUXfIqjTO9St6mlweOCa2Gx9qFawjkfMjgckvyi4='

def send_to_cloud(temperature, humidity, light_intensity, soil_moisture, door_state, light_state, fan_state, pump_state):
    def on_commands(command: Command):
        print(f"{command.name} command was sent")
        command.reply()

    iotc = IoTCClient(
        device_id,
        scope_id,
            IOTCConnectType.IOTC_CONNECT_DEVICE_KEY,
            device_key)

    iotc.connect()

    iotc.on(IOTCEvents.IOTC_COMMAND, on_commands)

    iotc.send_property({
        "LastTurnedOn": time.time()
    })

    while iotc.is_connected():

    # if iotc.is_connected():
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

        time.sleep(10)


temperature= str(random.randint(0, 40))
humidity = str(random.randint(0, 100))
light_intensity = str(random.randint(800, 1000))
soil_moisture = str(random.randint(100, 1000))
door_state = 0
light_state = 0
fan_state = 0
pump_state = 0

send_to_cloud(temperature,humidity, light_intensity, soil_moisture, door_state, light_state, fan_state, pump_state)