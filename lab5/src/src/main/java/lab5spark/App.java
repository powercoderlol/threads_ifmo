package lab5spark;

import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;

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
    static final String output_folder_task_2 = "output2";
    static final String output_folder_task_3 = "output3";

    private static LogData log_entry_extractor(String str) {
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

        // task 1
        dataSet.filter(col("returnCode").between(500, 599))
                .groupBy("request")
                .count()
                .select("request", "count")
                .sort(desc("count"))
                .coalesce(1)
                .toJavaRDD()
                .saveAsTextFile(hdfs_connect + output_folder_task_1);

        // task 2
        dataSet.groupBy("method", "returnCode", "date")
                .count()
                .filter(col("count").geq(10))
                .select("date", "method", "returnCode", "count")
                .sort("date")
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
                .sort("window.start")
                .coalesce(1)
                .toJavaRDD()
                .saveAsTextFile(hdfs_connect + output_folder_task_3);
    }
}
