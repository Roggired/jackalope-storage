# Taken from https://github.com/shuhaoliu/docker-clion-dev/blob/master/Dockerfile

FROM ubuntu:20.04 as base-image

ARG DEBIAN_FRONTEND=noninteractive

########################################################
# Essential packages for remote debugging and login in
########################################################

RUN apt-get update && apt-get upgrade -y && apt-get install -y \
    apt-utils gcc g++ openssh-server cmake build-essential gdb gdbserver rsync vim

ADD . /code
WORKDIR /code

# Taken from - https://docs.docker.com/engine/examples/running_ssh_service/#environment-variables

RUN mkdir /var/run/sshd
RUN echo 'root:root' | chpasswd
RUN sed -i 's/PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

# SSH login fix. Otherwise user is kicked off after login
RUN sed 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' -i /etc/pam.d/sshd

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

# 22 for ssh server. 7777 for gdb server.
EXPOSE 22 7777

RUN useradd -ms /bin/bash debugger
RUN echo 'debugger:pwd' | chpasswd

########################################################
# Add custom packages and development environment here
########################################################

FROM base-image as image-with-deps

RUN apt update && \
	apt install -y make cmake python3-pip python3-dev software-properties-common

RUN pip install conan==1.54.0 && pip install numpy && \
	conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan && \
	conan config set general.revisions_enabled=1

########################################################

FROM image-with-deps
CMD ["/usr/sbin/sshd", "-D"]
