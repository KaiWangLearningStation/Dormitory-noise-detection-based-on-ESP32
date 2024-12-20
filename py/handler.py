# Prepared Request
# 实际上，requests 在发送请求的时候，是在内部构造了一个 Request 对象，
# 并给这个对象赋予了各种参数，包括 url、headers、data 等等，
# 然后直接把这个 Request 对象发送出去，请求成功后会再得到一个 Response 对象，再解析即可。
# 那么这个 Request 是什么类型呢？实际上它就是 Prepared Request。

# pip install requests

import schedule
import json
import time
import datetime
from requests import Request, Session

uid = 'a0f3348e687648848ceaeb3e970f6ff5'
topic= 'dB'
topic2= 'dBHour'
msgUrl1 = 'https://apis.bemfa.com/va/getmsg?type=1&num=100&uid=%s&topic=%s'%(uid,topic)
msgUrl2 = 'https://apis.bemfa.com/va/getmsg?type=1&num=1&uid=%s&topic=%s'%(uid,topic2)
postMsgUrl = 'https://apis.bemfa.com/va/postJsonMsg'
headers = {
    'Content-Type':'application/json; charset=utf-8;',
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.116 Safari/537.36'
}

# 计算最近平均值
def handleDBAvg():
    s = Session()
    req = Request('GET', msgUrl1, headers=headers)
    prepped = s.prepare_request(req)
    r = s.send(prepped)
    print(r.json())
    list = r.json()['data']
    sum = 0
    leng = len(list)
    for item in iter(list):
        msg = item['msg']
        jb = json.loads(msg)
        # print(jb)
        sum = sum + jb['voice']
        pass
    #求平均值
    avg = sum / leng
    print(avg)
    return avg

# 获取数字
def getDBHour(value):
    t = time.localtime()
    hour = t.tm_hour % 12
    print('hour:%d'%(hour))
    s = Session()
    req = Request('GET', msgUrl2, headers=headers)
    prepped = s.prepare_request(req)
    r = s.send(prepped)
    # print(r.json())
    list = r.json()['data']
    leng = len(list)
    jb = ''
    if leng == 1:
        item = list[0]['msg']
        jb = json.loads(item)
        jb['%d'%(hour)] = value
    print(jb)    
    return json.dumps(jb)    


# 获取数字
def updateDBHour(value):
    print('updateDBHour %s'%(value))
    data = {
        'uid':uid,
        'topic':topic2,
        'type':1,
        'msg':'%s'%(value),
    }
    print(data)
    s = Session()
    req = Request('POST', postMsgUrl, json=data, headers=headers)
    prepped = s.prepare_request(req)
    r = s.send(prepped)
    print(r.text)

def job():
    print('job:每隔1分钟执行一次')
    print('job-startTime:%s' % (datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
    value = handleDBAvg()
    db = getDBHour(value)
    updateDBHour(db)
    print('job-endTime:%s' % (datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
    print('------------------------------------------------------------------------')

if __name__ == '__main__':
    index = 1
    print('计划任务开始执行,每间隔 %d 分钟执行一次'%(index))
    schedule.every(index).minutes.do(job)
    while True:
        schedule.run_pending()

    # 测试是否阻塞
    print('测试是否阻塞')


