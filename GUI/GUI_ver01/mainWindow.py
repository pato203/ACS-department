from tkinter import *
from tkinter import ttk #fancy style
from tkinter import messagebox
from tkinter import filedialog

# excell library
import xlrd
import xlwt

# matplotlib imports
import matplotlib
matplotlib.use("TkAgg")  # tkinter backend for matplotlib
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure # it has to be explicit
import matplotlib.animation
from matplotlib import style

# Serial imports
import serial.tools.list_ports
import serial

import datetime

# Classes
class ArduinoData:
    def __init__(self, time = 0.0, acc = [], gyro = [] ):
        self.time = time
        self.acc = acc
        self.gyro = gyro

    def __repr__(self):  # to represent in list() function
        return "t:" + str(self.time) + "acc:" + str(self.acc) + "gyr:" +str(self.gyro) + "\n"


# control variables
servMin=5
servMax=175
finZero=90
# Serial stuff
baudrates = [9600, 57600, 115200]
mySerial = serial.Serial()
portList = serial.tools.list_ports.comports()
connectCtrl = False  # serial.is_open doesnt prevent serial.in_waiting from throwing an exception
sendCtrl = False
streamCtrl = False
# Graphs
freezeGraphs = False
# Data sets
# received:
dataCount = 10000
dataIndex = 0
receivedList = [None] * dataCount
timeList = []
serList = []
# to send:
sendIndex = 0
beginningTime = datetime.datetime.now()
pauseTimeDelta = datetime.timedelta()  # set to zero

# handlers
def serialHandler():
    global sendIndex  # for some reason just this one
    #global connectCtrl, sendIndex, sendCtrl, timeList, beginningTime
    if mySerial.is_open:
        if connectCtrl:  # reading
            try:
                if (mySerial.in_waiting > 0):
                    updateData()
                    #print(data)
            except:
                    print("something went wrong")
                    e = sys.exc_info()[0]
                    print(e)
        # writing
        if sendCtrl and (len(timeList) >0) and (sendIndex < len(timeList)) and ((datetime.datetime.now() - beginningTime + pauseTimeDelta).total_seconds() >= timeList[sendIndex]): #
            print("sending data")
            sendData()
            sendIndex += 1
    window.after(1, serialHandler)


# GUI
# Toplevel menu handlers
def help(event):
    helpMsg()

def helpMsg():
    popup = Tk()
    popup.title("Help")
    content = "\t\t*****В разработке*****\nДля заморожения/разморожения графиков нажмите \"Shift\"+\"F\""
    label = ttk.Label(popup, text=content)
    label.pack(side="top", fill="x", pady=10)
    Btn1 = ttk.Button(popup, text="Закрыть", command=popup.destroy)
    Btn1.pack()
    popup.mainloop()

# Closing window
def on_closing():
    if messagebox.askokcancel("Quit", "Do you want to quit?"):
        if mySerial.is_open:
            mySerial.close()
            #ADD LINES LATER!
        window.destroy()

# Serial communication
def serialConnect(widget):
    mySerial.baudrate = combo2.get()
    mySerial.timeout = 1
    #mySerial.bytesize = 8
    #mySerial.stopbits = 1
    #mySerial.parity = 'N'
    if len(portList) > 0:
        mySerial.port = portList[combo1.current()].device
        mySerial.open()
        global connectCtrl
        connectCtrl = True
        widget.configure(text="Disconnect from Arduino\n(btn8)",
                         command=lambda: serialDisconnect(widget))

def serialDisconnect(widget):
    global connectCtrl
    connectCtrl = False
    mySerial.close()
    widget.configure(text="Connect to Arduino\n(btn8)",
                     command=lambda: serialConnect(widget))

#graph refreshing from file
def animate2(i):
    dataFile = open("sampleText.txt", "r")
    pullData = dataFile.read()
    dataList = pullData.split('\n')
    xList = []
    yList = []
    for eachLine in dataList:
        if len(eachLine) > 1:  # precaution not to read empty line
            x, y = eachLine.split(',')
            xList.append(float(x))
            yList.append(float(y))

    a2.clear()  # always draw just current graph
    a2.plot(xList, yList)

def switchGraphFreeze(event):
    global freezeGraphs
    freezeGraphs = not freezeGraphs


