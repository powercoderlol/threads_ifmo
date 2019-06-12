#!/bin/bash

stop-yarn.sh

hadoop fs -rm -r -f /output1 /output2 /output3

stop-dfs.sh

sed -i s/$HOSTNAME/master/ $HADOOP_CONF_DIR/core-site.xml