FROM ubuntu:22.04

MAINTAINER anonymous
RUN apt-get update

RUN useradd -m node_node_node


RUN apt-get install -y build-essential socat

WORKDIR /home/node_node_node

ADD ./flag /home/node_node_node
ADD ./node_node_node /home/node_node_node

ENTRYPOINT ["sh", "-c", "exec socat -s TCP-LISTEN:1337,reuseaddr,fork EXEC:/home/node_node_node/node_node_node,stderr"]

EXPOSE 1337