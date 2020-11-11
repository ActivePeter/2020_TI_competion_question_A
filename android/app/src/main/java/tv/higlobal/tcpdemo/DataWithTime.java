package tv.higlobal.tcpdemo;

import android.annotation.SuppressLint;

import java.sql.Date;
import java.text.SimpleDateFormat;

public class DataWithTime {
    DataWithTime(float value1,long time1){
        value=value1;
        time=time1;
    }
    public String getTime(){
        @SuppressLint("SimpleDateFormat") SimpleDateFormat formatter = new SimpleDateFormat("yyyy年-MM月dd日-HH时mm分ss秒");
        Date date = new Date(time);
        return formatter.format(date);
    }
    float value;
    long time;
}
