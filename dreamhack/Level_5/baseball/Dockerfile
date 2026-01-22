FROM ubuntu@sha256:0e5e4a57c2499249aafc3b40fcd541e9a456aab7296681a3994d631587203f97

RUN apt-get update -y && apt-get upgrade -y 
RUN apt-get install socat -y
RUN rm -rf /var/lib/apt/lists/*

ENV TERM=linux

RUN useradd -m prob -s /bin/bash
RUN chown -R prob:prob /home/prob

ADD /deploy/prob /home/prob
ADD /deploy/flag /home/prob
ADD /deploy/libc.so.6 /home/prob

WORKDIR /home/prob

RUN chmod 755 /home/prob/prob \
    && chown prob:prob /home/prob/prob

USER prob
CMD socat TCP-L:10005,reuseaddr,fork, EXEC:"./prob"
