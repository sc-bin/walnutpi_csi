import cv2
import numpy as np
import os
import time
os.environ["DISPLAY"] = ":0.0"
import walnutpi_csi



if __name__ == '__main__':
    cap = cv2.VideoCapture(0)
    isp = walnutpi_csi.isp(cap)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)
    

    if not cap.isOpened():
        print("摄像头初始化失败")
        exit()
    
    print("摄像头已初始化")
    print(f"实际宽度: {cap.get(cv2.CAP_PROP_FRAME_WIDTH)}")
    print(f"实际高度: {cap.get(cv2.CAP_PROP_FRAME_HEIGHT)}")
    print(f"实际FPS: {cap.get(cv2.CAP_PROP_FPS)}")

    print("开始读取摄像头图像...")
    
    while True:
        ret, frame = cap.read()
        if ret:
            cv2.imshow('Image', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
        else:
            print("无法读取摄像头图像")
            break
    cap.release()
    cv2.destroyAllWindows()