FROM devkitpro/devkitarm:20190212
ADD libnds-gbar2.tar /opt/devkitpro
RUN \
  apt-get update && \
  apt-get install -y python && \
  rm -rf /var/lib/apt/lists/* 
WORKDIR /data
