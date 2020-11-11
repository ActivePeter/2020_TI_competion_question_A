package tv.higlobal.tcpdemo;

import android.util.Log;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;

/**
 * Socket收发器 通过Socket发送数据，并使用新线程监听Socket接收到的数据
 *
 * @author jzj1993
 * @since 2015-2-22
 */
public abstract class SocketTransceiver implements Runnable {

    protected Socket socket;
    protected InetAddress addr;
    protected DataInputStream in;
    protected DataOutputStream out;
    public byte UnhandledData[];
    private boolean runFlag;

    /**
     * 实例化
     *
     * @param socket
     *            已经建立连接的socket
     */
    public SocketTransceiver(Socket socket) {
        this.socket = socket;
        this.addr = socket.getInetAddress();
    }

    /**
     * 获取连接到的Socket地址
     *
     * @return InetAddress对象
     */
    public InetAddress getInetAddress() {
        return addr;
    }

    /**
     * 开启Socket收发
     * <p>
     * 如果开启失败，会断开连接并回调{@code onDisconnect()}
     */
    public void start() {
        runFlag = true;
        new Thread(this).start();
    }

    /**
     * 断开连接(主动)
     * <p>
     * 连接断开后，会回调{@code onDisconnect()}
     */
    public void stop() {
//        Log.e("ip123","stop");

//        runFlag = false;
        try {
            socket.shutdownInput();
            socket=null;
            if(in!=null){
                in.close();
            }
            if(out!=null){
                out.close();
                out=null;
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 发送字符串
     *
     * @param s
     *            字符串
     * @return 发送成功返回true
     */
    OutputStream out1;
    public boolean send(byte a[]) {
        Log.e("hhhhh123","abcef");
        if(socket.isClosed()||!runFlag){
            return false;
        }
        if(out==null){
            try {
                out = new DataOutputStream(socket.getOutputStream());
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        if (out != null) {
            Log.e("hhhhh123","abc");
            try {
                out.write(a);
                out.flush();
                return true;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    /**
     * 监听Socket接收的数据(新线程中运行)
     */
    @Override
    public void run() {
//        try {
//            in = new DataInputStream(this.socket.getInputStream());
//            out = new DataOutputStream(this.socket.getOutputStream());
//        } catch (IOException e) {
//            e.printStackTrace();
//            runFlag = false;
//        }
        InputStream inputStream;
        byte[] buf = new byte[1024];
        try {
            inputStream=this.socket.getInputStream();
            int len;
            while (runFlag) {
                if(inputStream==null){
                    break;
                }
                if(inputStream.available()>0){
                    if((len=inputStream.read(buf))==-1){
                        break;
                    }
                    onReceive(addr,buf,len);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
            runFlag = false;
        }

        // 断开连接
        try {
            in.close();
            out.close();
            socket.close();
            in = null;
            out = null;
            socket = null;
        } catch (IOException e) {
            e.printStackTrace();
        }
        this.onDisconnect(addr);
    }

    /**
     * 接收到数据
     * <p>
     * 注意：此回调是在新线程中执行的
     *
     * @param addr
     *            连接到的Socket地址
     * @param s
     *            收到的字符串
     */
    public abstract void onReceive(InetAddress addr, byte s[],int len);

    /**
     * 连接断开
     * <p>
     * 注意：此回调是在新线程中执行的
     *
     * @param addr
     *            连接到的Socket地址
     */
    public abstract void onDisconnect(InetAddress addr);
}