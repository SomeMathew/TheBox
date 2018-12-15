import subprocess
import time
from face_test import person_recognition
from face_test import load_images
from face_test import load_unknown_image

#bash script to take a picture with the USB Cam
def take_picture_and_recognize(known_images, known_encodings):

    #Subprocess to run a bash script take a picture
    subprocess.call(['/var/lib/cloud9/theBox/usb_cam.sh'])
    time.sleep(0.05)
    subprocess.call(['/var/lib/cloud9/theBox/usb_cam.sh'])

    #Unknown image
    unknown_face = '/var/lib/cloud9/theBox/unknown.jpg'

    # Test to see if the list known_images is empty
    if not known_images:

        # If known_images is empty, load the empty list with known images
        load_images(known_images)
        unknown = load_unknown_image(known_images, known_encodings, unknown_face)

        #If the person is recognized, return True. else return false
        if person_recognition(unknown, known_encodings):
            return True
        else:
            return False

    #known_images is not empty and known_images are already loaded
    else:
        unknown = load_unknown_image(known_images, known_encodings, unknown_face)
        # test if person is recognized
        if person_recognition(unknown, known_encodings):
            return True
        else:
            return False

#Function to take a single picture
def take_picture():
    # Subprocess to run a bash script take a picture
    subprocess.call(['/var/lib/cloud9/theBox/usb_cam.sh'])
    time.sleep(0.05)
    subprocess.call(['/var/lib/cloud9/theBox/usb_cam.sh'])