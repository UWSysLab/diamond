# This is a comment
FROM ubuntu:wily
MAINTAINER Irene Zhang <iyzhang@cs.washington.edu>

ARG user=iyzhang

RUN apt-get update

RUN apt-get install -y python2.7 rsync ssh libevent-dev libprotobuf-dev libboost-program-options-dev python-pip
RUN apt-get install -y openjdk-8-jre-headless
RUN apt-get install -y libhiredis-dev libev-dev

RUN pip install redis

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
COPY ./run_docc.py /home/$user/
COPY ./run_retwis.py /home/$user/
COPY ./run_baseline.py /home/$user/
COPY ./run_baseline_retwis.py /home/$user/
COPY ./run_caching.py /home/$user/
COPY ./run_game.py /home/$user/
COPY ./run_redis_game.py /home/$user/
COPY ./experiment_common.py /home/$user/

COPY ./keys.txt /home/$user/
COPY ./gcelocalfiveshards.frontend0.config /home/$user/
COPY ./gcelocalfiveshards.frontend1.config /home/$user/
COPY ./gcelocalfiveshards.frontend2.config /home/$user/
COPY ./gcelocalfiveshards.frontend3.config /home/$user/
COPY ./gcelocalfiveshards.frontend4.config /home/$user/
COPY ./gcelocalfiveshards.frontend5.config /home/$user/
COPY ./gcelocalfiveshards.frontend6.config /home/$user/
COPY ./gcelocalfiveshards.frontend7.config /home/$user/
COPY ./gcelocalfiveshards.frontend8.config /home/$user/
COPY ./gcelocalfiveshards.frontend9.config /home/$user/
COPY ./gcelocalfiveshards.frontend10.config /home/$user/
COPY ./gcelocalfiveshards.frontend11.config /home/$user/
COPY ./gcelocalfiveshards.frontend12.config /home/$user/
COPY ./gcelocalfiveshards.frontend13.config /home/$user/
COPY ./gcelocalfiveshards.frontend14.config /home/$user/
COPY ./gcelocalfiveshards.frontend15.config /home/$user/
COPY ./gcelocaloneshard.frontend0.config /home/$user/
COPY ./gcelocaloneshard.frontend1.config /home/$user/
COPY ./gcelocaloneshard.frontend2.config /home/$user/
COPY ./gcelocaloneshard.frontend3.config /home/$user/
COPY ./gcelocaloneshard.frontend4.config /home/$user/
COPY ./gcelocaloneshard.frontend5.config /home/$user/
COPY ./gcelocaloneshard.frontend6.config /home/$user/
COPY ./gcelocaloneshard.frontend7.config /home/$user/
COPY ./gcelocaloneshard.frontend8.config /home/$user/
COPY ./gcelocaloneshard.frontend9.config /home/$user/
COPY ./gcelocaloneshard.frontend10.config /home/$user/
COPY ./gcelocaloneshard.frontend11.config /home/$user/
COPY ./gcelocaloneshard.frontend12.config /home/$user/
COPY ./gcelocaloneshard.frontend13.config /home/$user/
COPY ./gcelocaloneshard.frontend14.config /home/$user/
COPY ./gcelocaloneshard.frontend15.config /home/$user/

RUN chown -R $user /home/$user

USER $user

ENTRYPOINT ["python2.7"]
