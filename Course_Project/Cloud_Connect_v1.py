import random
import time

from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

scope_id = '0ne00B61247'
device_id = 'SmartGreenHouse'
device_key = 'Jue8raq3j7G8PTZVTFfhNnd3IRTDbsIcAqdnac3qJrM='

def send_to_cloud(temp,humidity,light_intensity,door_state,soil_moisture):
    LastTurnedOn = time.time()
    # temp = str(random.randint(0, 40))
    # humidity = str(random.randint(1, 10))
    # light_intensity =  str(random.randint(0,100))
    # door_state = str(random.randint(0, 1))
    # soil_moisture = str(random.randint(100, 200))

    telemetry_str = "DHT_Temperature: {}, Humidity: {}, Light_Intensity: {}, Door_State: {}, Soil_Moisture: {}".format(temp, humidity, light_intensity, door_state, soil_moisture)

    def on_commands(command: Command):

        print(f"{command.name} command was sent")
        iotc.send_property({
        "LastPowerOn": LastTurnedOn
        })
        iotc.send_property({
        "LastCommandReceived": time.time()
        })
        iotc.send_property({
            "SendTelemetry" : telemetry_str
        })

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
            'DHT_Temperature': str(random.randint(0, 40)),
            'Humidity' : str(random.randint(1, 10)),
            'Light_Intensity' : str(random.randint(0,100)),
            'Door_State' : str(random.randint(0,1)),
            'Soil_Moisture' : str(random.randint(100,200)),
        })
        time.sleep(1)
