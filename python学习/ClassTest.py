MateInfo = {'姓名':'谢青艳', '身份证号':'612326199203201675', '性别':'女', '年龄':'24', '职业':'文员'}
Key = 'cjh521xqy'

def AppendInfoToTxt(fileName, info):
    file = open(fileName, 'a')
    file.write(info)
    file.close()
    
def WriteInfoToTxt(fileName, info):
    file = open(fileName, 'w')
    file.write(info)
    file.close()
    
class Human:
    m_name = '崔剑华'
    m_id = '612326199212125814'
    m_sex = '男'
    m_isMarried = '已婚'
    m_privateKey = ''
    
    def __init__(self, age = 24, postn = '广东省深圳市龙华新区', profesn = 'C++软件工程师'):
        self.m_age = age
        self.m_postn = postn
        self.m_profesn = profesn
        
    def GetOpenedInfo(self):
        print("This people's opened information as follows:")
        print('姓名： ', self.m_name)
        print('性别： ', self.m_sex)
        print('职业： ', self.m_profesn)
        
    def WritInfoToTxt(self, fileName):
        info = '以下是此员工的个人信息：\n姓名：' + self.m_name + '\n证件：' + self.m_id + '\n性别：' + self.m_sex + '\n年龄：' + str(self.m_age) + '\n婚姻：' + self.m_isMarried + '\n职业：' + self.m_profesn + '\n地址：' + self.m_postn       
        file = open(fileName, 'w')
        file.write(info)
        file.close()
        
    def GetPrivatedInfo(self):
        print("私有信息，请输入查看权限：\n密码：")
        self.m_privateKey = input()
        if self.m_privateKey == 'cjh521xqy':
            print('\nThis people\'s pravited information as follows:\n身份证号：', self.m_id)
            print('是否已婚：', self.m_isMarried)
            print('是否查看配偶信息,请选择：YES or NO')
            res = input()
            if('YES' == res):  
                strInfo = '\n姓名：'+ MateInfo['姓名'] + '\n证件：'+ MateInfo['身份证号'] + '\n性别：'+ MateInfo['性别'] + '\n年龄：'+ str(MateInfo['年龄']) + '\n职业：'+ MateInfo['职业']
                print(strInfo);
            print('是否将信息导入到员工基本信息：YES or NO')
            aswer = input()
            if('YES' == aswer):
                AppendInfoToTxt(fileName, strInfo)
        else:
            print('密码错误，请重新输入！')
            self.m_privateKey = input()
            if self.m_privateKey == 'cjh521xqy':
                print('This people\'s pravited information as follows:\n身份证号码:', self.m_id)
                print('是否已婚：', self.m_isMarried)
            print('是否查看配偶信息,请选择：YES or NO')
            res = input()
            if('YES' == res):
                    print('\n姓名：', MateInfo['姓名'],
                          '\n证件：', MateInfo['身份证号'],
                          '\n性别：', MateInfo['性别'],
                          '\n年龄：', MateInfo['年龄'],
                          '\n职业：', MateInfo['职业']);
            else:
                print("警告：密码多次错误请确认好个人权限，再查看私人信息。")
                
    def GetMateIndex(self):
        print(MateInfo.keys())
        
    def GetMateInfo(self, key):
        if(key == Key):
           print(MateInfo.values())
        else:
            print('你不具备查看权限!')

        
            
            
  

    
