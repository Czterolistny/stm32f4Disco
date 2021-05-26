from http.server import BaseHTTPRequestHandler, HTTPServer
from enum import IntEnum, auto
import threading
import socket
import crcengine

import os
import time
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime
from matplotlib.dates import DateFormatter, MinuteLocator, date2num

UDP_IP = '192.168.1.105'
UDP_PORT = 8901

html_ctx = open('html/param.html', 'r')
html_log = open('html.log', 'a')
chart_data = open('log.txt', 'r')

serverHostName = '192.168.1.105'
serverPort = 8500

p_font = """<p style="font-size:200%;">"""
p_font2 = """<p style="font-size:120%;">"""
html_header = """<!DOCTYPE html>\n<meta http-equiv="refresh" content="3"><html><head>
\t<body> <h1 style="font-size:300%;">ST-DEFRO</h1>\n"""

def openSocket():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind((UDP_IP, UDP_PORT))
    except OSError:
        print("Cannot Open Socket %s at port %s" % UDP_IP, str(UDP_PORT) )
        sys.exit(0)
    return s

def openFiles():
    html = open('html/param.html', 'w+')
    log = open('log.txt', 'a')
    err = open('err.txt', 'a')
    return [html,log,err]

def drawFig():
    p = Plotter()
    while True:
        p.saveChart()
        time.sleep(60)

class Plotter:

    def __init__(self):

        self.__fig, self.__pl_main = plt.subplots()
        self.__pl_exhaust = self.__pl_main.twinx()
        self.__pl_fan = self.__pl_main.twinx()

        self.__pl_main.set_ylim(20, 70)
        self.__pl_exhaust.set_ylim(50, 300)

        self.__pl_fan.set_ylim(0, 100)
        self.__pl_fan.spines["right"].set_position(("axes", 1.1))

        self.__pl_main.set_ylabel('Temp Wody °C')
        self.__pl_exhaust.set_ylabel('Temp Spal °C')
        self.__pl_fan.set_ylabel('Wentylator %')

        self.__pl_main.set_yticks(np.arange(20,71,2))
        self.__pl_exhaust.set_yticks(np.arange(50, 300, 10))
        self.__pl_fan.set_yticks(np.arange(0, 101, 4))

        self.__p1, = self.__pl_main.plot(0, 0, color = 'mediumblue', label = 'Temp Wylot °C')
        self.__p4, = self.__pl_main.plot(0, 0, color = 'dodgerblue', label = 'Temp Wlot °C')
        self.__p2, = self.__pl_exhaust.plot(0, 0, color = 'mediumseagreen', \
                label = self.__pl_exhaust.get_ylabel())
        self.__p3, = self.__pl_fan.plot(0, 0, color = 'darkcyan', \
                label = self.__pl_fan.get_ylabel())

        self.__pl_main.yaxis.label.set_color(self.__p1.get_color())
        self.__pl_exhaust.yaxis.label.set_color(self.__p3.get_color())
        self.__pl_fan.yaxis.label.set_color(self.__p2.get_color())

        self.__pl_main.xaxis.set_major_formatter(DateFormatter('%H:%M'))
        self.__pl_main.xaxis.set_major_locator(MinuteLocator(interval=5))
        self.__pl_main.xaxis.set_minor_locator(MinuteLocator(interval=1))

        tkw = dict(size=2, width=1.0)
        self.__pl_main.tick_params(axis='y', colors = self.__p1.get_color(), **tkw)
        self.__pl_exhaust.tick_params(axis='y', colors = self.__p2.get_color(), **tkw)
        self.__pl_fan.tick_params(axis='y', colors = self.__p3.get_color(), **tkw)
        self.__pl_main.tick_params(axis='x', **tkw)

        lines = [self.__p1, self.__p4, self.__p2, self.__p3]
        self.__pl_main.legend(lines, [l.get_label() for l in lines], loc='upper left')

        self.__pl_main.grid(True)
        self.__fig.autofmt_xdate()
        self.__fig.tight_layout()
        self.__fig.set_size_inches(7.5, 6)

    def saveChart(self):

        date = []; t1 = []; t2 = []; t3 = []; fan = [];
        pos = chart_data.seek(0, os.SEEK_END)
        chart_data.seek(pos - 90000)
        chart_data.readline()

        for line in chart_data:
            d = line.split()
            _date = d[0].split('.')
            _hour = d[1].split(':')
            date.append(date2num(np.datetime64(datetime(21, int(_date[1]), int(_date[0]),\
                int(_hour[0]), int(_hour[1]), int(_hour[2])).strftime("%y-%m-%d %H:%M:%S"))))
            t1.append(int(d[3]))
            t2.append(int(d[4]))
            t3.append(int(d[5]))
            fan.append(int(d[6]))


        self.__pl_main.plot(date, t1, self.__p1.get_color(), label = 'Temp Wylot °C')
        self.__pl_main.plot(date, t2, self.__p4.get_color())
        self.__pl_exhaust.plot(date, t3, self.__p2.get_color(), label = 'Temp Spal °C')
        self.__pl_fan.plot(date, fan, self.__p3.get_color(), label = 'Wentylator %')

        self.__pl_main.set_xlim(date[0], date[-1])

        plt.savefig('html/chart.png', dpi=100)
        #self.__fig.clf()
        #plt.close(self.__fig)

