/**
 *  HTTP请求工具类
 *  @Date 2020-04-26
 **/

/**
 * 获取HTTP请求头
 * @param {Object} currentHeader - 需要配置的请求头
 * @return {Object} header
 **/
function getHeader(currentHeader) {
    const header = currentHeader || {};
    header['content-type'] = header['content-type'] || 'application/json';
    return header;
}

/**
 * 检查网络状态
 **/
function checkNetwork() {
    return new Promise((resolve, reject) => {
        wx.getNetworkType({
            success: function(res) {
                // 返回网络类型, 有效值：
                // wifi/2g/3g/4g/unknown(Android下不常见的网络类型)/none(无网络)
                const networkType = res.networkType;

                if (networkType === 'none') {
                    wx.showModal({
                        title: '当前网络不可用，请检查网络设置',
                        confirmText: '重试',
                        success: function(res) {
                            if (res.confirm) {
                                checkNetwork();
                            } else {
                                reject(new Error('NetWorkError'));
                            }
                        }
                    });
                } else {
                    resolve();
                }
            }
        });
    });
}

export default {

    request(url, data, method, headers, complete) {
        return new Promise((resolve, reject) => {
            checkNetwork().then(() => {
                wx.request({
                    url: url,
                    data: data,
                    method: method || 'GET', //  OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT
                    header: getHeader(headers), // 设置请求的 header
                    success: (res) => {
                        // HTTP响应code
                        if (res.statusCode === 200) {
                            // 需要处理一些公关逻辑，比如公共错误业务code等等
                            resolve(res.data);
                        } else {
                            reject(res);
                        }
                    },
                    fail: (err) => {
                        reject(err);
                    },
                    complete
                });
            }).catch(err => {
                console.log(err);
            });
        });
    },

    get(opts = {}) {
        return this.request(opts.url, opts.data, 'GET', opts.headers);
    },

    post(opts = {}) {
        return this.request(opts.url, opts.data, 'POST', opts.headers);
    },
};
