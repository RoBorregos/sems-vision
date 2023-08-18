FROM nvcr.io/nvidia/l4t-base:r32.7.1

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
      git \
      python3.8 python3.8-dev python3-pip \
      libopenmpi-dev libomp-dev libopenblas-dev libblas-dev libeigen3-dev \
      gfortran

RUN python3.8 -m pip install --upgrade pip
RUN python3.8 -m pip install cmake
RUN python3.8 -m pip install dlib==19.21.0 -vvv
RUN python3.8 -m pip install scipy==1.5.2 -vvv
RUN python3.8 -m pip install setuptools gdown

# pytorch 1.11.0
RUN gdown https://drive.google.com/uc?id=1hs9HM0XJ2LPFghcn7ZMOs5qu5HexPXwM
RUN python3.8 -m pip install torch-*.whl

# torchvision 0.12.0
RUN gdown https://drive.google.com/uc?id=1m0d8ruUY8RvCP9eVjZw4Nc8LAwM8yuGV
RUN python3.8 -m pip install torchvision-*.whl

#install yolov8 + sems-vision requirements
COPY docker/yolov8requirements.txt /yolov8requirements.txt
RUN python3.8 -m pip install -r yolov8requirements.txt
#RUN git clone https://github.com/ultralytics/yolov5.git
#WORKDIR yolov5
#RUN python3.8 -m pip install -r requirements.txt

RUN mkdir /galeriaAbierta
WORKDIR galeriaAbierta