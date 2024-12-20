function getOption(dataIndex, dataAll) {
    var option = {
        color: ["#d4237a"],
        grid: {
            top: '5%',
            left: '1%',
            right: '3%',
            bottom: '1%',
            containLabel: true
        },
        tooltip: {
            show: true,
            trigger: 'axis',
            formatter(params){
                let { data } = params[0];
                return data;
            }
        },
        xAxis: {
            type: 'category',
            boundaryGap: false,
            data: dataIndex,
            show: false,
            splitLine: {
                lineStyle: {
                    type: 'solid'
                }
            },
            axisLabel: {
                interval: 0,
                fontSize: 8,
            }
        },
        yAxis: {
            x: 'center',
            type: 'value',
            splitLine: {
                lineStyle: {
                    type: 'solid'
                }
            },
            axisLabel: {
                fontSize: 9,
                color:"#EC6D20",
            }
            // show: false
        },
        series: [{
            name: '温度曲线',
            type: 'line',
            smooth: true,
            lineStyle: {
                color: {
                    type: 'linear',
                    x: 0,
                    y: 0,
                    x2: 0,
                    y2: 1,
                    colorStops: [{
                        offset: 0, color: 'rgba(255,0,0,0.5)' // 0% 处的颜色
                    }, {
                        offset: 1, color: 'rgba(0,0,255,0.5)' // 100% 处的颜色
                    }],
                    global: false // 缺省为 false
                }
            },
            areaStyle: {
                color: {
                    type: 'linear',
                    x: 0,
                    y: 0,
                    x2: 0,
                    y2: 1,
                    colorStops: [{
                        offset: 0, color: 'rgba(255,0,0,0.5)' // 0% 处的颜色
                    }, {
                        offset: 1, color: 'rgba(0,0,255,0.5)' // 100% 处的颜色
                    }],
                    global: false // 缺省为 false
                }
            },
            symbol:'none',
            data: dataAll
        }]
    };
    return option;
  }

  exports.getOption = getOption;
