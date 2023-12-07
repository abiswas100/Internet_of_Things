import os
from flask import Flask, request, redirect
from twilio.twiml.messaging_response import MessagingResponse
from sendtxt_v1 import send_message

app = Flask(__name__)

@app.route("/sms", methods=['GET', 'POST'])
def smsreply():

    send_message("Hello ! From Smart Green House ...")
    return 1

if __name__ == '__main__':
    app.run(debug=True)
