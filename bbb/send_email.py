import smtplib
import datetime
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from email.mime.base import MIMEBase
from email import encoders


def send_alert(path_to_image, movement):
    #Email Info for the sender and the receiver
    email_user = 'someone@gmail.com'
    email_password = 'password'
    email_send = 'someone@gmail.com'

    subject = ''
    body = ''

    # param: movement = 0, alert person not recognized
    # param: movment = 1, unusual movement detected
    if movement is 0:
        subject = 'Alert! Attempted Unauthorized Access to theBox. Alert!'
        body = 'This individual attempted to access the box at: {} '.format(datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
    elif movement is 1:
        subject = 'Alert! Unusual Movement Detected by theBox. Alert!'
        body = 'theBox has detected unusual movement activity at: {}\n Image Taken Below.'.format(datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))

    #Form the parts of the email
    msg = MIMEMultipart()
    msg['From'] = email_user
    msg['To'] = email_send
    msg['Subject'] = subject

    #Attach the body to the email
    msg.attach(MIMEText(body, 'plain'))

    #Load and open an image
    filename = path_to_image
    attachment = open(filename, 'rb')

    #Attach the image to the email as an attachment
    part = MIMEBase('application', 'octet-stream')
    part.set_payload((attachment).read())
    encoders.encode_base64(part)
    part.add_header('Content-Disposition', "attachment; filename= " + filename)

    #Attach the email as an attachment & server information of the email server of the sender
    msg.attach(part)
    text = msg.as_string()
    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.starttls()
    server.login(email_user, email_password)

    #Send the email
    server.sendmail(email_user, email_send, text)
    server.quit()
