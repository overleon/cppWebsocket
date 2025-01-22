FROM ubuntu:latest

RUN apt-get update -y
RUN apt-get upgrade -y

RUN apt install -y git