import prompt from '@system.prompt'
const BundleName = 'com.wciot.shenghuo'
const AbilityName = 'com.wciot.shenghuo.ServiceAbility'
const ACTION_SYNC = 0
const ACTION_ASYNC = 1
const ABILITY_TYPE_EXTERNAL = 0
const ABILITY_TYPE_INTERNAL = 1
const CREAT_CLIENT = 1003
const SEND_CLIENT = 1004
const SUBSCRIBE_CLIENT = 1005
const UNSUBSCRIBE_CLIENT = 1006
export default {
    data: {
        title: '智能小屋',
        ipaddress:'',
        port:0,
        send_value:0,
        startflag: false,
        client_statu: false,
        subscribe_statu: false,
        property:[
            {
                "name":"光照强度",
                "propertyName":"Lux",
                "unit":"lux"
            },
            {
                "name":"环境湿度",
                "propertyName":"Humidity",
                "unit":"%"
            },
            {
                "name":"环境温度",
                "propertyName":"Temperature",
                "unit":"℃"
            }
        ],
        command:[
            {
                "name":"风扇",
                "paraName":"Motor",
                "type":"string"
            },
            {
                "name":"灯",
                "paraName":"Light",
                "type":"string"
            }
        ],
        properties:{
            "Lux":0,
            "Humidity":0,
            "Temperature":0,
            "Motor":"",
            "Light":""
        }
    },
    onInit() {

    },
    async creatclient(){
        let actionData={
            IPAddress:this.ipaddress,
            port:this.port
        }
        let action = {}
        action.bundleName = BundleName
        action.abilityName = AbilityName
        action.messageCode = CREAT_CLIENT
        action.data = actionData
        action.abilityType = ABILITY_TYPE_EXTERNAL
        action.syncOption = ACTION_SYNC
        prompt.showToast({
            message: '正在链接',
            duration: 5000,
        })
        let result = await FeatureAbility.callAbility(action)
        let ret = JSON.parse(result)
        if (ret.code == 0) {
            this.client_statu = true
            this.startflag = true
            prompt.showToast({
                message: '连接成功',
                duration: 3000,
            })
            this.subscribe()
        } else {
            this.client_statu = false
            this.startflag = false
            prompt.showToast({
                message: '连接失败',
                duration: 3000,
            })
        }
    },
    async subscribe(){
        if(this.client_statu) {
            let action = {}
            action.bundleName = BundleName
            action.abilityName = AbilityName
            action.messageCode = SUBSCRIBE_CLIENT
            action.abilityType = ABILITY_TYPE_EXTERNAL
            action.syncOption = ACTION_SYNC
            let _this = this
            let result = await FeatureAbility.subscribeAbilityEvent(action, function (callbackData) {
                let callbackJson = JSON.parse(callbackData)
                let data = callbackJson.data.clientOut
                if(data == 'STOP'){
                    _this.unsubscribe()
                }else {
                    let value = JSON.parse(data)
                    _this.properties = value.services[0].properties
                }
            });
            let ret = JSON.parse(result)
            if (ret.code == 0) {
                this.subscribe_statu = true
                this.startflag = true
            } else {
                this.subscribe_statu = false
            }
        }else {
            prompt.showToast({
                message: '客户端尚未连接无法订阅',
                duration: 3000,
            })
        }
    },
    async unsubscribe(){
        let action = {}
        action.bundleName = BundleName
        action.abilityName = AbilityName
        action.messageCode = UNSUBSCRIBE_CLIENT
        action.abilityType = ABILITY_TYPE_EXTERNAL
        action.syncOption = ACTION_SYNC
        let result = await FeatureAbility.unsubscribeAbilityEvent(action)
        let ret = JSON.parse(result)
        if (ret.code == 0) {
            this.subscribe_statu = false
            this.startflag = false
            prompt.showToast({
                message: '已退出',
                duration: 3000,
            })
        } else {
            prompt.showToast({
                message: '退出失败',
                duration: 3000,
            })
        }
    },
    tocommand(e){
        let command = e.target.attr.data
        let str = {}
        str[command.paraName] = this.send_value
        this.sendCommand(str)
    },
    tocommandscheck(e){
        let command = e.target.attr.data
        let str = {}
        if(e.checked){
            str[command.paraName] = "ON"
        }else {
            str[command.paraName] = "OFF"
        }
        this.sendCommand(str)
    },
    doinputvalue(e){
        this.send_value = e.value
    },
    doinputip(e){
        this.ipaddress = e.value
    },
    doinputport(e){
        this.port = e.value
    },
    startdevice(e){
        if(this.startflag){
            this.unsubscribe()
        }
        else{
            this.$element('simpledialog').show()
        }
    },
    async sendCommand(str){
        if(this.client_statu && this.subscribe_statu) {
            let actionData = str
            let action = {}
            action.bundleName = BundleName
            action.abilityName = AbilityName
            action.messageCode = SEND_CLIENT
            action.data = actionData
            action.abilityType = ABILITY_TYPE_EXTERNAL
            action.syncOption = ACTION_SYNC
            let result = await FeatureAbility.callAbility(action)
            let ret = JSON.parse(result)
            if (ret.code != 0) {
                prompt.showToast({
                    message: '发送失败',
                    duration: 3000,
                })
            }
        }else {
            prompt.showToast({
                message: '客户端尚未连接无法发送',
                duration: 3000,
            })
        }
    },
    cancelDialog(e) {
        prompt.showToast({
            message: 'Dialog cancelled'
        })
    },
    getIPAddress(e) {
        this.$element('simpledialog').close()
        this.creatclient()
    },
    cancel(e) {
        this.$element('simpledialog').close()
    }
}