def writeHTML(html, data, time):
    html.seek(0,0)
    html.write(html_header)
    setTemp,outTemp,inTemp,exTemp,fanSpeed = data[data.find(' '):].split()
    html.write('\t\t' + p_font + 'Temp Zadana: %s &#176C</p>\n' % setTemp)
    html.write('\t\t' + p_font + 'Temp Wylot: &#160&#160 %s &#176C</p>\n' % outTemp)
    html.write('\t\t' + p_font + 'Temp Spalin: &#160 %s &#176C</p>\n' % exTemp)
    html.write('\t\t' + p_font + 'Wentylator: &#160&#160&#160 %s &#37</p>\n' % fanSpeed)
    html.write('\t\t' + p_font + 'Temp Wlot: &#160&#160&#160 %s &#176C</p>\n' % inTemp)
    html.write('\t\t' + p_font2 + '%s</p>\n' % time[time.find('_') + 1:])
    html.write('\t\t<p><a href=img.html> Wykres </a></p>\n')
    html.write('\t</body>\n</html>\n')

def checkCRC(crc, data):
    if(str(crc(data[data.find(' '):].encode('utf-8'))) == data[:data.find(' ')]):
        return True
    else:
        return False

lifo_data = ['','','','']
lifo_ret = [False,False,False,False]
def checkDataVal(data):

    deviation = [1,1,3]
    for i in range(2, -1, -1):
        lifo_data[i+1] = lifo_data[i]
        lifo_ret[i+1] = lifo_ret[i]

    lifo_data[0] = data.split()
    lifo_ret[0] = True
    
    if( lifo_data[2] != ''):
        for i in range(2,5):
            diff = abs(int(lifo_data[0][i]) - int(lifo_data[2][i]));
            d1 = abs(int(lifo_data[0][i]) - int(lifo_data[1][i]));
            d2 = abs(int(lifo_data[2][i]) - int(lifo_data[1][i]));
            if( (d1 > diff + deviation[i-2]) or (d2 > diff + deviation[i-2]) ):
                lifo_ret[1] = False
                break;
            else:
                lifo_ret[1] = True
            
    return (lifo_ret[3], ' '.join(lifo_data[3]) + '\n')

def logEnabled(temp):
    return True

def checkDataIntegrity(data):
    if( (int(data.split(' ')[1]) > 80) or (int(data.split(' ')[1]) < 45) ):
        return False
    if( (int(data.split(' ')[2]) > 110) or (int(data.split(' ')[3]) > 100)\
    or (int(data.split(' ')[4]) > 330 ) or (int(data.split(' ')[5]) > 100) ):
        return False
    return True

def processData(socket, html, log, err):
    
    crc = crcengine.create(0x2f5c, 16, 0xb169, ref_in=False, ref_out=False, xor_out=0, name='crc-16')
    while True: 
        d, addr = socket.recvfrom(32)
        _time = datetime.now()
        time = _time.strftime('%d.%m %H:%M:%S')

        data = d.decode('utf-8')
        
        if( checkCRC(crc, data) == False ):
            err.write('CRC_ERR ' + time + ' ' + data)
        elif( checkDataIntegrity(data) == False ):
            err.write('RNG_ERR ' + time + ' ' + data)
        else:
            writeHTML(html, data, time)
            if( logEnabled(int(data.split(' ')[2])) == True ):
                valErr, data = checkDataVal(data)
                if( (valErr != False) and (data != '') ):
                    log.write(time + ' ')
                    log.write('%s' % data[data.find(' ') + 1:])
                    log.flush()
                if( (valErr == False) and (data != '') ):
                    err.write('VAL_ERR ' + time + ' ' + data)


class MyServer(BaseHTTPRequestHandler):
    
    __allow_reqs = ('param.html', 'favicon.ico', 'chart.png', 'img.html')
    def do_GET(self):
        req = self.requestline.split(' ')[1].lstrip('/')
        allowedReq = False
        for i in self.__allow_reqs:
            if( i == req ):
                allowedReq = True
        if(allowedReq == True):
            self.send_response(200)
        else:
            self.send_response(404)

        self.send_header("Content-type", "param.html")
        self.end_headers()
        
        self.read_sendCtx(req)
    
    def read_sendCtx(self, req):
        try:
            f_ctx = open('html/'+req, 'rb')
        except FileNotFoundError:
            return
        buf = f_ctx.read()
        self.wfile.write(buf)
        f_ctx.close()

    def log_message(self, format, *args):
        html_log.write("%s - - [%s] %s\n" % (self.address_string(), self.log_date_time_string(),\
        format % args))
        html_log.flush()

def runHTTPServer():

    webServer = HTTPServer((serverHostName, serverPort), MyServer)
    print("Server started http://%s:%s" % (serverHostName, serverPort))
    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        webServer.server_close()
        print('Server closed')
        return 0

def main():

    httpThread = threading.Thread(target=runHTTPServer, args=())
    httpThread.start()
    charThread = threading.Thread(target=drawFig, args=())
    charThread.start()

    html, log,err = openFiles()
    socket = openSocket()
    processData(socket, html, log, err)
   
if __name__=="__main__":
    main()

html.close()
html_log.close()
err.close()
log.close()
