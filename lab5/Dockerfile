FROM gettyimages/spark:2.4.1-hadoop-3.0

# environment variables
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64

# Install required packages
RUN apt -y update
RUN apt -y upgrade
RUN apt -y install ssh wget openssh-server

# Setup SSH
RUN ssh-keygen -t rsa -f ~/.ssh/id_rsa -P '' && \
    cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
COPY ssh/* $HOME/.ssh/config

# Copy HADOOP conf
COPY conf/hadoop/* $HADOOP_CONF_DIR/
RUN chmod +x $HADOOP_CONF_DIR/hadoop-env.sh

# Copy configs from host-machine
COPY scripts/* /scripts/
RUN chmod -R +x /scripts

# Download Kennedy Space Center logs
RUN wget ftp://ita.ee.lbl.gov/traces/NASA_access_log_Jul95.gz && \
    gzip -d NASA_access_log_Jul95.gz

RUN mkdir -p /app/hadoop/tmp && \
    mkdir -p /usr/local/hadoop_store/hdfs/namenode && \
    mkdir -p /usr/local/hadoop_store/hdfs/datanode
RUN hdfs namenode -format

ENTRYPOINT [ "/bin/bash", "-c", "service ssh start; tail -f /dev/null"]