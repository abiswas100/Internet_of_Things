import requests

iotc_sub_domain = "your_domain"
device_id = "your_device_id"
api_key = "your_api_key"

def _command_url():
    return f"https://{iotc_sub_domain}.azureiotcentral.com/api/devices/{device_id}/commands/SendData?api-version=2022-05-31"
    
def send_command():
    resp = requests.post(
        _command_url(),
        json={},
        headers={"Authorization": api_key}    
    )
    print(resp.json())
    
print(send_command())