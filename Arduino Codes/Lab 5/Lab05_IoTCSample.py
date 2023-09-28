import random
import time

from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

scope_id = '0ne00ADFCBE'
device_id = '1kivxnhp2a0'
device_key = 'bmOIBczHO7aCsoNhxf+tqhy+BxtbRrnxMmQWnKx8SVI='


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
