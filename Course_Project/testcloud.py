import random
import time
from Send_to_cloud_iotc import send_to_cloud
temperature= str(random.randint(0, 40))
humidity = str(random.randint(0, 100))
light_intensity = str(random.randint(800, 1000))
soil_moisture = str(random.randint(100, 1000))

send_to_cloud(25,humidity,light_intensity,soil_moisture)