def animate(i):
    global receivedList, dataIndex, freezeGraphs
    dataInd = dataIndex
    tList = [None]*dataInd
    accList = [[None]*dataInd,[None]*dataIndex,[None]*dataIndex]
    gyrList = [[None]*dataInd,[None]*dataIndex,[None]*dataIndex]
    for i in range(0, dataInd):
        tList[i] = receivedList[i].time
        for j in range(0,3):
            accList[j][i] = receivedList[i].acc[j]  # to turn the "array"
            gyrList[j][i] = receivedList[i].gyro[j]

    if dataInd > 0:
        left1, right1 = a1.get_xlim()
        bottom1, top1 = a1.get_ylim()
        left2, right2 = a2.get_xlim()
        bottom2, top2 = a2.get_ylim()
        a1.clear()  # always draw just current graph
        a1.plot(tList, gyrList[0], "#FF0000")
        a1.plot(tList, gyrList[1], "#BB1111")
        a1.plot(tList, gyrList[2], "#885555")

        a2.clear()
        a2.plot(tList, accList[0], "#FF0000") #x
        a2.plot(tList, accList[1], "#00FF00") #y
        a2.plot(tList, accList[2], "#0000FF") #z

        if freezeGraphs:
            a1.set_xlim(left1, right1)
            a1.set_ylim(bottom1, top1)
            a2.set_xlim(left2, right2)
            a2.set_ylim(bottom2, top2)


#Data management
def saveToFile(receivedList):
    if len(receivedList):
        currentTime = datetime.datetime.now().strftime("%Y-%m-%d %H-%M-%S") #string of current time with 1 sec accuracy

        #dataFile = open("Data from test "+currentTime+".txt", "w")
        book = xlwt.Workbook()
        sheet = book.add_sheet("Test results")
        row = sheet.row(0)
        row.write(0, "t, s")
        row.write(1, "ax, m/s^2")
        row.write(2, "ay, m/s^2")
        row.write(3, "az, m/s^2")
        row.write(4, "gyrx, rad/s")
        row.write(5, "gyry, rad/s")
        row.write(6, "gyrz, rad/s")

        rowIndex = 1
        for receivedData in receivedList:
            row = sheet.row(rowIndex)
            row.write(0,receivedData.time)
            for i in range(0, 3):
                row.write(1 + i,receivedData.acc[i])
            for i in range(0, 3):
                row.write(4 + i,receivedData.gyro[i])
            rowIndex += 1
        book.save("Data from test "+currentTime+".xls")
        print("Data saved to \"Data from test"+currentTime+".xls\"")


def saveToFile2(receivedList): #older version
    if len(receivedList):  # if longer than
        currentTime = datetime.datetime.now().strftime("%Y-%m-%d %H-%M-%S") #string of current time with 1 sec accuracy
        dataFile = open("Data from test "+currentTime+".txt", "w")
        for receivedData in receivedList:
            dataFile.write("t:"+str(receivedData.time)+"\n")
            dataFile.write("acc:")
            for i in range(0, 3):
                dataFile.write(str(receivedData.acc[i]))
            dataFile.write("\n")
            dataFile.write("gyr:")
            for i in range(0, 3):
                dataFile.write(str(receivedData.gyro[i]))
            dataFile.write("\n")
        dataFile.close()

def loadFromFile():
    global timeList, serList
    t = []
    ser = []
    errOccurred = False
    fileName = filedialog.askopenfilename(title="Select File", filetypes=(("MS Excell workbooks","*.xlsx *xls"),("Text files","*.txt"),("all files","*.*")))
    #print(fileName)

    if fileName != "":
        extension = fileName.split(".")[1]
        if (extension == "xlsx") or (extension== "xls"):
            f = xlrd.open_workbook(fileName)
            sheet = f.sheet_by_index(0) #OpenFirstSheet Indexation starts from 0!
            for i in range(1, sheet.nrows):
                try:
                    t.append(sheet.cell_value(i, 0))
                    s = []
                    for pos in sheet.row_values(i, 1, 5):  #i.e. 1,2,3,4
                        s.append(int(pos))
                    ser.append(s)
                except:
                    print("File read error!\nCheck your file.")
                    errOccurred = True
                    break
        elif extension == "txt":
            f = open(fileName, "r")
            for line in f:
                line = line.strip()
                try:
                    if line[:2] == "t:":
                        t.append(float(line[2:]))
                    else:
                        ser.append(parseSer(line))
                except:
                    print("File read error!")
                    errOccurred = True
                    break
        else:
            print("File read error!\nFile format not supported.")

        print(t, ser) #debugging

        if not errOccurred:
            print("Data loaded successfully")
        else:
            print("Loading error has occurred.")

        timeList = t[:]
        serList = ser[:]
    else:
        print("No file selected")

def parseSer(line):
    ser = []
    line = line[4:] #ser:12,15,48
    inStrList = line.split(",")
    for s in inStrList:
        ser.append(int(s))
    return ser[:]  #[;] is not necessary, but better save than sorry

