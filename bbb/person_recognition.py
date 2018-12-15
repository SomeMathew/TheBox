import face_recognition
import os

def load_images(known_images):
    known_dir = "/var/lib/cloud9/theBox/known/" # Directory holding all of the known images
    directory = os.fsencode(known_dir)

    # These lists will hold the image files and their encodings

    # Load all the known images
    for file in os.listdir(directory):
        filename = os.fsdecode(file)
        known_images.append(face_recognition.load_image_file(known_dir + filename))

def load_unknown_image(known_images, known_encodings, unknown_face):
    # Load the unknown image
    unknown_image = face_recognition.load_image_file(unknown_face)
    # Try to encode all the images
    try:
        #If known encodings is empty, encode all known images
        if not known_encodings:
            for image in known_images:
                known_encodings.append(face_recognition.face_encodings(image)[0]) #0th face
        #Encodings already loaded
        unknown = face_recognition.face_encodings(unknown_image)[0]
    except IndexError:
            print("Could not find faces...")
            quit()
    return unknown

def person_recognition(unknown, known_encodings):
    results = face_recognition.compare_faces(known_encodings, unknown)

    # Check if there is a valid match
    for match in results:
        if match:
            return True
            quit()
        else:
            return False