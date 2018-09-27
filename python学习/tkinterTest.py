import tkinter as TK
from tkinter import messagebox

#创建一个新的窗口
m_window = TK.Tk()
m_window.title('计算器')
m_window.geometry('600x800')

#字符串定义
m_labelVar = TK.StringVar()
m_labelVar.set('获取输入')
m_entryOutVar = TK.StringVar()


#button点击响应事件
on_hit = False
on_hit2 = False
def onButtonHit():
    global on_hit
    if on_hit == False:
        on_hit = True
        m_labelVar.set('(*^_^*)')
        m_entryOutVar = m_entryInPut.get()
        m_textOutPut.insert('end', m_entryOutVar)
        m_listBox.insert(1,'(*^_^*)')
    else:
        on_hit = False
        m_labelVar.set('~/(ㄒoㄒ)/~')
        m_textOutPut.delete(0.0, 'end')#清除显示内容
        m_listBox.insert(2,'~/(ㄒoㄒ)/~')
def onButtonHit1():
    global on_hit2
    if on_hit2 == False:
        on_hit2 = True
        m_labelVar.set('(✿◡‿◡)')
        m_textOutPut.insert('insert', '┗|｀O′|┛')
        m_listBox.insert('end','┗|｀O′|┛')
    else:
        on_hit2 = False
        m_labelVar.set('~~~///(^v^)\\\~~~')
        m_textOutPut.insert('end', '(●~◡~●)')
        m_listBox.insert('end','~~~///(^v^)\\\~~~')
        

#Lable
m_lable = TK.Label(m_window, textvariable=m_labelVar, font=('Arial',12), width=8, height=2)
m_lable.pack()

#输入框定义
m_entryInPut = TK.Entry(m_window, show='*', bg='green', font=('Arial',12), width=100)
m_entryInPut.pack()

#Test定义
m_textOutPut = TK.Text(m_window, bg='orange', width=100, height=2)
m_textOutPut.pack()

#Button使用
m_button = TK.Button(m_window, text='HIT Me', width=15, height= 1, command=onButtonHit)
m_button.pack()

m_button1 = TK.Button(m_window, text='插入', width=15, height= 1, command=onButtonHit1)
m_button1.pack()

#ListBox的使用
m_listBox = TK.Listbox(m_window, bg='violet', listvariable = '╮(╯▽╰)╭',width=100, height=3)
m_listBox.pack()

m_radioVar = TK.StringVar()
#radioButton选中点击事件
def radionSelection():
    m_listBox.insert('end','you have slected ' + m_radioVar.get())
    
#radioButton的使用
m_radioBtn1 = TK.Radiobutton(m_window, text = '北京', variable=m_radioVar, value='A', command=radionSelection)
m_radioBtn1.pack()
m_radioBtn2 = TK.Radiobutton(m_window, text = '深圳', variable=m_radioVar, value='B', command=radionSelection)
m_radioBtn2.pack()
m_radioBtn3 = TK.Radiobutton(m_window, text = '成都', variable=m_radioVar, value='C', command=radionSelection)
m_radioBtn3.pack()
def GetScaleVale(v):
    m_listBox.insert('end','you have slected ' + v)
    
#Scale的使用
m_scale = TK.Scale(m_window, label = '进度', bg='yellow', from_ = 0, to = 100, orient=TK.HORIZONTAL, length=1000, showvalue=0, tickinterval = 10, resolution=10,command=GetScaleVale)
m_scale.pack()
#整形定义
m_checkVar1 = TK.IntVar()
m_checkVar2 = TK.IntVar()
def CheckSelection():
    if((m_checkVar1.get()==1)&(m_checkVar2.get()==0)):
        m_listBox.insert('end','I like Huwei!')
    elif((m_checkVar1.get()==0)&(m_checkVar2.get()==1)):
        m_listBox.insert('end','I like apple!')
    elif((m_checkVar1.get()==1)&(m_checkVar2.get()==1)):
        m_listBox.insert('end','I like both of them!')
    else:
        m_listBox.insert('end','NO! NO! NO! I like SUN!')
        
#checkBtton的使用
m_checkBtn1 = TK.Checkbutton(m_window, text='I like Huwei',variable=m_checkVar1, onvalue=1, offvalue=0, command=CheckSelection)
m_checkBtn1.pack()
m_checkBtn2 = TK.Checkbutton(m_window, text='I like apple',variable=m_checkVar2, onvalue=1, offvalue=0, command=CheckSelection)
m_checkBtn2.pack()

#画布的使用
m_canvas = TK.Canvas(m_window, bg='blue', width=600, height=100)
image_file=TK.PhotoImage(file='aaaa.gif')
image = m_canvas.create_image(5, 5, anchor='nw',image=image_file)
m_canvas.pack()
x1,y1,x2,y2 = 200, 20, 500, 70
#line = m_canvas.create_line(x1, y1, x2, y2)#直线
#oval = m_canvas.create_oval(x1, y1, x2, y2, fill = 'red')#圆形
#arc = m_canvas.create_arc(200, 20, 300, 80, start=0, extent=160)#扇形
rect = m_canvas.create_rectangle(200, 20, 300, 80)#矩形
def onButtonMove():
    m_canvas.move(image,2,2)
    #m_canvas.move(rect,2,2)
m_button1 = TK.Button(m_window, text='矩形移动', width=15, height= 1, command=onButtonMove)
m_button1.pack()

#菜单的使用
def Job():
    a = a+1
def Job1():
    messagebox.showinfo(title='温馨提示', message='so happy!!!')
def Job2():
    messagebox.showwarning(title='警告', message='NO!NO!NO!!!')
def Job3():
    messagebox.showerror(title='报警', message='NO!NO!NO!Never!!')
def Job4():
    messagebox.askquestion(title='提示', message='are you sure?')
def Job5():
    messagebox.askyesno(title='提示', message='are you sure?')
def Job6():
    messagebox.askokcancel(title='提示', message='are you sure?')

    #TK.messagebox.showinfo(title='温馨提示', message='so happy!!!')
m_menuBar = TK.Menu(m_window)
filemenu1 = TK.Menu(m_menuBar, tearoff=0)
m_menuBar.add_cascade(label='练习项目1', menu=filemenu1)
filemenu1.add_command(label='项目1', command=Job1)
filemenu1.add_command(label='项目2', command=Job2)
filemenu1.add_command(label='项目3', command=Job3)
filemenu2 = TK.Menu(m_menuBar, tearoff=0)
m_menuBar.add_cascade(label='练习项目2', menu=filemenu2)
filemenu2.add_command(label='项目4', command=Job4)
filemenu2.add_command(label='项目5', command=Job5)
filemenu2.add_command(label='项目6', command=Job6)
filemenu3 = TK.Menu(m_menuBar, tearoff=0)
m_menuBar.add_cascade(label='练习项目3', menu=filemenu3)
filemenu3.add_command(label='项目7', command=Job)
filemenu3.add_command(label='项目8', command=Job)
filemenu3.add_command(label='项目9', command=Job)
m_window.config(menu=m_menuBar)

#控件的布局及显示
#方式一 pack
TK.Label(m_window, text="1").pack(side='top')#bottom,left,right
#方式2 grid
for i in range(5):
    for j in range(5):
        TK.Label(m_window, text="WWW").grid(row=i, column=j, ipadx=5, ipady=5)#外部扩展padx=5, pady=5
#方式3 place
TK.Label(m_window, text="1").place(x=100,y=500,anchor='nw')
        
#窗口时时刷新
m_window.mainloop()

