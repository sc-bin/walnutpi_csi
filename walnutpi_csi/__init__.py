import _aw_isp


class isp:
    def __init__(self, index: int = 0):
        self.index = index
        _aw_isp.start(index)
