import _aw_isp

class aw_isp:
    def __init__(self, index:int):
        self.index = index
        _aw_isp.start(index)
