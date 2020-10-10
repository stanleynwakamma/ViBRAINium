import zbar
import cv2
#import pymysql 
import imutils
from PIL import Image
import string
import re
import pymongo 

client = pymongo.MongoClient("mongodb+srv://admin:toor@besmartcluster.2axqx.mongodb.net/?retryWrites=true&w=majority")

cam = cv2.VideoCapture(0)
found=False

cv2.namedWindow("base-image", cv2.WINDOW_AUTOSIZE) 
cv2.startWindowThread()

def CenterText(text,_img,_color):
    #Calculates the center of the screen 
    font = cv2.FONT_HERSHEY_SIMPLEX
    textsize = cv2.getTextSize(text, font, 1, 2)[0]
    testX = (_img.shape[1] - textsize[0]) / 2
    testY = (_img.shape[0] + textsize[1]) / 2
    cv2.putText(_img,text, (int(testX),int(testY)), font, 1,color=_color,thickness=6)

def ScanTags(found):
    (_,image) = cam.read() 
    image = imutils.resize(image, width=400) 
    nimage = cv2.cvtColor(image,cv2.COLOR_BGR2GRAY)
    nimage = Image.fromarray(nimage)
    scanner = zbar.Scanner()
    results = scanner.scan(nimage)
    for result in results:
        if len(results) != 0:
            try:
                cv2.line(image, result.position[0],result.position[1], (0, 0, 255), 4)
                cv2.line(image, result.position[1],result.position[2], (0, 0, 255), 4)
                cv2.line(image, result.position[2],result.position[3], (0, 0, 255), 4)
                cv2.line(image, result.position[3],result.position[0], (0, 0, 255), 4)
            except IndexError:
                pass
            for x in mycol.find({},{ "_id": re.sub(r'['+chars+']', '',str(result.data.decode("utf-8"))[:10]))}):
            #     name=ex.fetchall()[0]['name']
            #     #Centers the text
            #     CenterText("Welcome, {0}".format(name),image,(0,255,0))
            #     print(result.data)
            # else:
            #     CenterText("ACCESS DENIED",image,(0,0,255))
    return image,found

while True and found == False:
    key = cv2.waitKey(1) & 0xFF
    img, found = ScanTags(found)
    cv2.imshow("base-image",img)
    if key == ord("q"):
        break
cam.release()
