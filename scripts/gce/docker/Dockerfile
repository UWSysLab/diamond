# This is a comment
FROM ubuntu:wily
MAINTAINER Irene Zhang <iyzhang@cs.washington.edu>

ARG user=iyzhang

RUN apt-get update

RUN apt-get install -y python2.7 rsync ssh libevent-dev libprotobuf-dev libboost-program-options-dev

RUN echo "StrictHostKeyChecking=no" >> /etc/ssh/ssh_config

RUN useradd -ms /bin/bash $user

ENV PATH /usr/bin:/bin:$PATH

ENV LD_LIBRARY_PATH /home/$user
ENV HOME /home/$user

COPY ./id_rsa /home/$user/.ssh/
COPY ./id_rsa.pub /home/$user/.ssh/
COPY ./run_terminalclient.py /home/$user/
COPY ./run_benchclient.py /home/$user/
COPY ./run_scalability.py /home/$user/

RUN chown -R $user /home/$user

USER $user

ENTRYPOINT ["python2.7"]
