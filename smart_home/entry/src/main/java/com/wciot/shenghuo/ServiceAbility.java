package com.wciot.shenghuo;

import ohos.aafwk.ability.Ability;
import ohos.aafwk.content.Intent;
import ohos.rpc.*;
import ohos.hiviewdfx.HiLog;
import ohos.hiviewdfx.HiLogLabel;
import ohos.utils.zson.ZSONObject;

import java.io.IOException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class ServiceAbility extends Ability {
    private static final HiLogLabel LABEL_LOG = new HiLogLabel(3, 0xD001100, "Demo");
    MyRemote myRemote = new MyRemote("MyService_MyRemote_shenghuo");

    @Override
    public void onStart(Intent intent) {
        HiLog.error(LABEL_LOG, "ServiceAbility::onStart");
        super.onStart(intent);
    }

    @Override
    public void onBackground() {
        super.onBackground();
        HiLog.info(LABEL_LOG, "ServiceAbility::onBackground");
    }

    @Override
    public void onStop() {
        super.onStop();
        HiLog.info(LABEL_LOG, "ServiceAbility::onStop");
    }

    @Override
    public void onCommand(Intent intent, boolean restart, int startId) {
    }

    @Override
    public IRemoteObject onConnect(Intent intent) {
        super.onConnect(intent);
        return myRemote.asObject();
    }

    @Override
    public void onDisconnect(Intent intent) {
    }

    class MyRemote extends RemoteObject implements IRemoteBroker {

        private static final int SUCCESS = 0;
        private static final int ERROR = 1;
        private static final int CREAT_CLIENT = 1003; //开启客户端 需要输入IP和端口号
        private static final int SEND_CLIENT = 1004; //客户端发送消息
        private static final int SUBSCRIBE_CLIENT = 1005; //开启订阅客户端消息
        private static final int UNSUBSCRIBE_CLIENT = 1006; //关闭订阅客户端消息并关闭连接
        public Set<IRemoteObject> remoteObjectHandlers = new HashSet<IRemoteObject>();
        BasicSocket basicSocket = null;
        public MyRemote(String descriptor) {
            super(descriptor);
        }

        @Override
        public IRemoteObject asObject() {
            return this;
        }

        @Override
        public boolean onRemoteRequest(int code, MessageParcel data, MessageParcel reply, MessageOption option) throws RemoteException {
            switch (code){
                case CREAT_CLIENT:
                    String dataStr = data.readString();
                    SocketClient socketClient = SocketClient.JsonToThis(dataStr);
                    basicSocket = new BasicSocket();
                    try {
                        basicSocket.CreatStocketClient(socketClient.getIPAddress(),socketClient.getPort());
                        reply.writeString(getReply(SUCCESS,socketClient));
                    } catch (IOException e) {
                        e.printStackTrace();
                        reply.writeString(getReply(ERROR,socketClient));
                    }
                    break;
                case SEND_CLIENT:
                    String str = data.readString();
                    basicSocket.SendMessage(str);
                    reply.writeString(getReply(SUCCESS,null));
                    break;
                case SUBSCRIBE_CLIENT:
                    remoteObjectHandlers.add(data.readRemoteObject());
                    startNotify();
                    reply.writeString(getReply(SUCCESS,remoteObjectHandlers.size()));
                    break;
                case UNSUBSCRIBE_CLIENT:
                    remoteObjectHandlers.clear();
                    basicSocket.Stop();
                    reply.writeString(getReply(SUCCESS,remoteObjectHandlers.size()));
                    break;
                default:
                    reply.writeString(getReply(ERROR,null));
                    return false;
            }
            return true;
        }

        //固定返回格式
        private String getReply(int code,Object obj){
            Map<String, Object> result = null;
            if(code == SUCCESS){
                result = new HashMap<String, Object>();
                result.put("code", SUCCESS);
                result.put("result", "成功");
                result.put("data",obj);
            }
            else {
                result = new HashMap<String, Object>();
                result.put("code", ERROR);
                result.put("result", "失败");
                result.put("data",new Object());
            }
            return ZSONObject.toZSONString(result);
        }

        //开启订阅
        public void startNotify() {
            basicSocket.ListenerInput(new SocketCallback() {
                @Override
                void GetInput(String str) {
                    MessageParcel data = MessageParcel.obtain();
                    MessageParcel reply = MessageParcel.obtain();
                    MessageOption option = new MessageOption();
                    Map<String, Object> event = new HashMap<String, Object>();
                    event.put("clientOut", str);
                    data.writeString(ZSONObject.toZSONString(event));
                    // 如果仅支持单FA订阅，可直接触发回调：remoteObjectHandler.sendRequest(100, data, reply, option);
                    for (IRemoteObject item : remoteObjectHandlers) {
                        try {
                            item.sendRequest(100, data, reply, option);
                        } catch (RemoteException e) {
                            e.printStackTrace();
                        }
                    }
                    reply.reclaim();
                    data.reclaim();
                }
            });
        }
    }
}