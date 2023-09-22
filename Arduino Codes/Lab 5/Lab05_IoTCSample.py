import random
import time

from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

scope_id = 'YOUR SCOPE ID'
device_id = 'YOUR DEVICE ID'
device_key = 'YOUR PRIMARY SAS KEY'


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
        'Temperature': str(random.randint(0, 40))
    })
    time.sleep(10)
