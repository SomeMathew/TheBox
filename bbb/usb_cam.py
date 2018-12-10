import subprocess
import time




def take_picture_and_recognize():
	
	#bash script to take a picture with the USB Cam
	subprocess.call(['/var/lib/cloud9/theBox/usb_cam.sh'])
	time.sleep(0.05)
	subprocess.call(['/var/lib/cloud9/theBox/usb_cam.sh'])
	
	#Import the facial recognition module
	from person_recognition import person_recognition
	
	#Test unknown person and see if they are recognized
	unknown_face = '/var/lib/cloud9/theBox/unknown.jpg'
	unknown_person = person_recognition(unknown_face)

	return unknown_person

	
