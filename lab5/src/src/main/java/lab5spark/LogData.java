package lab5spark;

public class LogData {
    private String request;
    private String method;
    private String returnCode;
    private String date;

    public LogData(String request, String method, String returnCode, String date) {
        this.request = request;
        this.method = method;
        this.returnCode = returnCode;
        this.date = date;
    }

    public String getRequest() {
        return request;
    }

    public void setRequest(String request) {
        this.request = request;
    }

    public String getMethod() {
        return method;
    }

    public void setMethod(String method) {
        this.method = method;
    }

    public String getReturnCode() {
        return returnCode;
    }

    public void setReturnCode(String returnCode) {
        this.returnCode = returnCode;
    }

    public String getDate() {
        return date;
    }

    public void setDate(String date) {
        this.date = date;
    }

    public Integer getIntResponseCode() {
        try {
            return Integer.valueOf(returnCode);
        } catch (NumberFormatException numberFormatException) {
            return null;
        }
    }
}
