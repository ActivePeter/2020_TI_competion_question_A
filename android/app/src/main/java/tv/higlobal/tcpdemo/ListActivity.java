package tv.higlobal.tcpdemo;

import android.content.Intent;
import android.content.SharedPreferences;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.List;

public class ListActivity extends AppCompatActivity {

    List<DataWithTime> templist;
    ListView listView;
    private String[] data;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list);
        Intent intent = getIntent();
        String type = intent.getStringExtra("name");

        SharedPreferences History = getSharedPreferences("History", 0);

        String jsondata;
        if(!type.equals("temp")){
            Log.e("server123hh","heartlist");
            jsondata= History.getString("heartlist", "");
        }else{
            Log.e("server123hh","templist");
            jsondata=History.getString("templist", "");
        }

        Gson gson = new Gson();
        Type listType = new TypeToken<List<DataWithTime>>() {
        }.getType();
        templist = gson.fromJson(jsondata, listType);

        data=new String[templist.size()];
        String danwei;
        if(type.equals("temp")){
            danwei="摄氏度\n";
        }else{
            danwei="次/分钟\n";
        }

//        heartlist=gson.fromJson(jsondata, listType);
        for (int i = 0; i < templist.size(); i++) {
            Log.e("server123hh",templist.get(i).getTime()+ "-----------"+templist.get(i).value);
            if(!type.equals("temp")){
                if(templist.get(i).value>=60&&templist.get(i).value<=100){
                    danwei="次/分钟\n";
                    data[i]=templist.get(i).value+ danwei+"            "+templist.get(i).getTime();
                }else{
                    danwei="次/分钟 （心率不正常）\n";
                    data[i]=templist.get(i).value+ danwei+"            "+templist.get(i).getTime();
                }

            }else{
                if(templist.get(i).value>=25&&templist.get(i).value<=38){
                    danwei="摄氏度\n";
                    data[i]=templist.get(i).value+ danwei+"            "+templist.get(i).getTime();
                }else{
                    danwei="摄氏度 （体表温度不正常）\n";
                    data[i]=templist.get(i).value+ danwei+"            "+templist.get(i).getTime();
                }
//                data[i]=templist.get(i).value+ danwei+"            "+templist.get(i).getTime();
            }
        }

        listView=findViewById(R.id.list);
        ArrayAdapter<String> adapter=new ArrayAdapter<String>(ListActivity.this,android.R.layout.simple_list_item_1,data);
        listView.setAdapter(adapter);


    }
}