def updateData():
    global dataIndex
    acc = []
    gyro = []
    t = 0
    dataComplete = True
    try:
        for i in range(0, 3):
            inStr = mySerial.readline()
            print(inStr)
            inStr = inStr.decode()
            inStr = inStr.strip()

            if inStr[:2] == "t:": #i.e. 't:'
                inStr = inStr[2:len(inStr)-1] #t:154894
                print(inStr) #debuging
                t = float(inStr)
            elif inStr[:4] == "acc:": #i.e. 'acc:'
                inStr = inStr[4:]
                inStrList = inStr.split(",")
                for s in inStrList:
                    acc.append(float(s))
            elif inStr[:4] == "gyr:": #i.e. 'gyr:'
                inStr = inStr[4:]
                inStrList = inStr.split(",")
                for s in inStrList:
                    gyro.append(float(s))
            else:
                dataComplete = False
                print(inStr)
                break #!!! try make something better
        if dataComplete:
            receivedList[dataIndex] = ArduinoData(t, acc, gyro)
            dataIndex += 1
    except:
        print("Data read error: ")
        e = sys.exc_info()[0]
        print(e)

def sendData():
    global serList, sendIndex
    mySerial.write(b"ser\n")
    for i in range(0,3): # 4 values in total
        mySerial.write(str(serList[sendIndex][i]).encode()+b",")
    mySerial.write(str(serList[sendIndex][3]).encode()+b"\n")
    print("Data has been sent")

def clearData():
    global dataIndex, receivedList
    if messagebox.askokcancel("Clear", "Do you want to clear received data?"):
        receivedList = [None] * dataCount
        dataIndex = 0



def sendServoPositions():
    global serList
    serList = [["","","",""]]
    serList[0][0] = spin1.get()
    serList[0][1] = spin2.get()
    serList[0][2] = spin3.get()
    serList[0][3] = spin4.get()
    stopSending()  # terminate test program
    if mySerial.is_open:
        # writing
        if (len(serList) >0): #
            print("sending manual imput")
            sendData()


def setServoZero():
    global serList, servMin
    serList = [["","","",""]]
    for i in range(0, 4):
        serList[0][i] = servMin
    stopSending()  # terminate test program
    if mySerial.is_open:
        # writing
        if (len(serList) >0): #
            print("sending zero positions input")
            sendData()


def setFinZero():
    global serList, finZero
    serList = [["","","",""]] # clear serList(preprogramed positions)
    for i in range(0, 4):
        serList[0][i] = finZero
    stopSending()  # terminate test program
    if mySerial.is_open:
        # writing
        if (len(serList) >0): #
            print("sending zero tail fin positions input")
            sendData()


def startSending():  #starts preprogramed instructions sending
    global sendCtrl, beginningTime
    if not sendCtrl:
        beginningTime = datetime.datetime.now()
    sendCtrl = True


def stopSending():
    global sendCtrl, sendIndex, pauseTimeDelta
    sendCtrl = False
    sendIndex = 0
    pauseTimeDelta = datetime.timedelta()  # i.e. zero


def pauseSending():
    global sendCtrl, beginningTime, pauseTimeDelta
    if sendCtrl:  # pause only if execution already started
        sendCtrl = False
        pauseTimeDelta = datetime.datetime.now() - beginningTime + pauseTimeDelta  #to continue where we ended
    else:
        print("Ошибка: испытания по программе не начались.")


#Send Arduino command to start sending stuff
def streamControl(widget):
    global streamCtrl
    streamCtrl = not streamCtrl
    if streamCtrl:
        mySerial.write(b"a")
        widget.configure(text="Закрыть поток\n(btn7)")
    else:
        mySerial.write(b"t")
        widget.configure(text="Открыть поток\n(btn7)")



# GUI
window = Tk()
window.title("Управление аэродинамическими испытаниями")
window.bind('<F>', switchGraphFreeze)
window.bind('<F1>', help)

menuBar = Menu(window)
moreMenu = Menu(menuBar, tearoff=0)
moreMenu.add_command(label="Help", command=helpMsg)
menuBar.add_cascade(label="More", menu=moreMenu)
window.config(menu=menuBar)

# ***Frame1***
frame1 = Frame(window, bd=10)
frame1.grid(column=1, row=0)

lbl1 = Label(frame1, text="Управление сервоприводами")
lbl1.grid(column=0, row=0)

buttonWidth1 = 20

button1 = ttk.Button(frame1, text="Нуля\nсервоприводов\n(Btn1)",
                     width=buttonWidth1,
                     command=setServoZero)
button1.grid(column=0,row=1, pady=5)

button2 = ttk.Button(frame1, text="Нуля рулей (Btn2)",
                     width=buttonWidth1,
                     command=setFinZero)
button2.grid(column=0,row=2, pady=5)

button3 = ttk.Button(frame1, text="Настроить сервоприводы\nвручную (Btn3)",
                     width=buttonWidth1,
                     command=sendServoPositions)
