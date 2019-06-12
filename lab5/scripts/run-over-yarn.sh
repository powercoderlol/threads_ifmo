#!/bin/bash

sh start.sh

start-yarn.sh

spark-submit --master yarn \
             --deploy-mode client \
             --class com.botsula.Application \
             /build_jar/lab5-1.0.jar /nasa_logs