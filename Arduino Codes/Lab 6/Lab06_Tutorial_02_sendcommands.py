import requests

iotc_sub_domain = "avhiapp2"
device_id = "16alpa3zasa"
api_key = "SharedAccessSignature sr=6a1b18f9-8bd5-4414-851c-cb3ac4971ed1&sig=YL0Yh%2FTD0kXo0TDNJfFjgx2mGeNvgA8bX26Yfac%2BJy0%3D&skn=IoTAzureML&se=1727618529424"

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