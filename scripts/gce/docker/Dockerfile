# This is a comment
FROM ubuntu:wily
MAINTAINER Irene Zhang <iyzhang@cs.washington.edu>

RUN apt-get update

RUN apt-get install -y python2.7 rsync ssh libevent-dev libprotobuf-dev

RUN echo "StrictHostKeyChecking=no" >> /etc/ssh/ssh_config

RUN useradd -ms /bin/bash iyzhang

ENV PATH /usr/bin:/bin:$PATH

ENV LD_LIBRARY_PATH /home/iyzhang
ENV HOME /home/iyzhang

COPY ./id_rsa /home/iyzhang/.ssh/
COPY ./id_rsa.pub /home/iyzhang/.ssh/
COPY ./run_terminalclient.py /home/iyzhang/
COPY ./run_benchclient.py /home/iyzhang/

RUN chown -R iyzhang /home/iyzhang

USER iyzhang

ENTRYPOINT ["python2.7"]