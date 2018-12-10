import face_recognition
import os

def person_recognition(unknown_face):
    known_dir = "/var/lib/cloud9/theBox/known/" # Directory holding all of the known images
    directory = os.fsencode(known_dir)

    # These lists will hold the image files and their encodings
    known_images = []
    known_encodings = []

    # Load all the known images
    for file in os.listdir(directory):
        filename = os.fsdecode(file)
        known_images.append(face_recognition.load_image_file(known_dir + filename))

    # Load the unknown image
    unknown_image = face_recognition.load_image_file(unknown_face)

    # Try to encode all the images
    try:
        for image in known_images:
            known_encodings.append(face_recognition.face_encodings(image)[0]) #0th face
        unknown = face_recognition.face_encodings(unknown_image)[0]
    except IndexError:
            print("Could not find faces...")
            return False
            quit()

    results = face_recognition.compare_faces(known_encodings, unknown)

    # Check if there is a valid match
    for match in results:
        if match:
            return True
            quit()
        else:
            return False