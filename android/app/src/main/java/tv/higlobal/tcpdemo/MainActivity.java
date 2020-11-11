package tv.higlobal.tcpdemo;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import java.io.BufferedWriter;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

//class SocketConnectThread extends Thread{
//    public void run(){
//        try {
//            //指定ip地址和端口号
//            mSocket = new Socket(mIpAddress,mClientPort);
//            if(mSocket != null){
//                //获取输出流、输入流
//                mOutStream = mSocket.getOutputStream();
//                mInStream = mSocket.getInputStream();
//            }
//        } catch (Exception e) {
//            e.printStackTrace();
//            mHandler.sendEmptyMessage(MSG_SOCKET_CONNECTFAIL);
//            return;
//        }
//        Log.i(“ni”,"connect success");
//        mHandler.sendEmptyMessage(MSG_SOCKET_CONNECT);
//    }
//
//}

public class MainActivity extends AppCompatActivity {
    public class AttData {
        AttData(double x1, double y1, double z1, int step) {
            x = x1;
            y = y1;
            z = z1;
            stepcnt = step;
        }

        double x;
        double y;
        double z;
        int stepcnt;
    }

    int heartBuffStep = 0;
    float heartBuff[] = new float[60];

    void pushDataToHeartBuff(float a) {
        heartBuff[heartBuffStep] = a;
        heartBuffStep++;
        heartBuffStep %= 60;
    }

    float getHeartBeatRateAver() {
        float sum = 0;
        for (int i = 0; i < 60; i++) {
            sum += heartBuff[i];
        }
        return sum / 60;
    }

    static void printInfo(SocketTransceiver st, String msg) {
        System.out.println("server123 " + st.getInetAddress().getHostAddress());
        System.out.println("server123 " + msg);
    }

    static int isFlag(byte a) {
        if (a == 'x') {
            return 8;
        } else if (a == 't') {
            return 10;
        } else if (a == 'z') {
            return 2;
        } else if (a == 'y') {
            return 2;
        } else {
            return -1;
        }
    }

    static int byteToPositiveShort(byte a) {
        if (a < 0) {
            return (short) (256 + a);
        } else {
            return a;
        }
    }

    static int getIntFromTwoByte(byte a, byte b) {
        return (short) ((byteToPositiveShort(a)) << 8 | byteToPositiveShort((b)));
    }

    static int getIntFromFourByte(byte a, byte b, byte c, byte d) {
        return (short) (
                (byteToPositiveShort(a)) << 24 |
                        (byteToPositiveShort(b)) << 16 |
                        (byteToPositiveShort(c)) << 8 |
                        byteToPositiveShort((d))
        );
    }

    final int msgUpdateGUI = 1;
    final int msgUpdateAttitude = 2;
    final int msgUpdateTemp = 3;

    final int msgUpdateTempCheck = 4;
    final int msgUpdateHeartCheck = 5;
    final String TAG = "server123";
    BufferedWriter writer = null;
    WaveView waveView;

