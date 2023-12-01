from twilio.rest import Client 
import keys 

client = Client(keys.account_sid, keys.auth_token) 

def send_message(text):
    message = client.messages.create(  
    body= "Hi Baby !!! ",
    from_= keys.twilio_number,
    to= keys.client_number
    )
    print(message.body)
