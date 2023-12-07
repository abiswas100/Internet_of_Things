import random
import time

from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

scope_id = '0ne00B6D4A2'
device_id = 'scv5qwawrz'
device_key = 'xdtpUXfIqjTO9St6mlweOCa2Gx9qFawjkfMjgckvyi4='

def send_to_cloud(temperature, humidity, light_intensity, soil_moisture):
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
        iotc.send_telemetry({
            # 'temperature': str(random.randint(0, 40)),
            # 'humidity' : str(random.randint(0, 100)),
            # 'light_intensity' : str(random.randint(800, 1000)),
            # 'soil_moisture' : str(random.randint(100, 1000))
            'temperature': temperature,
            'humidity' : humidity,
            'light_intensity' : light_intensity,
            'soil_moisture' : soil_moisture
        })
        time.sleep(10)


temperature= str(random.randint(0, 40))
humidity = str(random.randint(0, 100))
light_intensity = str(random.randint(800, 1000))
soil_moisture = str(random.randint(100, 1000))

send_to_cloud(25,humidity,light_intensity,soil_moisture)