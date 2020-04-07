import cv2
import matplotlib.pyplot as plt
import numpy as np
import json

def read_track_1():
    image = cv2.imread("track1.png")
    h, w, _  = image.shape

    nh, nw = 32, 17
    tracks = json.load(open("track_1_policy.json", "r"))
    for position_x, steps in tracks.items():
        color = list(map(int, np.random.randint(0, 256, 3)))
        shift = np.random.rand()*0.2 - 0.1
        for i in range(len(steps) -1):
            begin = (int(w/nw*(steps[i][1] + 0.5+ shift)), int(h/nh*(steps[i][0] + 0.5+ shift)))
            end = (int(w/nw*(steps[i+1][1] + 0.5+ shift)), int(h/nh*(steps[i+1][0] + 0.5+ shift)))
            image = cv2.line(image, begin, end, color, 3) 
            image = cv2.circle(image, end, 5, color, -1)
    cv2.imshow('image', image)
    cv2.imwrite('track1_visualization.png', image)
    cv2.waitKey()

def read_track_2():
    image = cv2.imread("track2.png")[:-3,2 :-3, :]
    h, w, _  = image.shape

    nh, nw = 30, 32
    tracks = json.load(open("track_2_policy.json", "r"))

    for position_x, steps in tracks.items():
        color = list(map(int, np.random.randint(0, 256, 3)))
        shift = np.random.rand()*0.2 - 0.1
        for i in range(len(steps) -1):
            begin = (int(w/nw*(steps[i][1] + 0.5+ shift)), int(h/nh*(steps[i][0] + 0.5+ shift)))
            end = (int(w/nw*(steps[i+1][1] + 0.5+ shift)), int(h/nh*(steps[i+1][0] + 0.5+ shift)))
            image = cv2.line(image, begin, end, color, 3) 
            image = cv2.circle(image, end, 5, color, -1)
    cv2.imshow('image', image)
    cv2.imwrite('track2_visualization.png', image)
    cv2.waitKey()


if __name__ == "__main__":
    read_track_1()
    read_track_2()
