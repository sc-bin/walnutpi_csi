from setuptools import setup, Extension, find_packages
import os
from setuptools.command.build_ext import build_ext
import subprocess

# 检查是否在目标平台上
__model_path = "/proc/device-tree/model"
__model_path_tmp = "/tmp/walnutpi-board_model"  # 方便walnutpi-build构建镜像时判断
_model = None
if os.path.exists(__model_path):
    with open(__model_path, "r") as f:
        _model = f.read().strip()
    print(f"Model: {_model}")
elif os.path.exists(__model_path_tmp):
    with open(__model_path_tmp, "r") as f:
        _model = f.read().strip()
    print(f"Model: {_model}")
else:
    print("Model: Unknown")


ext_modules = []
if _model is not None:
    current_dir = os.path.dirname(os.path.abspath(__file__))
    if "walnutpi-2b" in _model:
        ext_modules = [
            Extension(
                "_aw_isp",
                sources=["walnutpi_csi/aw_isp/py_isp.c"],
                libraries=["AWIspApi"],
                library_dirs=[
                    current_dir,
                ],
                include_dirs=[os.path.join(current_dir, "walnutpi_csi/aw_isp/")],
            )
        ]

# 项目元数据现在从 pyproject.toml 读取，所以一些东西不在这里定义
setup(
    packages=find_packages(),
    ext_modules=ext_modules,
    package_data={
        "": ["*.so", "lib/*.so"],
    },
    include_package_data=True,
)