    boolean isConnected = false;
    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler() {
        @SuppressLint({"SetTextI18n", "DefaultLocale"})
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == msgUpdateGUI) {
                waveView.invalidate();
            } else if (msg.what == msgUpdateAttitude) {
                AttData data = (AttData) msg.obj;
                int stepcnt = data.stepcnt;
                Tv_attitude.setText(String.format("姿态传感器原始数据 %6.2f %6.2f %6.2f", data.x, data.y, data.z));
                Tv_stepcnt.setText(String.format("步数 %d 步 距离 %6.3f m", stepcnt, stepcnt * 0.5f));
//                waveView.invalidate();
            } else if (msg.what == msgUpdateTemp) {
                Tv_momentFreq.setText(String.format("实时心率 %6.2f bpm", momentFreq));
                Tv_temp.setText(String.format("实时体温 %6.3f 摄氏度\n最近一次测试体温 %6.2f摄氏度", msg.arg1 / 100.0, testTemp));
                Tv_heartInterval.setText(String.format("最近一次心跳间隔 %d ms", deltaMillis));
                Tv_heartAvr.setText(String.format("最近一次测量十秒平均心率 %6.2f bpm", avrFreq));
            } else if (msg.what == msgUpdateHeartCheck) {
                heartlist.add(new DataWithTime(avrFreq, System.currentTimeMillis()));
                String data = gson.toJson(heartlist);
                SharedPreferences History = getSharedPreferences("History", 0);
                SharedPreferences.Editor editor = History.edit();
                editor.putString("heartlist", data);
                editor.apply();
            } else if (msg.what == msgUpdateTempCheck) {
                templist.add(new DataWithTime(testTemp, System.currentTimeMillis()));
                String data = gson.toJson(templist);
                SharedPreferences History = getSharedPreferences("History", 0);
                SharedPreferences.Editor editor = History.edit();
                editor.putString("templist", data);
                editor.apply();
            }
        }
    };
    TextView Tv_attitude;

    TextView Tv_temp;
    TextView Tv_stepcnt;


    List<DataWithTime> templist = new ArrayList<>();
    List<DataWithTime> heartlist = new ArrayList<>();

    Gson gson = new Gson();

    TextView Tv_momentFreq;
    TextView Tv_heartInterval;
    TextView Tv_heartAvr;
    float momentFreq = 0;
    int deltaMillis = 0;
    float avrFreq = 0;
    float testTemp = 0;
    private RadioGroup radiogroup;
    Timer timer = new Timer();   //定义全局变量
    TimerTask task = new TimerTask() {
        @Override
        public void run() {
            Message message = handler.obtainMessage();
            ;
            message.what = msgUpdateGUI;
            handler.sendMessage(message);
        }
    };
    Button checktempBtn;
    Button checkHeartBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SharedPreferences History = getSharedPreferences("History", 0);
        Gson gson = new Gson();
        Type listType = new TypeToken<List<DataWithTime>>() {
        }.getType();
        templist = gson.fromJson(History.getString("templist", ""), listType);
        heartlist=gson.fromJson(History.getString("heartlist", ""), listType);


        waveView = findViewById(R.id.waveview);
        Tv_attitude = findViewById(R.id.attitude);
        Tv_stepcnt = findViewById(R.id.stepcnt);
        Tv_temp = findViewById(R.id.textView5);
        radiogroup = findViewById(R.id.radiogroup);
        Tv_momentFreq = findViewById(R.id.textView2);
        Tv_heartInterval = findViewById(R.id.textView3);
        Tv_heartAvr = findViewById(R.id.textView4);
        timer.scheduleAtFixedRate(task, 500, 50);
        checktempBtn = findViewById(R.id.button2);
        checkHeartBtn = findViewById(R.id.button);
        checktempBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, ListActivity.class);
                intent.putExtra("name", "temp");
                startActivity(intent);
            }
        });
        checkHeartBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(MainActivity.this, ListActivity.class);
                intent.putExtra("name", "heart");
                startActivity(intent);
            }
        });
        radiogroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkId) {
                switch (checkId) {
                    case R.id.radioTemp:
                        Toast.makeText(MainActivity.this, "已切换到温度曲线", Toast.LENGTH_SHORT).show();
                        waveView.setSelectedWave(WaveView.WaveKind.Temperature);
                        break;
                    case R.id.radioHeart:
                        Toast.makeText(MainActivity.this, "已切换到心跳曲线", Toast.LENGTH_SHORT).show();
                        waveView.setSelectedWave(WaveView.WaveKind.Heart);
                        break;
                    case R.id.radioAttitude:
                        Toast.makeText(MainActivity.this, "已切换到位姿曲线", Toast.LENGTH_SHORT).show();
                        waveView.setSelectedWave(WaveView.WaveKind.Attitude);
                        break;
                }
            }
        });
        radiogroup.check(R.id.radioTemp);
        TcpServer server = new TcpServer(1234) {

            @Override
            public void onConnect(SocketTransceiver client) {
                printInfo(client, "Connect");
            }

            @Override
            public void onConnectFailed() {
                System.out.println("Client Connect Failed");
            }

            AttData attdata;
            int cnt=0;
            byte attDataStepH=0;
            byte attDataStepL=0;
            @Override
            public void onReceive(SocketTransceiver client, byte s[], int len) {
                Log.e(TAG, "len:" + len);
                byte flag;
                int flagLen = 0;
                int i;
                for (i = 0; i < len; i++) {
                    flagLen = isFlag(s[i]);
                    if (flagLen > 0) {
                        break;
                    }
                }
                flag = s[i];
//                Log.e(TAG,"flaglen:"+flagLen);
                if (flagLen >= len - i) {
                    return;
                }
                if (flag == 'x') {
//                    Log.e(TAG,"data 1:"+getIntFromTwoByte((byte)-1,(byte)-1));
//                    s[i+1]=1;
//                    s[i+2]=1;
                    int x = getIntFromTwoByte(s[i + 1], s[i + 2]);//(byteToPositiveShort(s[i+1]))<<8|byteToPositiveShort((s[i+2]));
                    int y = getIntFromTwoByte(s[i + 3], s[i + 4]);//(byteToPositiveShort(s[i+3]))<<8|byteToPositiveShort((s[i+4]));
                    int z = getIntFromTwoByte(s[i + 5], s[i + 6]);//(byteToPositiveShort(s[i+5]))<<8|byteToPositiveShort((s[i+6]));
                    int count = getIntFromTwoByte(s[i + 7], s[i + 8]);//(byteToPositiveShort(s[i+7]))<<8|byteToPositiveShort((s[i+8]));
                    Log.e(TAG, "data x:" + x / 16.0 + " y:" + y / 16.0 + " z:" + z / 16.0 + " cnt:" + count);
                    attDataStepH=s[i + 7];
                    attDataStepL=s[i + 8];
                    Message msg = handler.obtainMessage();
                    msg.what = msgUpdateAttitude;
                    if (attdata == null) {
                        attdata = new AttData(x / 16.0, y / 16.0, z / 16.0, count);

                    } else {
                        attdata.x = x / 16.0;
                        attdata.y = y / 16.0;
                        attdata.z = z / 16.0;
                        attdata.stepcnt = count;
                    }
                    WaveView.DataBuff a = waveView.DataBuff_Attitude;
                    a.push(z / 16f);
//                    waveView.invalidate();
                    msg.obj = attdata;
                    handler.sendMessage(msg);
                } else if (flag == 't') {
                    cnt++;
                    if(cnt==10){
                        cnt=0;

                    }
                    byte buf[]=new byte[3];
                    buf[0]='t';
                    buf[1]=attDataStepH;
                    buf[2]=attDataStepL;

                    if(client!=null){
                        client.send(buf);
                    }

                    int x = getIntFromTwoByte(s[i + 1], s[i + 2]);//(byteToPositiveShort(s[i+1]))<<8|byteToPositiveShort((s[i+2]));
                    int y = getIntFromFourByte(s[i + 3], s[i + 4], s[i + 5], s[i + 6]);
                    momentFreq = getIntFromFourByte((byte) 0, (byte) 0, s[i + 7], s[i + 8]) / 100.0f;
                    pushDataToHeartBuff(momentFreq);
                    deltaMillis = getIntFromFourByte((byte) 0, (byte) 0, s[i + 9], s[i + 10]);
                    waveView.DataBuff_Heart.push(y);
                    waveView.DataBuff_Temp.push(x / 100.0f);
                    Message msg = handler.obtainMessage();
                    msg.what = msgUpdateTemp;
                    msg.arg1 = x;
                    handler.sendMessage(msg);
                    Log.e(TAG, "data x:" + x / 100.0 + " data heart:" + y);
                } else if (flag == 'z') {
                    int x = getIntFromTwoByte(s[i + 1], s[i + 2]);//(byteToPositiveShort(s[i+1]))<<8|byteToPositiveShort((s[i+2]));
                    avrFreq = x / 100f;

                    Message msg = handler.obtainMessage();
                    msg.what = msgUpdateHeartCheck;
                    handler.sendMessage(msg);

//                    templist.add(new DataWithTime(avrFreq,System.currentTimeMillis()));
//                    String data = gson.toJson(templist);
//                    SharedPreferences settings = getSharedPreferences("Setting", 0);
//                    SharedPreferences.Editor editor = settings.edit();
//                    sp.putString("listStr", data);

//                    Log.e(TAG, "data x:" + x / 100.0 + " data heart:" + y);
                } else if (flag == 'y') {
                    int x = getIntFromTwoByte(s[i + 1], s[i + 2]);//(byteToPositiveShort(s[i+1]))<<8|byteToPositiveShort((s[i+2]));
                    testTemp = x / 100f;

                    Message msg = handler.obtainMessage();
                    msg.what = msgUpdateTempCheck;
                    handler.sendMessage(msg);
//                    Log.e(TAG, "data x:" + x / 100.0 + " data heart:" + y);
                }
//                Log.e(TAG,"len:"+len);
//                printInfo(client, "Send Data: " + s);
//                client.send(s);
            }

            @Override
            public void onDisconnect(SocketTransceiver client) {
                printInfo(client, "Disconnect");
            }

            @Override
            public void onServerStop() {
                System.out.println("--------Server Stopped--------");
            }
        };
        System.out.println("--------Server Started--------");
        server.start();
        isConnected = true;
        /* 更新UI */
//        btn_connect.setText("断开");
//        Toast.makeText(MainActivity.this, "连接成功：）", Toast.LENGTH_SHORT).show();
    }
}
