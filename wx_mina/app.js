// app.js
import mqtt from './utils/mqtt.js';
var event = require('./utils/event');//导入event

App({
  data:{
    isMqttConnected: false, // 判断是否连接上MQTT
    isStartConnectMqtt: false, // 判断是否已经启动连接MQTT
    payload:"",
    client: null,//记录重连的次数
    reconnectCounts: 0,//MQTT连接的配置
    options: {
      protocolVersion: 4, //MQTT连接协议版本
      clean: false,
      reconnectPeriod: 1000, //1000毫秒，两次重新连接之间的间隔
      connectTimeout: 30 * 1000, //30秒，两次重新连接之间的间隔
      resubscribe: true, //如果连接断开并重新连接，则会再次自动订阅已订阅的主题（默认true）
      clientId: 'a0f3348e687648848ceaeb3e970f6ff5',
      password: '',
      username: '',
      pubTopic: '',
      subTopic: 'dB'
    },
  },

  /**
   * 判断mqtt是否启动连接
   */
  checkStartMqttConnect: function(){
    console.log("checkStartMqttConnect:" + this.data.isMqttConnected);
    return this.data.isStartConnectMqtt;
  },

  /**
   * 重新连接mqtt，应用于重新配置三元组连接
   */
  reconnect: function() {
    this.data.isStartConnectMqtt = false;
    this.connect();
  },

  /**
   * 连接mqtt服务器，并监听事件
   */
  connect:function(e){
    var that = this;

    if (this.data.isStartConnectMqtt){
      console.log("connect has already connect!");
      return;
    }

    let host = 'wxs://bemfa.com:9504/wss';

    console.log("这就是网址" + host);
    console.log("this.data.options data:" + JSON.stringify(this.data.options));

    this.data.client = mqtt.connect(host, this.data.options);
    console.log(this.data.client);

    that.data.isStartConnectMqtt = true;

    this.data.client.on(
        'connect',
        function (connack) {
          that.data.isMqttConnected = true;
          var status = {
            status: 1
          };
          event.emit("connect", status); //发送更新数据
          wx.showToast({ title: '连接成功' });

          that.data.client.subscribe(that.data.options.subTopic);
        }
    );

    that.data.client.on("message", function (topic, payload) {
      console.log(" 收到 topic:" + topic + " , payload :" + payload);
      //第一个参数： 发送给指定的事件的名词，第二个参数：需要发送的数据
      // {"v":18.3}}
      var obj = JSON.parse("" + payload);
      if(topic === that.data.options.subTopic) {
        event.emit("sensor", obj) //发送更新数据
      }
    });
    //服务器连接异常的回调
    that.data.client.on("error", function (error) {
      that.data.isMqttConnected = false;
      console.log(" 服务器 error 回调" + error);
      wx.showToast({ title: '连接失败' })
    });
    //服务器重连连接异常的回调
    that.data.client.on("reconnect", function () {
      that.data.isMqttConnected = false;
      console.log(" 服务器 reconnect回调");
      wx.showToast({ title: '重连失败' })
    });
    //服务器重连连接异常的回调
    that.data.client.on("end", function () {
      that.data.isMqttConnected = false;
      console.log(" 服务器 end回调")
    });
    //服务器连接异常的回调
    that.data.client.on("offline", function (errr) {
      that.data.isMqttConnected = false;
      console.log(" 服务器offline回调");
      wx.showToast({ title: '服务离线' })
    })},
});
