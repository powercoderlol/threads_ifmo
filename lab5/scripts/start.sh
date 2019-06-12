#!/bin/bash

chmod +x /build/lab5spark-1.0.jar

sed -i s/master/$HOSTNAME/ $HADOOP_CONF_DIR/core-site.xml

${HADOOP_HOME}/sbin/start-all.sh

hadoop fs -mkdir /nasa_logs
hadoop fs -put /usr/spark-2.4.1/NASA_access_log_Jul95 /nasa_logs