package com.wciot.shenghuo;

import ohos.utils.zson.ZSONObject;

public class SocketClient {
    private String IPAddress;
    private int port;

    public int getPort() {
        return port;
    }

    public String getIPAddress() {
        return IPAddress;
    }

    public void setIPAddress(String IPAddress) {
        this.IPAddress = IPAddress;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public static SocketClient JsonToThis(String dataStr){
        SocketClient socketClient = new SocketClient();
        try {
            socketClient = ZSONObject.stringToClass(dataStr, SocketClient.class);
        } catch (RuntimeException e) {
            System.out.print(e.getMessage());
            return null;
        }
        return socketClient;
    }
}
