/**
 *  Storage存储工具类
 *  包括 CURD方法
 *  @Date 2020-04-26
 **/

const getStorageValue = (value, expireFn) => {
    if (value === '' || value === null || value === undefined) {
        return '';
    }
    let expire = value.expire;
    if (expire && /^\d{13}$/.test(expire)) {
        let d = new Date().getTime();
        if (expire <= d) {
            expireFn && expireFn();
            return '';
        }
    }
    return value.value;
};

let storage = {
    /**
     * 异步保存值到本地存储
     * @param {String} key     需要保存的键名
     * @param {Object|String|Array|Boolean} value  需要保存的值
     * @param {Number} expires 存储的过期时间（单位：s）
     */
    set: function(key, value, expires = 0) {
        let v = {};
        if (expires) {
            let d = new Date().getTime();
            v.expire = d + expires * 1000;
        }
        v.value = value;
        try {
            wx.setStorage({
                key: key,
                data: v,
            });
        } catch (e) {
            console.log('setStorage fail:  ' + e);
        }
    },
    /**
     * 同步保存值到本地存储
     * @param {String} key     需要保存的键名
     * @param {Object|String|Array|Boolean} value  需要保存的值
     * @param {Number} expires 存储的过期时间（单位：s）
     */
    setSync: function(key, value, expires = 0) {
        let v = {};
        if (expires) {
            let d = new Date().getTime();
            v.expire = d + expires * 1000;
        }
        v.value = value;
        try {
            wx.setStorageSync(key, v);
        } catch (e) {
            console.log('setStorageSync fail:  ' + e);
        }
    },

    /**
     * 需要获取对应key的本地存储
     * @param  {String} key 对应的key
     * @return {Object|String|Array|Boolean}  返回值
     */
    get: function(key) {
        let value;
        try {
            value = wx.getStorageSync(key);
            let expireFn = () => { wx.removeStorageSync(key) };
            return getStorageValue(value, expireFn);
        } catch (e) {
            console.log('getStorage fail:  ' + e);
            return '';
        }

    },
    /**
     * 删除一个或多个本地存储
     * @method remove
     * @param  {Array||String} key 需要删除的key
     */
    remove: function(key) {
        let that = this;
        if (typeof (key) === 'object') {
            for (let i in key) {
                that._remove(key[i]);
            }

        } else {
            this._remove(key);
        }
    },
    /**
     * 删除一个本地存储
     * @method _remove
     * @param  {String} key 需要删除的key
     */
    _remove: function(key) {
        try {
            wx.removeStorageSync(key);
        } catch (e) {
            console.log('removeStorage fail:  ' + e);
        }
    }
};

export { storage };