button3.grid(column=0,row=3, pady=5)

#subframe 1_1
frame1_1 = Frame(frame1)
frame1_1.grid(column=0)

lbl1_1 = Label(frame1_1, text="Серво 1: ")
lbl1_1.grid(column=0,row=0)

lbl1_2 = Label(frame1_1, text="Серво 2: ")
lbl1_2.grid(column=0,row=1)

lbl1_3 = Label(frame1_1, text="Серво 3: ")
lbl1_3.grid(column=0,row=2)

lbl1_4 = Label(frame1_1, text="Серво 4: ")
lbl1_4.grid(column=0,row=3)

spin1 = Spinbox(frame1_1, from_=servMin, to=servMax, width=5)
spin1.grid(column=1,row=0)

spin2 = Spinbox(frame1_1, from_=servMin, to=servMax, width=5)
spin2.grid(column=1,row=1)

spin3 = Spinbox(frame1_1, from_=servMin, to=servMax, width=5)
spin3.grid(column=1,row=2)

spin4 = Spinbox(frame1_1, from_=servMin, to=servMax, width=5)
spin4.grid(column=1,row=3)

# ***Frame2***
frame2 = Frame(window, bd=10)
frame2.grid(column=1, row=1)

lbl2 = Label(frame2, text="Управление данными")
lbl2.grid(column=0,row=0)

buttonWidth2 = 15

button4 = ttk.Button(frame2, text="Сохранить\n(Btn4)",
                     width=buttonWidth2,
                     command=lambda: saveToFile(receivedList[:dataIndex]))
button4.grid(column=0,row=1, pady=5)

button5 = ttk.Button(frame2, text="Открыть файл\n(Btn5)",
                     width=buttonWidth2,
                     command=loadFromFile)
button5.grid(column=0,row=2, pady=5)

button6 = ttk.Button(frame2, text="Очистить(Btn6)",
                     width=buttonWidth2,
                     command=clearData)
button6.grid(column=0,row=3, pady=5)

# ***Frame3***
frame3 = Frame(window, bd=10)
frame3.grid(column=0, row=1, stick="w")

lbl3 = Label(frame3, text="Управление испытаниями")
lbl3.grid(column=0,row=0)

buttonWidth3 = 15

button7 = ttk.Button(frame3, text="Запустить поток(Btn7)",
                     width=buttonWidth3,
                     command=lambda: streamControl(button7))
button7.grid(column=0, row=1, pady=5)

button8 = ttk.Button(frame3, text="Connect to\n Arduino\n(Btn8)",
                     width=buttonWidth3, command=lambda: serialConnect(button8))
button8.grid(column=1, row=1, pady=5)

button9 = ttk.Button(frame3, text="Старт(Btn9)",
                     width=buttonWidth3,
                     command=startSending)
button9.grid(column=0,row=2, pady=5)

button10 = ttk.Button(frame3, text="Стоп(Btn10)",
                      width=buttonWidth3,
                      command=stopSending)
button10.grid(column=1,row=2, pady=5)

button11 = ttk.Button(frame3, text="Пауза(Btn11)",
                      width=buttonWidth3,
                      command=pauseSending)
button11.grid(column=2, row=2, pady=5)

combo1 = ttk.Combobox(frame3, state="readonly") #add postcommand=scanSeralPorts
combo1["values"]=portList
if len(portList)>0:
 combo1.current(0)
combo1.grid(column=2, row=1)

combo2 = ttk.Combobox(frame3, state="readonly")
combo2["values"]=baudrates
combo2.current(2)
combo2.grid(column=3, row=1)

# ***Frame4***
frame4 = Frame(window, bd=10)
frame4.grid(row=0, column=0)
#Matplotlib
#Graphs
style.use("ggplot") #change later to custom style
f = Figure(figsize=(10,5), dpi=100) # figsize = width*height find out what does what
f.subplotpars.update(0.06, 0.1, 0.99, 0.95)
a1 = f.add_subplot(121) #rownumber*colnumber position
a2 = f.add_subplot(122) #probably 112


canvas = FigureCanvasTkAgg(f, frame4)
canvas.draw()
canvas.get_tk_widget().pack()#grid(row=0, column=0) #otherwise ther is an err with toolbar

toolbar = NavigationToolbar2Tk(canvas, frame4)
toolbar.update()
canvas._tkcanvas.pack(side=TOP, fill=BOTH, expand=True)#grid(column=1, row=1)

#graph animation
ani = matplotlib.animation.FuncAnimation(f, animate, interval=1000) #gives the interval in ms internally to animate(i)

#serial handler
window.after(0, serialHandler) #execute as ofthen as possible

window.protocol("WM_DELETE_WINDOW", on_closing)
window.mainloop()
print(list(receivedList))  # debugging




