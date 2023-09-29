import random
import time

from iotc.models import Command, Property
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

scopeId = '0ne00AE04F2'
device_id = 'ec3aqb46ip'
device_key = 'WDhq8Ep7MheHsWci1qGYqq26XrrimZfJ8nquRgT7A4Y='

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
