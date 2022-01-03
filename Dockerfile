FROM tlcpackstaging/ci_gpu:20211226-225913-218d2919f

RUN curl -o miniconda.sh -L https://repo.anaconda.com/miniconda/Miniconda3-py37_4.10.3-Linux-x86_64.sh
RUN bash miniconda.sh -b -p /opt/miniconda3
RUN whoami
# RUN echo 'source miniconda3/bin/activate > ~/.bashrc'
# bash miniconda.sh -b -p
# source miniconda3/bin/activate
