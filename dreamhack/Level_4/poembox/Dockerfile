FROM ubuntu:24.04@sha256:d22e4fb389065efa4a61bb36416768698ef6d955fe8a7e0cdb3cd6de80fa7eec

RUN apt update && apt-get install -y socat && apt-get install -y adduser
RUN adduser --disabled-login --gecos '' -- 'poem'

WORKDIR /home/poem
USER poem

COPY ./deploy/flag ./flag
COPY ./deploy/prob ./prob

USER root
RUN chmod 444 ./flag
RUN chmod 755 ./prob

USER poem
EXPOSE 9090
CMD socat TCP-LISTEN:9090,reuseaddr,fork EXEC:./prob,stderr
