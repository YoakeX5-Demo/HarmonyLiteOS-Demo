package com.wciot.shenghuo;

import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.net.InetSocketAddress;
import java.net.Socket;

public class BasicSocket {
    public Socket socket = null; //客户端
    public BufferedWriter out = null; //阻塞模式发送
    Thread thread = null; //内置监听线程

    //创建客户端
    public void CreatStocketClient(String IPAddress,int Port) throws IOException {
        Stop();
        socket = new Socket();
        socket.connect(new InetSocketAddress(IPAddress,Port),5000);
        out = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
    }

    //监听接收回调
    public void ListenerInput(SocketCallback callback){
        thread = new Thread(() -> {
            boolean stop = false;
            while (true){
                try {
                    Thread.sleep(100);
                    if(socket!=null){
                        byte[] buffer = new byte[1024];
                        InputStream im = socket.getInputStream();
                        int len = im.read(buffer);
                        if(len < 0){
                            stop = true;
                            break;
                        }
                        String str = new String(buffer,0,len);
                        if(str != null){
                            callback.GetInput(str);
                        }
                    }
                }catch (IOException | InterruptedException e){
                    e.printStackTrace();
                }
            }
            if(stop){
                callback.GetInput("STOP");
            }
        });
        thread.start();
    }

    //发送
    public void SendMessage(String msg){
        try {
            out.write(msg + "\n");
            out.flush();
        }catch (IOException  e) {
            e.printStackTrace();
        }
    }

    //停止
    public void Stop(){
        try {
            if(thread != null){
                thread.interrupt();
                thread = null;
            }
            if(out != null){
                out.close();
                out = null;
            }
            if(socket != null){
                socket.close();
                socket = null;
            }
        }catch (IOException e){
            e.printStackTrace();
        }
    }
}
