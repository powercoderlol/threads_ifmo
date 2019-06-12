#!/bin/bash

sh start.sh

#./bin/spark-submit \
#  --class <main-class> \
#  --master <master-url> \
#  --deploy-mode <deploy-mode> \
#  --conf <key>=<value> \
#  ... # other options
#  <application-jar> \
#  [application-arguments]

spark-submit --master spark://master:7077 \
             --class lab5spark.App /build_jar/lab5spark-1.0.jar /nasa_logs