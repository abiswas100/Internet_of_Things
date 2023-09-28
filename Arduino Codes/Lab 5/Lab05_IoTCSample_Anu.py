import random
import time

from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

scope_id = '0ne00ADFCBE'
device_id = '2fclswaoqbz'
device_key = '2s83CCMrNp+8MspcJ45dg4Bxz2YGLQpxRebjrZm2z70='

LastTurnedOn = time.time()
temp = str(random.randint(0, 40))
pressure = str(random.randint(1, 10))
humidity =  str(random.randint(0,100))

telemetry_str = "Temperature: {}, Pressure: {}, Humidity: {}".format(temp, pressure, humidity)


def on_commands(command: Command):

    print(f"{command.name} command was sent")
    iotc.send_property({
    "LastPowerOn": LastTurnedOn
    })
    iotc.send_property({
    "LastCommandReceived": time.time()
    })
    iotc.send_property({
        # "SendTelemetry" : iotc.send_telemetry({
        #                                         'Temperature': str(random.randint(0, 40)),
        #                                         'Pressure' : str(random.randint(1, 10)),
        #                                         'Humidity' : str(random.randint(0,100))
        #                                         })
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
        'Temperature': str(random.randint(0, 40)),
        'Pressure' : str(random.randint(1, 10)),
        'Humidity' : str(random.randint(0,100))
    })
    # iotc._send_message(str('hello'))
    time.sleep(60)
