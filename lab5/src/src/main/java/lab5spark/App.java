package lab5spark;

import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;
import scala.Tuple2;
import scala.Tuple3;

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.util.Locale;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static org.apache.spark.sql.functions.*;

public class App {

    static final String application_name = "Kennedy Space Center Website Log Parser";
    static final String ERROR_MESSAGE = "Incorrect file location";

    static final String hdfs_connect = "hdfs://master:9000/";

    static final String in_date_format = "dd/MMM/yyyy:HH:mm:ss Z";
    static final String out_date_format = "dd/MMM/yyyy";

    static final String regex_pattern =
            "^(\\S+) (\\S+) (\\S+) \\[([\\w:/]+\\s[+\\-]\\d{4})] \"(\\S+) (\\S+) (\\S+)\" (\\d{3}) (\\d+)";

    static final String output_folder_task_1 = "output1";
    static final String output_folder_task_1_v2 = "output11";
    static final String output_folder_task_2 = "output2";
    static final String output_folder_task_2_v2 = "output22";
    static final String output_folder_task_3 = "output3";

    private static lab5spark.LogData log_entry_extractor(String str) {
        Pattern logPattern = Pattern.compile(regex_pattern);
        Matcher matcher = logPattern.matcher(str);
        return matcher.find() ?
                new LogData(matcher.group(6),
                            matcher.group(5),
                            matcher.group(8),
                            formatDate(matcher.group(4)))
                            : null;
    }

    private static String formatDate(String datetime) {
        DateTimeFormatter inputFormat =
                DateTimeFormatter.ofPattern(
                        in_date_format, Locale.US);
        DateTimeFormatter outputFormat =
                DateTimeFormatter.ofPattern(
                        out_date_format, Locale.US);

        return LocalDate.parse(datetime, inputFormat).format(outputFormat);
    }

    public static void main(String[] args) {
        if (args.length == 0) {
            throw new IllegalArgumentException(ERROR_MESSAGE);
        }

        // Init session
        SparkSession session = SparkSession.builder().master("local").appName(application_name).getOrCreate();
        JavaSparkContext jsc = new JavaSparkContext(session.sparkContext());
        JavaRDD<LogData> input = jsc.textFile(hdfs_connect + args[0])
                .map(App::log_entry_extractor)
                .filter(Objects::nonNull);

        Dataset<Row> dataSet = session.createDataFrame(input, LogData.class);

        long start_time;
        long end_time;
        Long duration_v1, duration_v2;

        start_time = System.currentTimeMillis();
        // task 1
        dataSet.filter(col("returnCode").between(500, 599))
                .groupBy("request")
                .count()
                .select("request", "count")
//                .sort("count")
                .coalesce(1)
                .toJavaRDD()
                .saveAsTextFile(hdfs_connect + output_folder_task_1);
        end_time = System.currentTimeMillis();
        duration_v1 = end_time - start_time;

        start_time = System.currentTimeMillis();
        taskOne_v2(input, hdfs_connect + output_folder_task_1);
        end_time = System.currentTimeMillis();
        System.out.println("Execution time: " + String.valueOf(end_time - start_time));
        duration_v2 = end_time - start_time;

        try{
            BufferedWriter fw = new BufferedWriter(new FileWriter("duration.txt"));
            fw.write(duration_v1.toString() + " " + duration_v2.toString());
            fw.close();
        }catch(Exception e){System.out.println(e);}

        //taskTwo_v2(input, hdfs_connect + output_folder_task_2);

        // task 2 v_1
        dataSet.groupBy("method", "returnCode", "date")
                .count()
                .filter(col("count").geq(10))
                .select("date", "method", "returnCode", "count")
//                .sort("date")
                .coalesce(1)
                .toJavaRDD()
                .saveAsTextFile(hdfs_connect + output_folder_task_2);

        // task 3
        dataSet.filter(col("returnCode").between(400, 599))
                .groupBy(window(to_date(col("date"), out_date_format), "1 week", "1 day"))
                .count()
                .select(date_format(col("window.start"), out_date_format),
                        date_format(col("window.end"), out_date_format),
                        col("count"))
//                .sort("window.start")
                .coalesce(1)
                .toJavaRDD()
                .saveAsTextFile(hdfs_connect + output_folder_task_3);
    }

    private static void taskOne_v2(JavaRDD<LogData> data, String outputDir) {
        data.filter(logRow -> logRow.getIntReturnCode() >= 500 && logRow.getIntReturnCode() <= 599)
                .mapToPair(logRow -> new Tuple2<>(logRow.getRequest(), 1))
                .reduceByKey((a, b) -> a + b)
                .coalesce(1)
                .saveAsTextFile(outputDir);
    }

    private static void taskTwo_v2(JavaRDD<LogData> data, String outputDir) {
        data.mapToPair(logRow -> new Tuple2<>(new Tuple3<>(logRow.getDate(), logRow.getMethod(), logRow.getReturnCode()), 1))
                .reduceByKey((a, b) -> a + b)
                .filter(f -> f._2() >= 10)
                .coalesce(1)
                .saveAsTextFile(outputDir);
    }
}
