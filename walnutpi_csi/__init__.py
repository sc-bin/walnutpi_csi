import cv2

from walnutpi_csi.aw_isp import aw_isp


class isp():
    def __init__(self, cap :cv2.VideoCapture, isp_index: int = 0):
        cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('N','V','2','1'))
        self.isp = aw_isp(isp_index)
  
