// index.js
var event = require('../../utils/event');//导入event
var echatUtil = require('../../utils/echatUtil');//导入echart option
var echarts = require('../../components/ec-canvas/echarts.min');

const app = getApp();

// 记录当前历史数据
var dataIndex = [];
var dataAll = [];

function initChart(canvas, width, height, dpr) {
  const chart = echarts.init(canvas, null, {
    width: width,
    height: height,
    devicePixelRatio: dpr // new
  });
  canvas.setChart(chart);

  var option = echatUtil.getOption(dataIndex, dataAll);

  chart.setOption(option);
  mChart = chart;
  return chart;
}

var mChart = null;

Page({
  data: {
    topic: {
      emojiSwitch: 'emoji',  // 切换开关
    },
    key: {
      voice: "voice",
      emoji: "emoji",
    },
    voice: "0",
    line: {
      onInit: initChart
    },
  },

  /**
   * 生命周期函数--监听页面初次渲染完成
   */
  onReady: function () {
    /**
     * 第一个参数：指定订阅的事件名
     * 第二个参数：对象
     * 第三个参数：指定订阅回调的方法
     */
    event.on("sensor", this, this.receiveMessage)
  },

  /**
   * 生命周期函数--监听页面显示
   */
  onShow: function () {
    if(!app.checkStartMqttConnect()) {
      app.connect();  //连接服务器
    }
  },

  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload: function () {
    //取消事件绑定
    event.remove("sensor", this);
  },

  /**
   * 接收订阅回调的方法
   * @param evntdata 接收更新数据
   * @constructor
   */
  receiveMessage(msg) {
    let that = this;
    console.log("更新数据===》", msg);
    //赋值
    let value = msg[that.data.key.voice];

    this.updateChart(value);
    this.setData({
      voice: value,
    });
  },

  // 更新曲线
  updateChart(value) {
    console.log("voice===》", value);
    if (dataAll.length >= 100) {
      // 踢掉第一个
      dataAll.shift();
      dataIndex.shift();
      dataAll.push(value);
      dataIndex.push(0);
    } else {
      dataAll.push(value);
      dataIndex.push(0);
    }
    var option = echatUtil.getOption(dataIndex, dataAll);
    mChart.setOption(option);
  },
  onEmoji1Switch(){
    let that = this;
    let msg = JSON.parse("{\"emoji\":1}");
    console.log(JSON.stringify(msg));
    app.data.client.publish(that.data.topic.emojiSwitch, JSON.stringify(msg));
  },
  onEmoji2Switch(){
    let that = this;
    let msg = JSON.parse("{\"emoji\":2}");
    console.log(JSON.stringify(msg));
    app.data.client.publish(that.data.topic.emojiSwitch, JSON.stringify(msg));
  },
  onEmoji3Switch(){
    let that = this;
    let msg = JSON.parse("{\"emoji\":3}");
    console.log(JSON.stringify(msg));
    app.data.client.publish(that.data.topic.emojiSwitch, JSON.stringify(msg));
  },
  onEmoji4Switch(){
    let that = this;
    let msg = JSON.parse("{\"emoji\":4}");
    console.log(JSON.stringify(msg));
    app.data.client.publish(that.data.topic.emojiSwitch, JSON.stringify(msg));
  },
});
